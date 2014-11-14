/////////////////////////////////////////////
//     filename:  server.c                             //
//     created date:  2009/12/15               //
//     by hackqiang                                   //
////////////////////////////////////////////

#include "common.h"
#include "server.h"

USER *userTable;		//在线用户列表

int main(int argc, char **argv)
{

	userTable=initUserTable();
	
	pthread_t tid;
	
	//开始一个新的进程监视用户下线的信息，使用UDP协议
	pthread_create(&tid,NULL,(void *)dealusermsg,NULL);
	
	int listenfd,connfd;
	socklen_t clilen;
	struct sockaddr_in addrcli,addrsrv;
	memset(&addrsrv,0,sizeof(addrsrv));
	addrsrv.sin_family=AF_INET;
	addrsrv.sin_addr.s_addr=htonl(INADDR_ANY);
	addrsrv.sin_port=htons(SERVER_PORT);
	
	listenfd=socket(AF_INET,SOCK_STREAM,0);
	bind(listenfd,(struct sockaddr *)&addrsrv,sizeof(addrsrv));

	//监听用户连接，使用TCP协议
	listen( listenfd,CLI_MAX );
	threadargs args;
	while(1)
	{
		clilen=sizeof(addrcli);
		connfd=accept(listenfd,(struct sockaddr *)&addrcli,&clilen);
		
    		args.connfd=connfd;
    		args.addr=&addrcli;
	
		pthread_create(&tid,NULL,(void *)doit,&args);
	
	}
	
	return EXIT_SUCCESS;
}



void doit(threadargs *args)
{

	FILE *srvlog;
	srvlog=fopen("chatserver.log","a+");

	time_t td;
	char timebuff[64]={0};

	char buff[32]={0};
	int n;

	time(&td);
	strftime(timebuff,sizeof(timebuff),"%H:%M:%S",localtime(&td));
	
	n=read(args->connfd,buff,16);
	
	fprintf(srvlog,"[%s] %s@%s:%u tried to login.\n",timebuff,buff,inet_ntoa(args->addr->sin_addr),(unsigned)ntohs(args->addr->sin_port) );
	fflush(srvlog);
	
	
	USER_INFO userinfo;
	if( checkUser(buff,&userinfo) )
	{
		USER user;
		char send_info[64]={0};
		memset(&user,0,sizeof(USER));
		memcpy(&user.userInfo,&userinfo,sizeof(userinfo) );
		memcpy(&user.userInfo.addr,args->addr,sizeof(struct sockaddr_in) );
		user.userInfo.online=1;
		user.next=NULL;
		
		addUser(userTable,&user);
		
		time(&td);
		strftime(timebuff,sizeof(timebuff),"%H:%M:%S",localtime(&td));
		
		fprintf(srvlog,"[%s] %s@%s:%u login success.\n",timebuff,buff,inet_ntoa(user.userInfo.addr.sin_addr),(unsigned)ntohs(user.userInfo.addr.sin_port) );
		fflush(srvlog);
		
		sprintf(send_info,"success@%u",(unsigned)ntohs(user.userInfo.addr.sin_port) );
		write( args->connfd,send_info,sizeof(send_info) );
		
		fprintf(stdout,"[%s] %s@%s:%u login.\n",timebuff,buff,inet_ntoa(user.userInfo.addr.sin_addr),(unsigned)ntohs(user.userInfo.addr.sin_port) );

		senduserTable(userTable,args->connfd,user.userInfo.friends);
		
		int udpfd;
		udpfd=socket(AF_INET,SOCK_DGRAM,0);
		noticeUsers(udpfd,userTable,&user.userInfo,user.userInfo.friends );
		sendoffmsg(udpfd,&(user.userInfo));
//		displayUser(userTable);
		close(udpfd);

	}
	else
	{
		time(&td);
		strftime(timebuff,sizeof(timebuff),"%H:%M:%S",localtime(&td));
		
		fprintf( srvlog,"[%s] %s@%s:%u  login failed : username or passwork error !\n",timebuff,buff,inet_ntoa(args->addr->sin_addr),(unsigned)ntohs(args->addr->sin_port) );
		fflush(srvlog);
		write( args->connfd,"fail",sizeof("fail") );
		
		fprintf( stdout,"[%s] %s@%s:%u  login failed : username or passwork error !\n",timebuff,buff,inet_ntoa(args->addr->sin_addr),(unsigned)ntohs(args->addr->sin_port) );

	}
	
//	displayUser(userTable);
	
	fclose(srvlog);
	close(args->connfd);
	pthread_exit(NULL);
}

