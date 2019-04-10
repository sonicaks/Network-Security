#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "RSA.h"

#include <vector>
#include <string>

using namespace std;

#define PORT 5004

int publicKey = 7711;
int privateKey = 20232583;
int mod = 607114259; // 13687 * 44357

void clear(char *buff) {
    int i;
    for (i = 0; i < 1024; i++) {
        buff[i] = '\0';
    }
}

int main() {
    int sockFd;
    struct sockaddr_in servAddr;

    /* Open socket */
    if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Unable to create socket...\n");
        exit(0);
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servAddr.sin_port = htons(PORT);

    /* Connect */
    if (connect(sockFd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        printf("Unable to connect to server socket...\n");
        exit(0);
    }

    char rec[1024], decodedRec[1024];
    clear(rec);
    recv(sockFd, rec, 1024, 0);
    printf("S: %s\n", rec);
    clear(decodedRec);
    strcpy(decodedRec, decode(rec, privateKey, mod).data());
    printf("S: %s\n", decodedRec);

    char *msg[] = {"REGISTER", "LOGIN", "COMPOSE", "INBOX", "QUIT"};

    int ex = 0;

    while (1) {
        /* Send message to server */
        char buff[1024], msgToSend[1024], decodedBuff[1024];
        printf("C: ");
        scanf("%s", buff);
        clear(msgToSend);
        strcpy(msgToSend, encode(buff, publicKey, mod).data());
        send(sockFd, msgToSend, sizeof(msgToSend), 0);

        int idx = -1, i;
        
        for (i = 0; i < 5; i++) {
            if (strcmp(msg[i], buff) == 0) {
                idx = i;
                break;
            }
        }

        /* Receive message from server */
        switch (idx) {
            case 0:
                /* Handle register */
                {
                    clear(buff);
                    recv(sockFd, buff, 1024, 0);
                    printf("S: %s\n", buff);
                    clear(decodedBuff);
                    strcpy(decodedBuff, decode(buff, privateKey, mod).data());
                    printf("S: %s", decodedBuff);

                    clear(buff);
                    scanf("%s", buff);
                    clear(msgToSend);
                    strcpy(msgToSend, encode(buff, publicKey, mod).data());
                    send(sockFd, msgToSend, sizeof(msgToSend), 0);

                    clear(buff);
                    recv(sockFd, buff, 1024, 0);
                    printf("S: %s\n", buff);
                    clear(decodedBuff);
                    strcpy(decodedBuff, decode(buff, privateKey, mod).data());
                    printf("S: %s\n", decodedBuff);
                }

                break;

            case 1:
                /* Handle login */
                {
                    clear(buff);
                    recv(sockFd, buff, 1024, 0);
                    printf("S: %s\n", buff);
                    clear(decodedBuff);
                    strcpy(decodedBuff, decode(buff, privateKey, mod).data());
                    printf("S: %s", decodedBuff);

                    clear(buff);
                    scanf("%s", buff);
                    clear(msgToSend);
                    strcpy(msgToSend, encode(buff, publicKey, mod).data());
                    send(sockFd, msgToSend, sizeof(msgToSend), 0);

                    clear(buff);
                    recv(sockFd, buff, 1024, 0);
                    printf("S: %s\n", buff);
                    clear(decodedBuff);
                    strcpy(decodedBuff, decode(buff, privateKey, mod).data());
                    printf("S: %s\n", decodedBuff);
                }
                
                break;

            case 2:
                /* Handle message composition */
                {
                    clear(buff);
                    recv(sockFd, buff, 1024, 0);
                    printf("S: %s\n", buff);
                    clear(decodedBuff);
                    strcpy(decodedBuff, decode(buff, privateKey, mod).data());
                    printf("S: %s\n", decodedBuff);

                    if (strcmp(decodedBuff, "250 OK") == 0) {
                        while (1) {
                            printf("C: REPT or DATA: ");
                            clear(buff);
                            scanf("%s", buff);

                            if (strcmp(buff, "REPT") == 0) {
                                printf("C: Email of REPT: ");
                                clear(buff);
                                buff[0] = 'R';
                                buff[1] = 'E';
                                buff[2] = 'P';
                                buff[3] = 'T';
                                buff[4] = ':';
                                buff[5] = ' ';
                                scanf("%s", buff + 6);
                                clear(msgToSend);
                                strcpy(msgToSend, encode(buff, publicKey, mod).data());
                                send(sockFd, msgToSend, sizeof(msgToSend), 0);

                                clear(buff);
                                recv(sockFd, buff, 1024, 0);
                                printf("S: %s\n", buff);
                                clear(decodedBuff);
                                strcpy(decodedBuff, decode(buff, privateKey, mod).data());
                                printf("S: %s\n", decodedBuff);
                            } else {
                                clear(msgToSend);
                                strcpy(msgToSend, encode("DATA", publicKey, mod).data());
                                send(sockFd, msgToSend, sizeof(msgToSend), 0);

                                clear(buff);
                                recv(sockFd, buff, 1024, 0);
                                printf("S: %s\n", buff);
                                clear(decodedBuff);
                                strcpy(decodedBuff, decode(buff, privateKey, mod).data());
                                printf("S: %s\n", decodedBuff);

                                while (1) {
                                    printf("C: ");
                                    clear(buff);
                                    scanf("%s", buff);
                                    clear(msgToSend);
                                    strcpy(msgToSend, encode(buff, publicKey, mod).data());
                                    send(sockFd, msgToSend, sizeof(msgToSend), 0);

                                    if (strcmp(buff, "X.X") == 0) break;
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
                    clear(buff);
                    recv(sockFd, buff, 1024, 0);
                    printf("S: %s\n", buff);
                    clear(decodedBuff);
                    strcpy(decodedBuff, decode(buff, privateKey, mod).data());
                    printf("S: %s\n", decodedBuff);

                    if (strcmp(decodedBuff, "250 OK") == 0) {
                        clear(buff);
                        recv(sockFd, buff, 1024, 0);
                        printf("S: %s\n", buff);
                        clear(decodedBuff);
                        strcpy(decodedBuff, decode(buff, privateKey, mod).data());
                        printf("S: %s\n", decodedBuff);
                    }
                }

                break;
            
            case 4:
                {
                    clear(buff);
                    clear(msgToSend);
                    strcpy(msgToSend, encode("221 Connection Closed", publicKey, mod).data());
                    send(sockFd, msgToSend, sizeof(msgToSend), 0);
                    ex = 1;
                }

                break;
        }

        if (ex) break;
    }

    close(sockFd);

    return 0;
}