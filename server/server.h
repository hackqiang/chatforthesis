/////////////////////////////////////////////
//     filename:  server.h                             //
//     created date:  2009/12/15               //
//     by hackqiang                                   //
////////////////////////////////////////////

#ifndef SERVER_H
#define SERVER_H

typedef struct threadargs
{
  int connfd;
  struct sockaddr_in *addr;
}threadargs;


void dealusermsg();
void doit(threadargs *args);

#endif
