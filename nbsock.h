/*
 * nbsock.h
 * desc: util fuctions using non-bolcking socket api
 */

#ifndef  NB_SOCK_H
#define  NB_SOCK_H

void msleep(unsigned int msecs);

int tcp_create(int port, char* ipaddr = NULL);
int tcp_connect(int port, char* ipaddr = NULL);
int tcp_accept(int sockfd, char* ipaddr = NULL, int* port = NULL);
int tcp_close(int sockfd);
int tcp_read(int sockfd, char* buf, int size, int delay = 0);
int tcp_write(int sockfd, char* buf, int size, int delay = 0);

int udp_open(int port, char* ipaddr = NULL);
int udp_close(int sockfd);
int udp_read(int sockfd, char* buf, int len, char* ipaddr = NULL, int* port = NULL, int delay = 0);
int udp_write(int sockfd, char* buf, int len, char* ipaddr, int port, int delay = 0);

#endif // #ifndef  NB_SOCK_H
