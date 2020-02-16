#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<sys/socket.h>
#include<arpa/inet.h>
void perr_exit(const char *s) {
	perror(s);
	exit(-1);
}

int Accept(int fd,struct sockaddr_in *sa,socklen_t *salen) {
	int n;
again:
	if((n=accept(fd,(struct sockaddr*) sa,salen))<0) {
		if((errno == ECONNABORTED) || (errno == EINTR)) //非阻塞或信号
			goto again;
		else
			perr_exit("accept error");
	}
	return n;
}

int Bind(int fd,const struct sockaddr_in *sa, socklen_t salen) {
	int n;
	if((n=bind(fd,(struct sockaddr*)sa,salen))<0)
		perr_exit("bind error");
	return n;
}

int Connect(int fd,const struct sockaddr_in *sa,socklen_t salen) {
	int n;
	if((n=connect(fd,(struct sockaddr*) sa,salen))<0)
		perr_exit("connect error");
	return n;
}

int Listen(int fd,int backlog) {
	int n;
	if((n=listen(fd,backlog))<0)
		perr_exit("listen error");
	return n;
}

int Socket(int family, int type,int protocol) {
	int n;
	if((n=socket(family,type,protocol))<0)
		perr_exit("socket error");
	return n;
}
