#include<stdio.h>
#include<string.h>
int send_404(int fd) {
	char buf[BUFSIZ]={'\0'};
	sprintf(buf, "HTTP/1.1 404 NOT FOUND\r\nConnection: keep-alive\r\n\
        Server: net_test_http1.1\r\nContent-Type: text/html\r\n\r\n404 not found");
	write(fd, buf, sizeof(buf));
	return 0;
}

int send_head(int fd,long int len,char* content_type) {
    char buf[BUFSIZ]={'\0'};
	//sprintf(buf, "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nServer: net_test_http1.1\r\nContent-Length: %ld\r\n", len);
    sprintf(buf, "HTTP/1.1 200 OK\r\n");
    write(fd, buf, sizeof(buf));
    bzero(buf,sizeof(buf));
    sprintf(buf, "Connection: keep-alive\r\n");
	write(fd, buf, sizeof(buf));
    bzero(buf,sizeof(buf));
    sprintf(buf, "Server: net_test_http1.1\r\n" );
	write(fd, buf, sizeof(buf));
    bzero(buf,sizeof(buf));
    sprintf(buf, "Content-Length: %ld\r\n", len);
	write(fd, buf, sizeof(buf));
    bzero(buf,sizeof(buf));
    //printf("%s",buf);
    if (!strcmp(content_type, "html"))
		sprintf(buf, "Content-Type: text/html\n");
	else if (!strcmp(content_type, "png"))
		sprintf(buf, "Content-Type: image/png\n");
	else if (!strcmp(content_type, "jpg"))
		sprintf(buf, "Content-Type: image/jpg\n");
	else if (!strcmp(content_type, "gif"))
		sprintf(buf, "Content-Type: image/gif\n");
	else if (!strcmp(content_type, "mp3"))
		sprintf(buf, "Content-Type: audio/mpeg-3\n");
	else if (!strcmp(content_type, "mp4"))
		sprintf(buf, "Content-Type: video/mpeg\n");
	else if (!strcmp(content_type, "avi"))
		sprintf(buf, "Content-Type: video/avi\n");
	else if (!strcmp(content_type, "css"))
		sprintf(buf, "Content-Type: text/css\n");
//    printf("%s",buf);
    write(fd,buf,sizeof(buf));
    bzero(buf,sizeof(buf));
    sprintf(buf,"\r\n");
    write(fd,buf,sizeof(buf));
    return 0;
}
