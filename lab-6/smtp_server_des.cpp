#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "DES.h"

#include <vector>
#include <string>

using namespace std;

#define PORT 5004

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
string key = "AABB09182736CCDD";
vector<string> roundKey;

void *run(void *args) {
    struct client cli = *(struct client *)args;

    char *msg[] = {"REGISTER", "LOGIN", "COMPOSE", "INBOX", "QUIT"};

    int logInId = 0;
    int ex = 0;

    while (1) {
        int i, idx = -1;

        /* Receive */
        char buff[1024], msgToSend[1024], decodedBuff[1024];

        clear(buff);
        clear(decodedBuff);
        recv(cli.cliFd, buff, 1024, 0);
        strcpy(decodedBuff, decode(buff, roundKey).data());

        for (i = 0; i < 5; i++) {
            if (strcmp(msg[i], decodedBuff) == 0) {
                idx = i;
                break ;
            }
        }

        switch (idx) {
            case 0:
                /* Handle registration below */
                {
                    clear(msgToSend);
                    strcpy(msgToSend, encode("Enter a number denoting your username: ", roundKey).data());
                    send(cli.cliFd, msgToSend, sizeof(msgToSend), 0);
                    clear(buff);
                    clear(decodedBuff);
                    recv(cli.cliFd, buff, 1024, 0);
                    strcpy(decodedBuff, decode(buff, roundKey).data());

                    int id = 0;
                    for (i = 0; decodedBuff[i] != '\0'; i++) {
                        id = id * 10 + (decodedBuff[i] - '0');
                    }
                    if (reg[id]) {
                        clear(msgToSend);
                        strcpy(msgToSend, encode("Username already taken", roundKey).data());
                        send(cli.cliFd, msgToSend, sizeof(msgToSend), 0);
                    } else {
                        reg[id] = 1;
                        clear(msgToSend);
                        strcpy(msgToSend, encode("250 OK", roundKey).data());
                        send(cli.cliFd, msgToSend, sizeof(msgToSend), 0);
                    }
                }

                break;
            
            case 1:
                /* Handle login below */
                {
                    clear(msgToSend);
                    strcpy(msgToSend, encode("Enter your email: ", roundKey).data());
                    send(cli.cliFd, msgToSend, sizeof(msgToSend), 0);

                    clear(buff);
                    clear(decodedBuff);
                    recv(cli.cliFd, buff, 1024, 0);
                    strcpy(decodedBuff, decode(buff, roundKey).data());
                    
                    int id = 0;
                    for (i = 0; decodedBuff[i] != '\0'; i++) {
                        id = id * 10 + (decodedBuff[i] - '0');
                    }
                    if (reg[id]) {
                        clear(msgToSend);
                        strcpy(msgToSend, encode("250 OK", roundKey).data());
                        send(cli.cliFd, msgToSend, sizeof(msgToSend), 0);
                        logIn[id] = 1;
                        logInId = id;
                    } else {
                        clear(msgToSend);
                        strcpy(msgToSend, encode("Please register", roundKey).data());
                        send(cli.cliFd, msgToSend, sizeof(msgToSend), 0);
                    }
                }

                break;

            case 2:
                /* Handle message composition */
                {
                    if (!logInId) {
                        clear(msgToSend);
                        strcpy(msgToSend, encode("Please login", roundKey).data());
                        send(cli.cliFd, msgToSend, sizeof(msgToSend), 0);
                    } else {
                        clear(msgToSend);
                        strcpy(msgToSend, encode("250 OK", roundKey).data());
                        send(cli.cliFd, msgToSend, sizeof(msgToSend), 0);

                        vector<int> usersToSend;
                        string msg = "";
                        msg += "\nFROM: ";
                        msg += to_string(logInId);
                        msg += "@mymailserver";
                        msg += "\nDATA: ";

                        while (1) {
                            clear(buff);
                            clear(decodedBuff);
                            recv(cli.cliFd, buff, 1024, 0);
                            strcpy(decodedBuff, decode(buff, roundKey).data());

                            int sendId = 0;
                            int ok = 0;
                            for (i = 0; decodedBuff[i] != '\0'; i++) {
                                if (decodedBuff[i] >= '0' && decodedBuff[i] <= '9') {
                                    ok = 1;
                                    sendId = sendId * 10 + (decodedBuff[i] - '0');
                                }
                            }
                            
                            if (ok) {
                                if (!reg[sendId]) {
                                    clear(msgToSend);
                                    strcpy(msgToSend, encode("550 No such user here", roundKey).data());
                                    send(cli.cliFd, msgToSend, sizeof(msgToSend), 0);
                                } else {
                                    usersToSend.push_back(sendId);
                                    clear(msgToSend);
                                    strcpy(msgToSend, encode("250 OK", roundKey).data());
                                    send(cli.cliFd, msgToSend, sizeof(msgToSend), 0);
                                }
                            } else {
                                /* Indicates client has sent DATA */
                                clear(msgToSend);
                                strcpy(msgToSend, encode("354 Start mail input; end with X.X", roundKey).data());
                                send(cli.cliFd, msgToSend, sizeof(msgToSend), 0);

                                while (1) {
                                    clear(buff);
                                    clear(decodedBuff);
                                    recv(cli.cliFd, buff, 1024, 0);
                                    strcpy(decodedBuff, decode(buff, roundKey).data());

                                    if (strcmp(decodedBuff, "X.X") == 0) {
                                        break;
                                    } else {
                                        for (i = 0; decodedBuff[i] != '\0'; i++) {
                                            msg += decodedBuff[i];
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
                        clear(msgToSend);
                        strcpy(msgToSend, encode("Please login", roundKey).data());
                        send(cli.cliFd, msgToSend, sizeof(msgToSend), 0);
                    } else {
                        clear(msgToSend);
                        strcpy(msgToSend, encode("250 OK", roundKey).data());
                        send(cli.cliFd, msgToSend, sizeof(msgToSend), 0);

                        clear(buff);
                        int k = 0;
                        for (i = 0; i < (int)inbox[logInId].size(); i++) {
                            for (int j = 0; j < inbox[logInId][i].length(); j++) {
                                buff[k++] = inbox[logInId][i][j];
                            }
                        }
                        clear(msgToSend);
                        strcpy(msgToSend, encode(buff, roundKey).data());
                        send(cli.cliFd, msgToSend, sizeof(msgToSend), 0);

                        inbox[logInId].clear();
                    }
                }

                break;

            case 4:
                /* Quit */
                {
                    clear(msgToSend);
                    strcpy(msgToSend, encode("221 Connection Closed", roundKey).data());
                    send(cli.cliFd, msgToSend, sizeof(msgToSend), 0);
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

    roundKey = generateRoundKey(key);

    /* Accept */
    while (1) {
        int cliFd;
        struct sockaddr_in cliAddr;
        int cliLen = sizeof(cliAddr);
        cliFd = accept(servFd, (struct sockaddr *)&cliAddr, (socklen_t *)&cliLen);

        char msgToSend[1024] = {'\0'};
        strcpy(msgToSend, encode("HELLO", roundKey).data());
        send(cliFd, msgToSend, sizeof(msgToSend), 0);

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