void dealusermsg()
{
	char buff[1024]={0};
	char recv_buff[1024]={0};
	socklen_t len;
	int udpfd,n,i;
	struct sockaddr_in udpaddr;
	memset(&udpaddr,0,sizeof(udpaddr));
	udpaddr.sin_family=AF_INET;
	udpaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	udpaddr.sin_port=htons(SERVER_PORT);
	udpfd=socket(AF_INET,SOCK_DGRAM,0);
	bind(udpfd,(struct sockaddr *)&udpaddr,sizeof(udpaddr));

	int sendudpfd;
	sendudpfd=socket(AF_INET,SOCK_DGRAM,0);

	time_t td;
	char timebuff[64]={0};
	char dest_name[16]={0};
	char src_name[16]={0};
	int dest_uid;
	MESSAGE *msg;
	USER *p,*q;
	MESSAGE message;
	USER_INFO user;
	
	while(1)
	{
		memset(buff,0,sizeof(buff) );
		len=sizeof(udpaddr);
		
		time(&td);
		strftime(timebuff,sizeof(timebuff),"%H:%M:%S",localtime(&td));

		n=recvfrom(udpfd,recv_buff,1024,0,(struct sockaddr *)&udpaddr,&len);
		msg=(MESSAGE *)recv_buff;
		
		
		if( 0 == msg->type )//下线
		{
			delUser(userTable,msg->content.userinfo.name);
			fprintf(stdout,"[%s] %s Logout\n",timebuff,msg->content.userinfo.name);
		}
		if( 3 == msg->type )//加好友请求，信息存放在msg->content.message，格式为 "请求用户name:目标用户name"
		{
			dest_uid=addfriend(msg->content.message);

			i=0;
			while( *(msg->content.message+i) !=':' )
				i++;
			memset(src_name,0,16);
			memset(dest_name,0,16);
			strncpy(src_name,msg->content.message,i);
			strcpy(dest_name,msg->content.message+i+1);
			
//			printf("src=%s,dest=%s\n",src_name,dest_name);
			//取得目标用户信息
			p=userTable->next;
			while( (p!=NULL) && strcmp(dest_name,p->userInfo.name) )
				p=p->next;
			
			//取得请求用户信息
			q=userTable->next;
			while( (q!=NULL) && strcmp(src_name,q->userInfo.name) )
				q=q->next;
				
			if(p==NULL)
			{
//				printf("p=NULL\n");
				adduidtofriends(dest_uid,q->userInfo.friends);
				memset(&user,0,sizeof(USER_INFO));
				user.uid=dest_uid;
				user.online=0;
				strcpy(user.name,dest_name);
				memset(&message,0,sizeof(MESSAGE));
				memset(buff,0,sizeof(buff) );
				message.type=1;
				memcpy(&(message.content),&(user),sizeof(USER_INFO) );
				memcpy(buff,&message,sizeof(MESSAGE) );
//				printf("send %s to %s\n",user.name,q->userInfo.name);
				sendto( sendudpfd,buff,sizeof(buff), 0 ,(struct sockaddr *)&(q->userInfo.addr) , sizeof(struct sockaddr) );
				
				memset(&message,0,sizeof(MESSAGE));
				memset(buff,0,sizeof(buff) );
				message.type=7;
				memcpy(&(message.content),"添加好友成功",sizeof("添加好友成功") );
				memcpy(buff,&message,sizeof(MESSAGE) );
				sendto( sendudpfd,buff,sizeof(buff), 0 ,(struct sockaddr *)&(q->userInfo.addr) , sizeof(struct sockaddr) );
				continue;
			}
			else//如果目标用户在线
			{

				adduidtofriends(p->userInfo.uid,q->userInfo.friends);
				adduidtofriends(q->userInfo.uid,p->userInfo.friends);
				
//				printf("目标=%s,请求人为%s\n",p->userInfo.name,q->userInfo.name);
				//发送目标用户信息给请求用户,发送请求用户信息给目标用户
				memset(&message,0,sizeof(MESSAGE));
				memset(buff,0,sizeof(buff) );
				message.type=1;
				memcpy(message.content.message,&(p->userInfo),sizeof(USER_INFO) );
				memcpy(buff,&message,sizeof(message) );
				sendto( sendudpfd,buff,sizeof(buff), 0 ,(struct sockaddr *)&(q->userInfo.addr) , sizeof(struct sockaddr) ) ;	
			
				memset(&message,0,sizeof(MESSAGE));
				memset(buff,0,sizeof(buff) );
				message.type=7;
				memcpy(&(message.content),"添加好友成功",sizeof("添加好友成功") );
				memcpy(buff,&message,sizeof(MESSAGE) );
				sendto( sendudpfd,buff,sizeof(buff), 0 ,(struct sockaddr *)&(q->userInfo.addr) , sizeof(struct sockaddr) );
				
				
				memset(&message,0,sizeof(MESSAGE));
				memset(buff,0,sizeof(buff) );
				message.type=1;
				memcpy(message.content.message,&(q->userInfo),sizeof(USER_INFO) );
				memcpy(buff,&message,sizeof(message) );
				sendto( sendudpfd,buff,sizeof(buff), 0 ,(struct sockaddr *)&(p->userInfo.addr) , sizeof(struct sockaddr) );
			}	
		}
		if( 4 == msg->type )//删除好友请求,信息存放在msg->content.message，格式为 "请求用户name:目标用户name"
		{
			dest_uid=delfriend(msg->content.message);

			i=0;
			while( *(msg->content.message+i) !=':' )
				i++;
			strncpy(src_name,msg->content.message,i);
			strcpy(dest_name,msg->content.message+i+1);
			
			//取得目标用户信息
			p=userTable->next;
			while( (p!=NULL) && strcmp(dest_name,p->userInfo.name) )
				p=p->next;
			
			//取得请求用户信息
			q=userTable->next;
			while( (p!=NULL) && strcmp(src_name,q->userInfo.name) )
				q=q->next;			
			if(p==NULL)
			{
				deluidfromfriends(dest_uid,q->userInfo.friends);
				continue;
			}
			else//如果目标用户在线
			{

				deluidfromfriends(p->userInfo.uid,q->userInfo.friends);
				deluidfromfriends(q->userInfo.uid,p->userInfo.friends);

				//发送目标用户信息给请求用户,发送请求用户信息给目标用户
				memset(&message,0,sizeof(MESSAGE));
				memset(buff,0,sizeof(buff) );
				message.type=0;
				memcpy(message.content.message,&(p->userInfo),sizeof(USER_INFO) );
				memcpy(buff,&message,sizeof(message) );
				sendto( udpfd,buff,sizeof(buff), 0 ,(struct sockaddr *)&(q->userInfo.addr) , sizeof(struct sockaddr) ) ;
			
				memset(&message,0,sizeof(MESSAGE));
				memset(buff,0,sizeof(buff) );
				message.type=0;
				memcpy(message.content.message,&(q->userInfo),sizeof(USER_INFO) );
				memcpy(buff,&message,sizeof(message) );
				sendto( udpfd,buff,sizeof(buff), 0 ,(struct sockaddr *)&(p->userInfo.addr) , sizeof(struct sockaddr) );
								
				memset(&message,0,sizeof(MESSAGE));
				memset(buff,0,sizeof(buff) );
				message.type=7;
				memcpy(&(message.content),"删除好友成功",sizeof("删除好友成功") );
				memcpy(buff,&message,sizeof(MESSAGE) );
				sendto( sendudpfd,buff,sizeof(buff), 0 ,(struct sockaddr *)&(q->userInfo.addr) , sizeof(struct sockaddr) );
			}		
		}
		if( 5 == msg->type )//收到离线消息
		{
			addMsg(msg->content.message);

		}
		if( 6 == msg->type )//注册申请
		{
			signup(msg->content.message);
				
		}
	}

}
