#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <vector>
#include <string>

using namespace std;

#define PORT 5003

struct client {
    int cliFd;
    struct sockaddr_in cliAddr;
};

void clear(char *buff) {
    for (int i = 0; i < 1024; i++) {
        buff[i] = '\0';
    }
}

int reg[105];
int logIn[105];
vector<string> inbox[105];

void *run(void *args) {
    struct client cli = *(struct client *)args;

    char *msg[] = {"REGISTER", "LOGIN", "COMPOSE", "INBOX", "QUIT"};

    int logInId = 0;
    int ex = 0;

    while (1) {
        int i, idx = -1;

        /* Receive */
        char buff[1024];

        clear(buff);
        recv(cli.cliFd, buff, 1024, 0);

        for (i = 0; i < 5; i++) {
            if (strcmp(msg[i], buff) == 0) {
                idx = i;
                break ;
            }
        }

        switch (idx) {
            case 0:
                /* Handle registration below */
                {
                    send(cli.cliFd, "Enter a number denoting your username: ", sizeof("Enter a number denoting your username: "), 0);
                    clear(buff);
                    recv(cli.cliFd, buff, 1024, 0);

                    int id = 0;
                    for (i = 0; buff[i] != '\0'; i++) {
                        id = id * 10 + (buff[i] - '0');
                    }
                    if (reg[id]) {
                        send(cli.cliFd, "Username already taken", sizeof("Username already taken"), 0);
                    } else {
                        reg[id] = 1;
                        send(cli.cliFd, "250 OK", sizeof("250 OK"), 0);
                    }
                }

                break;
            
            case 1:
                /* Handle login below */
                {
                    send(cli.cliFd, "Enter your email: ", sizeof("Enter your email: "), 0);

                    clear(buff);
                    recv(cli.cliFd, buff, 1024, 0);
                    
                    int id = 0;
                    for (i = 0; buff[i] != '\0'; i++) {
                        id = id * 10 + (buff[i] - '0');
                    }
                    if (reg[id]) {
                        send(cli.cliFd, "250 OK", sizeof("250 OK"), 0);
                        logIn[id] = 1;
                        logInId = id;
                    } else {
                        send(cli.cliFd, "Please register", sizeof("Please register"), 0);
                    }
                }

                break;

            case 2:
                /* Handle message composition */
                {
                    if (!logInId) {
                        send(cli.cliFd, "Please login", sizeof("Please login"), 0);
                    } else {
                        send(cli.cliFd, "250 OK", sizeof("250 OK"), 0);

                        vector<int> usersToSend;
                        string msg = '\n' + to_string(logInId);
                        msg += ' ';

                        while (1) {
                            clear(buff);
                            recv(cli.cliFd, buff, 1024, 0);

                            int sendId = 0;
                            int ok = 0;
                            for (i = 0; buff[i] != '\0'; i++) {
                                if (buff[i] >= '0' && buff[i] <= '9') {
                                    ok = 1;
                                    sendId = sendId * 10 + (buff[i] - '0');
                                }
                            }
                            
                            if (ok) {
                                if (!reg[sendId]) {
                                    send(cli.cliFd, "550 No such user here", sizeof("550 No such user here"), 0);
                                } else {
                                    usersToSend.push_back(sendId);
                                    send(cli.cliFd, "250 OK", sizeof("250 OK"), 0);
                                }
                            } else {
                                /* Indicates client has sent DATA */
                                send(cli.cliFd, "354 Start mail input; end with X.X", sizeof("354 Start mail input; end with X.X"), 0);

                                while (1) {
                                    clear(buff);
                                    recv(cli.cliFd, buff, 1024, 0);
                                    if (strcmp(buff, "X.X") == 0) {
                                        break;
                                    } else {
                                        for (i = 0; buff[i] != '\0'; i++) {
                                            msg += buff[i];
                                        }
                                    }
                                }

                                for (i = 0; i < (int)usersToSend.size(); i++) {
                                    inbox[usersToSend[i]].push_back(msg);
                                }

                                break;
                            }
                        }
                    }
                }

                break;

            case 3:
                /* Handle inbox */
                {
                    if (!logInId) {
                        send(cli.cliFd, "Please login", sizeof("Please login"), 0);
                    } else {
                        send(cli.cliFd, "250 OK", sizeof("250 OK"), 0);

                        clear(buff);
                        int k = 0;
                        for (i = 0; i < (int)inbox[logInId].size(); i++) {
                            for (int j = 0; j < inbox[logInId][i].length(); j++) {
                                buff[k++] = inbox[logInId][i][j];
                            }
                        }
                        send(cli.cliFd, buff, 1024, 0);

                        inbox[logInId].clear();
                    }
                }

                break;

            case 4:
                /* Quit */
                {
                    send(cli.cliFd, "221 Connection Closed", sizeof("221 Connection Closed"), 0);
                    ex = 1;
                }
            
                break;
        }

        if (ex) break;
    }

    pthread_exit(NULL);

    return NULL;
}

int main() {
    int servFd;
    struct sockaddr_in servAddr;
    
    /* Create socket */
    if ((servFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Unable to create socket...\n");
        exit(0);
    }

    /* Forcefully attach socket to port */
    int opt = 1;
    if (setsockopt(servFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
        printf("Unable to attach to socket...\n");
        exit(0);
    } 

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(PORT);

    /* Bind */
    if (bind(servFd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        printf("Unable to bind to socket...\n");
        exit(0);
    }

    /* Listen */
    listen(servFd, 5);

    /* Accept */
    while (1) {
        int cliFd;
        struct sockaddr_in cliAddr;
        int cliLen = sizeof(cliAddr);
        cliFd = accept(servFd, (struct sockaddr *)&cliAddr, (socklen_t *)&cliLen);

        send(cliFd, "HELLO", sizeof("HELLO"), 0);

        struct client cli;
        cli.cliFd = cliFd;
        cli.cliAddr = cliAddr;

        pthread_t cliThread;
        pthread_create(&cliThread, NULL, run, (void *)&cli);
        pthread_detach(cliThread);
    }

    close(servFd);

    return 0;
}