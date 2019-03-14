#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define PORT 5003

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

    char rec[1024];
    recv(sockFd, rec, 1024, 0);
    printf("S: %s\n", rec);

    char *msg[] = {"REGISTER", "LOGIN", "COMPOSE", "INBOX", "QUIT"};

    int ex = 0;

    while (1) {
        /* Send message to server */
        char buff[1024];
        printf("C: ");
        scanf("%s", buff);
        send(sockFd, buff, 1024, 0);

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
                    printf("S: %s", buff);

                    clear(buff);
                    scanf("%s", buff);
                    send(sockFd, buff, 1024, 0);

                    clear(buff);
                    recv(sockFd, buff, 1024, 0);
                    printf("S: %s\n", buff);
                }

                break;

            case 1:
                /* Handle login */
                {
                    clear(buff);
                    recv(sockFd, buff, 1024, 0);
                    printf("S: %s", buff);

                    clear(buff);
                    scanf("%s", buff);
                    send(sockFd, buff, 1024, 0);

                    clear(buff);
                    recv(sockFd, buff, 1024, 0);
                    printf("S: %s\n", buff);
                }
                
                break;

            case 2:
                /* Handle message composition */
                {
                    clear(buff);
                    recv(sockFd, buff, 1024, 0);
                    printf("S: %s\n", buff);

                    if (strcmp(buff, "250 OK") == 0) {
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
                                send(sockFd, buff, 1024, 0);

                                clear(buff);
                                recv(sockFd, buff, 1024, 0);
                                printf("S: %s\n", buff);
                            } else {
                                send(sockFd, "DATA", sizeof("DATA"), 0);

                                clear(buff);
                                recv(sockFd, buff, 1024, 0);
                                printf("S: %s\n", buff);

                                while (1) {
                                    printf("C: ");
                                    clear(buff);
                                    scanf("%s", buff);
                                    send(sockFd, buff, 1024, 0);

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

                    if (strcmp(buff, "250 OK") == 0) {
                        clear(buff);
                        recv(sockFd, buff, 1024, 0);
                        printf("S: %s\n", buff);
                    }
                }

                break;
            
            case 4:
                {
                    clear(buff);
                    send(sockFd, "221 Connection Closed", sizeof("221 Connection Closed"), 0);
                    ex = 1;
                }

                break;
        }

        if (ex) break;
    }

    close(sockFd);

    return 0;
}