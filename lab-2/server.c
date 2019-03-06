#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 5555

struct client {
    int cliSockFd;
    struct sockaddr_in cliAddr;
};

int size(int n) {
    int sz = 0;
    while (n > 0) {
        sz++;
        n /= 10;
    }
    return sz;
}

void clear(char *buff) {
    for (int i = 0; i < 1024; i++) {
        buff[i] = '\0';
    }
}

void *run(void *args) {
    struct client cli = *(struct client *)args;

    char *msg[] = {"Hello", "Bye"};
    char *rep[] = {"Hello from server", "Goodbye"};
    
    while (1) {
        /* Receive message from client */
        char buff[1024];
        clear(buff);
        recv(cli.cliSockFd, buff, 1024, 0);
        printf("Client: %s\n", buff);

        /* Reply to client */
        int i;
        for (i = 0; i < 2; i++) {
            if (strcmp(msg[i], buff) == 0) {
                break;
            }
        }
        if (i != 2) {
            send(cli.cliSockFd, rep[i], 1024, 0);
        } else {
            int j, x, y, sum;
            x = y = sum = 0;
            for (j = 0; buff[j] != '+'; j++) {
                x = x * 10 + (buff[j] - '0');
            }
            for (j = j + 1; buff[j] != '\0'; j++) {
                y = y * 10 + (buff[j] - '0');
            }
            sum = x + y;
            int len = size(sum);
            char revSum[len];
            int k = 0;
            while (sum > 0) {
                revSum[k++] = (char)(sum % 10 + '0');
                sum /= 10;
            }
            buff[j] = '=';
            j++;
            for (k = len - 1; k >= 0; j++, k--) {
                buff[j] = revSum[k];
            }
            buff[j] = '\0';
            send(cli.cliSockFd, buff, sizeof(buff), 0);
        }
        printf("Replied to client %d: %s\n", cli.cliSockFd, buff);
        if (i == 1) break;
    }

    pthread_exit(NULL);
}

int main() {
    int sockFd;
    struct sockaddr_in servAddr;

    /* Open a socket */
    if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Cannot create socket...\n");
        exit(0);
    }

    /* Initialize the structure sockaddr_in which belongs to <netinet/in.h>
       "sin_family" specifies the family and is therefore AF_INET
       "sin_addr" specifies the internet address of the server
       The above field is set to INADDR_ANY for machines having single IP
       "sin_port" specifies the port number of the server
    */
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(PORT);

    /* Associate the server with the port with the bind system call */
    if (bind(sockFd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        printf("Unable to bind local address...\n");
        exit(0);
    }

    /* listen specifies that up to 4 concurrent client requests will be
       queued up while the system is executing the "accept" system call
       below
    */
    listen(sockFd, 4);

    while (1) {
        int cliSockFd;
        struct sockaddr_in cliAddr;
        int cliLen = sizeof(cliAddr);
        cliSockFd = accept(sockFd, (struct sockaddr *)&cliAddr, &cliLen);

        if (cliSockFd < 0) {
            printf("Accept error...\n");
            exit(0);
        }

        printf("Connected with client %d\n", cliSockFd);

        struct client cli;
        cli.cliSockFd = cliSockFd;
        cli.cliAddr = cliAddr;

        pthread_t cliThread;
        pthread_create(&cliThread, NULL, run, &cli);
    }

    close(sockFd);

    return 0;
}