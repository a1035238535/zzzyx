#ifndef HTTP_H
#define HTTP_H

int send_404(int fd);
int send_head(int fd,long int len,char* content_type);
void HTTPServer(int *sfd,char* arg) ;

#endif
