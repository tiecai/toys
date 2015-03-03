/*
 * nbsock.c
 * desc: util fuctions using non-bolcking socket api. it's fine to change to .cpp
 * TODO: tcp_connect(): use non-bolcking way
 *       tcp_accept(): use select() or poll() before accept()
 */

#include "nbsock.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>

#define  SOCK_DELAY_TIME  1


//
void msleep(unsigned int msecs) {
	if(0 == msecs) {
		sleep(0); // yield
	} else {
		struct timeval tt;
		tt.tv_sec = msecs / 1000;
		tt.tv_usec = (msecs % 1000) * 1000;
		select(0, 0, 0, 0, &tt);
	}
}

// 
int get_last_error() {
	return errno;
}

//
int get_socket_error(int sockfd) {
	int optval;
	socklen_t optlen = sizeof optval;

	if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
		return errno;
	} else {
		return optval;
	}
}


// create a tcp server socket and bind to port and listen
int tcp_create(int port, char* ipaddr) {
	if (port <= 0) {
		return -2;
	}

	sigset(SIGPIPE, SIG_IGN);

	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd <= 0) {
		return -1;
	}

	int val = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, val | O_NONBLOCK);

	int temp = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &temp, sizeof(temp));
//	setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (char *) &temp, sizeof(temp));

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (NULL == ipaddr) {
		server.sin_addr.s_addr = htonl(INADDR_ANY);
	} else {
		server.sin_addr.s_addr = inet_addr(ipaddr); // inet_addr("192.168.114.88")
	}

	int bd = bind(sockfd, (struct sockaddr *) &server, sizeof(server));
	if (bd < 0) {
		if (sockfd > 0) {
			close(sockfd);
			sockfd = 0;
		}
		return -1;
	}

	int ls = listen(sockfd, 25);
	if (ls < 0) {
		if (sockfd > 0) {
			close(sockfd);
			sockfd = 0;
		}
		return -1;
	}

	return sockfd;
}

// create a tcp client socket and connect to ipaddr/port (in nonblock way)
int tcp_connect(int port, char* ipaddr) {
	if (port <= 0) {
		return -2;
	}

	sigset(SIGPIPE, SIG_IGN);

	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd <= 0) {
		return -1;
	}

	struct sockaddr_in serv_addr_in;
	serv_addr_in.sin_family = AF_INET;
	serv_addr_in.sin_port = htons(port);
	if (NULL == ipaddr) {
		serv_addr_in.sin_addr.s_addr = inet_addr("127.0.0.1");
	} else {
		serv_addr_in.sin_addr.s_addr = inet_addr(ipaddr);
	}

	if (connect(sockfd, (struct sockaddr *) &serv_addr_in, sizeof(serv_addr_in)) == 0) {
		int val = fcntl(sockfd, F_GETFL, 0);
		fcntl(sockfd, F_SETFL, val | O_NONBLOCK);

		int temp = 16384; // 3000000
		setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *) &temp, sizeof(temp));
		setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *) &temp, sizeof(temp));

		temp = 1; // TRUE
		setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char *) &temp, sizeof(temp));
	} else { // EISCONN is no need close
		if (sockfd > 0) {
			close(sockfd);
			sockfd = 0;
		}
		return -1;
	}

	return sockfd;
}

// accept() and set connected socket non-bolcking   (select before accept)
int tcp_accept(int sockfd, char* ipaddr, int* port) {
	if (sockfd <= 0) {
		return -2;
	}

	struct sockaddr_in addr_clnt;
	socklen_t tp = sizeof(addr_clnt);
	int x = 0;

	while ((x = accept(sockfd, (struct sockaddr *) &addr_clnt, &tp)) < 0) {
		int err = get_last_error();
		if ((err == EWOULDBLOCK) || (err == EINTR) || (err == EAGAIN)) {
			msleep(SOCK_DELAY_TIME); //
		} else {
//			printf("unhandled accept error! %d \n", err);
			return -1;
		}
	}

	if (ipaddr != NULL) {
		strcpy(ipaddr, inet_ntoa(addr_clnt.sin_addr));
	}
	if (port != NULL) {
		*port = ntohs(addr_clnt.sin_port);
	}

	int val = fcntl(x, F_GETFL, 0);
	fcntl(x, F_SETFL, val | O_NONBLOCK);

	int temp = 16384; // 3000000
	setsockopt(x, SOL_SOCKET, SO_RCVBUF, (char *) &temp, sizeof(temp));
	setsockopt(x, SOL_SOCKET, SO_SNDBUF, (char *) &temp, sizeof(temp));

	return x;
}

// close socket and all data to be sent are dumped
int tcp_close(int sockfd) {
	if (sockfd <= 0) {
		return 0;
	}

	struct linger linge;
	linge.l_onoff = 1;
	linge.l_linger = 0;

	setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (char *) &linge, sizeof(linge));

	int ret = close(sockfd);
	if (ret == 0) {
		return 0;
	} else {
		return -1;
	}
}

