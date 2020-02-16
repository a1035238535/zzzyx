#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<string.h>
#include<errno.h>
#include"wrap.h"
#include<pthread.h>
#define USLEEP_MS 1000

#define  SERV_IP "127.0.0.1"
#define SERV_PORT 6666

int cfd;
struct sockaddr_in serv_addr;

void* func(void *arg) {
	cfd=Socket(AF_INET,SOCK_STREAM,0);
	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET,SERV_IP,&serv_addr.sin_addr.s_addr);
	int ret;
	ret = Connect(cfd,&serv_addr,sizeof(serv_addr));
	int read_n;
	char *c = (char*) arg;
	char buf[BUFSIZ];
	sprintf(buf,"%s",c);
	write(cfd,buf,strlen(buf));
	read_n=read(cfd,buf,sizeof(buf));
	write(STDOUT_FILENO,buf,read_n);
	close(cfd);
	pthread_exit(NULL);
}

int main(void) {
	int i=0;
	char c='a',temp[]="a\n";
	pthread_t threads[20];
	for(int i=0;i<20;i++)
	{
		usleep(100000);
		printf("i=%d \n" ,i);
		temp[0] = c+i;
		pthread_create(&(threads[i]),NULL,func,(void*)temp);
		//func(NULL);
	}
	return 0;
}
