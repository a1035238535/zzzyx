#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<ctype.h>
#include<arpa/inet.h>
#include<string.h>
#include <sys/epoll.h>
#include"wrap.h"
#define SERV_PORT 6666
#define OPEN_MAX 128

int main() {
	int lfd,cfd,sfd;
	//创建服务器和客户端地址结构体
	struct sockaddr_in srv_addr,cli_addr;

	struct epoll_event temp,ep[OPEN_MAX];

	//创建服务器socket
	lfd = Socket(AF_INET,SOCK_STREAM,0);
	//设置地址模式为ipv4
	srv_addr.sin_family = AF_INET;
	//将本地端口转换为网络序（小端转大端）
	srv_addr.sin_port = htons(SERV_PORT);
	//转换IP  宏INADDR_ANY取可用任意IP
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//设置端口复用
	int opt=1;
	setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	//绑定
	Bind(lfd,&srv_addr,sizeof(srv_addr));

	//设置监听数 默认128
	Listen(lfd,32);

	//创建epoll句柄
	ssize_t nready,efd,res;
	efd = epoll_create(128);
	if(efd==-1)
		perr_exit("epoll_create error");

	//文件描述符加入epoll
	temp.events = EPOLLIN;
	temp.data.fd = lfd;
  res = epoll_ctl(efd,EPOLL_CTL_ADD,lfd,&temp);
	if(res == -1)
		perr_exit("epoll_ctl error");
	while(1) {
		//阻塞使用epoll_wait ep为监听获取到的数组
		nready = epoll_wait(efd,ep,OPEN_MAX,-1);
		if(nready == -1)
			perr_exit("epoll_wait error");
		//循环处理ep数组
		for(int i=0;i<nready;i++) {
			//不是读事件就下一个
			if(!ep[i].events & EPOLLIN)
				continue;
			//如果是监听的文件描述符
			if(ep[i].data.fd == lfd) {
				//建立连接
				socklen_t cli_len=sizeof(cli_addr);
				bzero(&cli_addr,cli_len);
				cfd = Accept(lfd,&cli_addr,&cli_len);
				char cli_IP[BUFSIZ];
				printf("client IP:%s,client port:%d \n",
					inet_ntop(AF_INET,&cli_addr.sin_addr.s_addr,
						cli_IP,sizeof(cli_IP)),ntohs(cli_addr.sin_port));
				//加入epoll
				temp.events = EPOLLIN;
				temp.data.fd = cfd;
				res = epoll_ctl(efd,EPOLL_CTL_ADD,cfd,&temp);
				if(res == -1)
					perr_exit("epoll_ctl_error");
			}
			//监听的客户端连接
			else {
				char buf[BUFSIZ];
				int n=0;
				sfd = ep[i].data.fd;
				n = read(sfd,buf,sizeof(buf));
				//读取为0时说明客户端关闭连接
				if(n == 0) {
					res = epoll_ctl(efd,EPOLL_CTL_DEL,sfd,NULL);
					if(res == -1)
						perr_exit("epoll_ctl error");
					close(sfd);
					printf("client %d closed connection\n",sfd);

				}
				else if(n<0) {
					perror("read n<0 error:") ;
					res = epoll_ctl(efd,EPOLL_CTL_DEL,sfd,NULL);
					close(sfd);
				}
				else {
					for(int i=0;i<n;i++)
						buf[i] = toupper(buf[i]);
					write(STDOUT_FILENO,buf,n);
					write(sfd,buf,n);
				}
			}
		}
	}
	close(lfd);
	close(efd);
	return 0;
}
