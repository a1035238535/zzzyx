#ifndef WRAP_H
#define WRAP_H

void perr_exit(const char *s) ;

int Accept(int fd,struct sockaddr_in *sa,socklen_t *salen) ;

int Bind(int fd,const struct sockaddr_in *sa, socklen_t salen) ;

int Connect(int fd,const struct sockaddr_in *sa,socklen_t salen) ;

int Listen(int fd,int backlog) ;

int Socket(int family, int type,int protocol) ;

#endif
