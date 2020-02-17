#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<ctype.h>
#include<arpa/inet.h>
#include<string.h>
#include <sys/epoll.h>
#include<fcntl.h>
#include"wrap.h"
#include"http.h"
#include"threadpool.h"
#define SERV_PORT 8080
#define OPEN_MAX 128

void *process(void *arg);

ssize_t efd;

int main() {
	int lfd,cfd;
	//创建服务器和客户端地址结构体
	struct sockaddr_in srv_addr,cli_addr;
	struct epoll_event temp,ep[OPEN_MAX];
	//创建线程池
	thread_pool_t *thp = threadpool_create(3,10,10);

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
	ssize_t nready,res;
	efd = epoll_create(128);
	if(efd==-1)
		perr_exit("epoll_create error");

	//文件描述符加入epoll
	temp.events = EPOLLIN;
	temp.data.fd = lfd;
  	res = epoll_ctl(efd,EPOLL_CTL_ADD,lfd,&temp);
	if(res == -1)
		perr_exit("epoll_ctl error");
	printf("server init complete!\n");
	while(1) {
		//阻塞使用epoll_wait ep为监听获取到的数组
		nready = epoll_wait(efd,ep,OPEN_MAX,-1);
		//printf("nready = %ld\n",nready);
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
				fcntl (cfd,F_SETFL,O_NONBLOCK);
				temp.events = EPOLLIN | EPOLLET;
				temp.data.fd = cfd;
				res = epoll_ctl(efd,EPOLL_CTL_ADD,cfd,&temp);
				if(res == -1)
					perr_exit("cfd epoll_ctl_error");
			}
			//监听的客户端连接
			else {
				int *sfd = &ep[i].data.fd;
				threadpool_add(thp,process,(void*)sfd);
			}
		}
	}
	close(lfd);
	close(efd);
	threadpool_destroy(thp);
	return 0;
}

void *process(void *arg) {
    int *sfd = (int*) arg;
	char buf[BUFSIZ];
	int n=0,res;
	n = read(*sfd,buf,sizeof(buf));
	//读取为0时说明客户端关闭连接
	if(n == 0) {
		res = epoll_ctl(efd,EPOLL_CTL_DEL,*sfd,NULL);
		if(res == -1)
			perror("read epoll_ctl error");
		close(*sfd);
		printf("client %d closed connection\n",*sfd);

	}
	else if(n<0) {
		perror("read n<0 error:") ;
		res = epoll_ctl(efd,EPOLL_CTL_DEL,*sfd,NULL);
		close(*sfd);
	}
	else {
		// for(int i=0;i<n;i++)
		// 	buf[i] = toupper(buf[i]);
		// write(STDOUT_FILENO,buf,n);
		// write(*sfd,buf,n);
		HTTPServer(sfd,buf);
		// res = epoll_ctl(efd,EPOLL_CTL_DEL,*sfd,NULL);
		// close(*sfd);
	}
}

// GET /1 HTTP/1.1
// HOST: LOCALHOST:8080
// CONNECTION: KEEP-ALIVE
void HTTPServer(int *sfd,char* arg) {
	char *buf = arg;
	char method[10]={'\0'};
	char url[128]={"./source"};
	char content_type[10] = {'\0'};
	char send_buf[BUFSIZ];
	int pos=0,i=0;
	//获取请求的方法

	//printf("%s\n\n",buf);

	for(i=0;buf[pos]!=' ';i++,pos++)
		method[i] = buf[pos];
	printf("%s\n",method);
	//获取请求的URL

	pos++;
	for(i=0;url[i]!='\0';i++)  ;
	for(;buf[pos]!=' ';i++,pos++)
		url[i] = buf[pos];
	url[i]='\0';

	for(i=1;url[i]!='.' && i<sizeof(url);i++);
	if(i<sizeof(url)) {
		strcpy(content_type,url+i+1);
	}
	else {
		strcpy(content_type,"html");
		strcat(url,".html");
	}
	printf("content_type: %s\n", content_type);
	printf("%s\n",url);
	int fd = open(url,O_RDONLY);
	//打开文件失败，返回404
	if(fd<0){
		perror("file open error:");
		send_404(*sfd);
		return;
	}
	//获取文件长度
	long int size= lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);
	printf("file size = %ld\n",size);
	send_head(*sfd,size,content_type);
	int ret =0;
	while((ret = read(fd,send_buf,sizeof(send_buf)))!=0)
	{
		//printf("read %d byte\n",ret);
		if(ret == -1){
			printf("read error: ");
			break;
		}
		ret = write(*sfd,send_buf,ret);
		if(ret == -1) {
			//滑动窗口不够 休眠等待处理
			usleep(5000);
			perror("write error :");
			//epoll_ctl(efd,EPOLL_CTL_DEL,*sfd,NULL);
			//close(*sfd);
			//break;
		}
	}
	printf("close fd %d\n",fd);
	close(fd);
	return;
}
