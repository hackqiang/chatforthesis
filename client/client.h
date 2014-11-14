/////////////////////////////////////////////
//     filename: 	client.h                              //
//     created date:  2010/03/02               //
//     by hackqiang                                   //
////////////////////////////////////////////

#ifndef CLIENT_H
#define CLIENT_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include<ctype.h>
#include <malloc.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include<gtk/gtk.h>

#include "../config.h"


//客户端存储的在线用户信息结构
typedef struct USER_INFO
{
	int uid;
	int online;
	char name[16];	//用户昵称
	char friends[128];
	struct sockaddr_in addr;		//用户地址信息
	GString *chatlog;		//用来存放聊天记录
	int count;				//未读消息计数
}USER_INFO;

typedef struct MESSAGE
{
	int type;			//0表示下线，1表示上线，2表示聊天内容 ,3表示加好友 ，4表示删除好友，5表示离线消息，6表示申请帐号。
	union
	{
		USER_INFO userinfo;
		char message[512];
	}content;
}MESSAGE;


typedef struct DEALFRIENDS_DATA
{
	GtkWidget *e1;
	GtkWidget *e2;
	int type;
}DEALFRIENDS_DATA;

typedef struct ARGS
{
  char buff[1024];
}ARGS;

void getnamepwd_dialog(GtkWidget *widget,gpointer data);
void chatwithbuddy_dialog(gchar *text);
void dealfriends_dialog(gpointer data);
void signup_dialog(gpointer data);

int sendtobuddy(USER_INFO *user,const gchar *msg_text);
void savechatlog(gpointer data);

int updatefriends();
USER_INFO *finduserinfo(gchar *text);
void cancel(GtkWidget *widget, gpointer data);
void dealmessage();


#endif
