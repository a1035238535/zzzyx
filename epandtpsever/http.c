#include<stdio.h>
#include<string.h>
#include<unistd.h>
int send_404(int fd) {
	char buf[BUFSIZ]={'\0'};
	sprintf(buf, "HTTP/1.1 404 NOT FOUND\r\nConnection: keep-alive\r\nServer: net_test_http1.1\r\nContent-Type: text/html\r\n\r\n404 not found\r\n");
	int ret = write(fd,buf,strlen(buf));
	printf("ret = %d\n",ret);
	return 0;
}

int send_head(int fd,long int len,char* content_type) {
    char buf[BUFSIZ]={'\0'};
	sprintf(buf, "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nServer: net_test_http1.1\r\nContent-Length: %ld\r\n", len);
    write(fd, buf, strlen(buf));
    bzero(buf,sizeof(buf));
    if (!strcmp(content_type, "html"))
		sprintf(buf, "Content-Type: text/html\r\n\r\n");
	else if (!strcmp(content_type, "png"))
		sprintf(buf, "Content-Type: image/png\r\n\r\n");
	else if (!strcmp(content_type, "jpg"))
		sprintf(buf, "Content-Type: image/jpg\r\n\r\n");
	else if (!strcmp(content_type, "gif"))
		sprintf(buf, "Content-Type: image/gif\r\n\r\n");
	else if (!strcmp(content_type, "mp3"))
		sprintf(buf, "Content-Type: audio/mpeg-3\r\n\r\n");
	else if (!strcmp(content_type, "mp4"))
		sprintf(buf, "Content-Type: video/mpeg\r\n\r\n");
	else if (!strcmp(content_type, "avi"))
		sprintf(buf, "Content-Type: video/avi\r\n\r\n");
	else if (!strcmp(content_type, "css"))
		sprintf(buf, "Content-Type: text/css\r\n\r\n");
//    printf("%s",buf);
    write(fd,buf,strlen(buf));
    return 0;
}