// non-bolcking recv()
int tcp_read(int sockfd, char* buf, int size, int delay) {
	if (sockfd <= 0)
		return -2;
	if (buf == NULL)
		return -2;
	if (size <= 0)
		return -2;

	int dl = 0;
	if (delay > 0)
		dl = delay;

	char *ptr = buf;
	int rxnum = 0;
	int rxn = 0;

	while (size > 0) {
		rxn = recv(sockfd, ptr, size, 0);
		if (rxn > 0) {
			rxnum += rxn;
			ptr += rxn;
			size -= rxn;
		} else if (rxn < 0) {
			int err = get_last_error();
			if((err == EWOULDBLOCK) || (err == EINTR) || (err == EAGAIN)) {
				if (dl-- > 0) {
					msleep(SOCK_DELAY_TIME); //
				} else { // timeout
					rxn = 0;
					break;
				}
			} else { // unhandled error happened
				rxn = -1;
				break;
			}
		} else { // rxn == 0 : remote peer close socket
			rxn = -1;
			break;
		}
	}

	if(rxnum > 0) {
		return rxnum;
	} else {
		return rxn;
	}
}

// non-bolcking send()
int tcp_write(int sockfd, char* buf, int size, int delay) {
	if (sockfd <= 0)
		return -2;
	if (buf == NULL)
		return -2;
	if (size <= 0)
		return -2;

	int dl = 0;
	if (delay > 0)
		dl = delay;

	char *ptr = buf;
	int txnum = 0;
	int txn = 0;

	while (size > 0) {
		txn = send(sockfd, ptr, size, 0);
		if (txn > 0) {
			txnum += txn;
			ptr += txn;
			size -= txn;
		} else if (txn < 0) {
			int err = get_last_error();
			if((err == EWOULDBLOCK) || (err == EINTR) || (err == EAGAIN)) {
				if (dl-- > 0) {
					msleep(SOCK_DELAY_TIME); //
				} else { // timeout
					txn = 0;
					break;
				}
			} else { // unhandled error happened
				txn = -1;
				break;
			}
		} else { // txn == 0 : remote peer close socket
			txn = -1;
			break;
		}
	}

	if(txnum > 0) {
		return txnum;
	} else {
		return txn;
	}
}


// create udp socket and set non-bolcking
int udp_open(int port, char* ipaddr) {
	if (port <= 0) {
		return -2;
	}

	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd <= 0) {
		return -1;
	}

	int val = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, val | O_NONBLOCK);

	int temp = 160000; // 3000000
	setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *) &temp, sizeof(temp));
	setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *) &temp, sizeof(temp));

	temp = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &temp, sizeof(temp));
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (char *) &temp, sizeof(temp));

	struct sockaddr_in sd;
	sd.sin_family = AF_INET;
	sd.sin_port = htons(port);
	if (NULL == ipaddr) {
		sd.sin_addr.s_addr = htonl(INADDR_ANY);
	} else {
		sd.sin_addr.s_addr = inet_addr(ipaddr); // inet_addr("192.168.114.88")
	}

	int bd = bind(sockfd, (struct sockaddr *) &sd, sizeof(struct sockaddr_in));
	if (bd < 0) {
		if (sockfd > 0) {
			close(sockfd);
			sockfd = 0;
		}
		return -1;
	}

	return sockfd;
}

// close udp socket
int udp_close(int sockfd) {
	if (sockfd <= 0) {
		return 0;
	}

	int ret = close(sockfd);
	if (ret == 0) {
		return 0;
	} else {
		return -1;
	}
}

// non-bolcking recvfrom()
int udp_read(int sockfd, char* buf, int len, char* ipaddr, int* port, int delay) {
	if (sockfd <= 0)
		return -2;
	if (buf == NULL)
		return -2;
	if (len <= 0)
		return -2;

	int dl = 0;
	if (delay > 0)
		dl = delay;

	struct sockaddr_in rxaddr;
	socklen_t a_size = sizeof(rxaddr);

	int ret = 0;
	while ((ret = recvfrom(sockfd, buf, len, 0, (sockaddr*) &rxaddr, &a_size)) <= 0) { // < 0
		if(ret < 0) {
			int err = get_last_error();
			if((err == EWOULDBLOCK) || (err == EINTR) || (err == EAGAIN)) {
				if(dl-- > 0) {
					msleep(SOCK_DELAY_TIME); //
				} else {
					ret = 0;
					break;
				}
			} else { // unhandled error happened
				ret = -1;
				break;
			}
		} else { // ret == 0 : ignore
			
		}
	}

	if (ret > 0) {
		if (ipaddr != NULL) {
			strcpy(ipaddr, inet_ntoa(rxaddr.sin_addr));
		}
		if (port != NULL) {
			*port = ntohs(rxaddr.sin_port);
		}
	}
	return ret;
}

// non-bolcking sendto()
int udp_write(int sockfd, char* buf, int len, char* ipaddr, int port, int delay) {
	if (sockfd <= 0)
		return -2;
	if (buf == NULL)
		return -2;
	if (len <= 0)
		return -2;
	if (ipaddr == NULL)
		return -2;
	if (strlen(ipaddr) == 0)
		return -2;
	if (port <= 0)
		return -2;

	int dl = 0;
	if (delay > 0)
		dl = delay;

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ipaddr);
	addr.sin_port = htons(port);

	int ret = 0;
	while ((ret = sendto(sockfd, (char *) buf, len, 0, (sockaddr*) &addr, sizeof(sockaddr))) <= 0) { // < 0
		if(ret < 0) {
			int err = get_last_error();
			if((err == EWOULDBLOCK) || (err == EINTR) || (err == EAGAIN)) {
				if(dl-- > 0) {
					msleep(SOCK_DELAY_TIME); //
				} else {
					ret = 0;
					break;
				}
			} else { // unhandled error happened
				ret = -1;
				break;
			}
		} else { // ret == 0 : ignore
			
		}
	}

	return ret;
}

