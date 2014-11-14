/////////////////////////////////////////////
//     filename:  common.h                        //
//     created date:  2009/12/15               //
//     by hackqiang                                   //
////////////////////////////////////////////

#ifndef COMMON_H
#define COMMON_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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
#include <mysql.h>

#include "../config.h"

//数据库中储存的用户结构
//	create table userinfo (
//	uid int AUTO_INCREMENT PRIMARY KEY,
//	name char(16) NOT NULL UNIQUE,
//	pwd char(16) NOT NULL,
//	friends char(128) DEFAULT 0
//	offlinemessage
//	);

//数据库中储存的信息结构
//	create table offlinemsg (
//	uid int NOT NULL,
//	msg varchar(1024) NOT NULL
//	);

//服务器存储的在线用户信息结构
typedef struct USER_INFO
{
	int uid;
	int online;
	char name[16];	//用户昵称
	char friends[128];
	struct sockaddr_in addr;		//用户地址信息
	void *void_block;
	int int_block;
}USER_INFO;

//服务端和客户端存储的用户表结构
typedef struct USER_NODE
{
	USER_INFO userInfo;
	struct USER_NODE *next;
}USER;

//通讯信息结构
typedef struct MESSAGE
{
	int type;			//0表示下线，1表示上线，2表示聊天内容 ,3表示加好友 ，4表示删除好友，5表示离线消息，6表示申请帐号。
	union
	{
		USER_INFO userinfo;
		char message[512];
	}content;
}MESSAGE;


typedef struct clithreadargs
{
  int connfd;
  MESSAGE message;
  struct sockaddr_in *addr;
}clithreadargs;

USER *initUserTable();
int addUser(USER *userTable,USER *user);
int delUser(USER *userTable,const char *name);
int isexistUser(USER *userTable,const char *name);
int displayUser(USER *userTable);
int userinfo(const struct USER_INFO *user);
int isonline(USER *userTable,const char *name);

int addMsg(const char *msg);


int checkUser(const char *buff,USER_INFO *userinfo);
int isfriends(int uid,const char *friends);
int adduidtofriends(int uid,char *friends);
int deluidfromfriends(int uid,char *friends);
int addfriend(const char *buf);
int delfriend(const char *buf);
int signup(const char *buf);


int noticeUsers(int fd,USER *userTable,const USER_INFO *user,const char *friends);
int sendoffmsg(int fd,USER_INFO *user);
int senduserTable(USER *userTable,int connfd,const char *friends);

#endif
