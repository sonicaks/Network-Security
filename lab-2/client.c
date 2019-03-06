#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5555

void clear(char *buff) {
    for (int i = 0; i < 1024; i++) {
        buff[i] = '\0';
    }
}

int main() {
    int sockFd;
    struct sockaddr_in servAddr;

    /* Opening a socket */
    if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Unable to create socket...\n");
        exit(0);
    }

    /* Initialize the structure sockaddr_in */
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servAddr.sin_port = htons(PORT);

    /* connect() system call establishes connection to server process */
    if ((connect(sockFd, (struct sockaddr *)&servAddr, sizeof(servAddr))) < 0) {
        printf("Unable to connect to server...\n");
        exit(0);
    }

    while (1) {
        /* Send message to server */
        char buff[1024];
        clear(buff);
        printf("Client: ");
        scanf("%s", buff);
        send(sockFd, buff, 1024, 0);

        int ok = 0;
        if (strcmp("Bye", buff) == 0) {
            ok = 1;
        }

        /* Response from server */
        clear(buff);
        recv(sockFd, buff, 1024, 0);
        printf("Server: %s\n", buff);
        
        if (ok) break;
    }

    close(sockFd);
    
    return 0;
}