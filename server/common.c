/////////////////////////////////////////////
//     filename:  common.c                        //
//     created date:  2009/12/15               //
//     by hackqiang                                   //
////////////////////////////////////////////

#include "common.h"

USER *initUserTable()
{
	USER *usertable;
	usertable=(USER *)malloc(sizeof(USER));
	memset( usertable,0,sizeof(USER) );
	usertable->next=NULL;
	return usertable;
}


int addUser(USER *userTable,USER *user)
{
	USER *p=(USER *)malloc(sizeof(USER));
	memcpy(p,user,sizeof(USER));
	p->next=userTable->next;
	userTable->next=p;
	return 0;
}


int delUser(USER *userTable,const char *name)
{
	USER *p=userTable->next;
	if(p==NULL)
		return -1;
	if( !(strcmp(name,p->userInfo.name)) )
	{
		userTable->next=p->next;
		free(p);
	}
	else
	{
		while( strcmp(name,p->next->userInfo.name) && p->next )
		{
			p=p->next;
		}
//		printf("name:|%s|\n",p->next->userInfo.name);
		if(p->next==NULL && !strcmp(name,p->userInfo.name) )
			return -1;	
		p->next=p->next->next;
		free(p->next);
	}
	return 0;
}

int addMsg(const char *msg)
{
	char buff[1024]={0};
	int uid;
	int i=0;
	while( *(msg+i)!=',' )
		i++;
	uid=atoi(msg);
	strcpy(buff,msg+i+1);
	
	MYSQL mysql;
	int t;
	char query[128];
	memset(query,0,128);
	sprintf(query,"insert into offlinemsg values(%d,'%s')",uid,buff);
	mysql_init(&mysql);
	if ( !mysql_real_connect(&mysql,DB_HOST, DB_USERNAME, DB_PWD, DB_USERDB,0,NULL,0) )
		printf( "Error connecting to database: %s\n",mysql_error(&mysql));
	t = mysql_real_query(&mysql,query,(unsigned int)strlen(query));
	if( t )
		printf("Error making query: %s\n",mysql_error(&mysql));
	
	mysql_close(&mysql);
	return 0;
}

//向uid的用户发送离线消息
int sendoffmsg(int fd,USER_INFO *user)
{
 	sleep(3);
 	MYSQL mysql;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int t;
	char buff[1024]={0};
	char query[128];
	memset(query,0,128);
	sprintf(query,"select msg from offlinemsg where uid=%d",user->uid);
	MESSAGE message;
	mysql_init(&mysql);
	if ( !mysql_real_connect(&mysql,DB_HOST, DB_USERNAME, DB_PWD, DB_USERDB,0,NULL,0) )
		printf( "Error connecting to database: %s\n",mysql_error(&mysql));
	t = mysql_real_query(&mysql,query,(unsigned int)strlen(query));
	if( t )
		printf("Error making query: %s\n",mysql_error(&mysql));
	res = mysql_store_result(&mysql);

	int i;
	while((row = mysql_fetch_row(res)))
	{
		for (i=0 ; i < mysql_num_fields(res); i++)
		{
//			printf("%s\n",row[i]);
			memset(buff,0,1024);
			memset(&message,0,sizeof(MESSAGE));
			message.type=2;
			strcpy(message.content.message,row[i]);
			memcpy(buff,&message,sizeof(MESSAGE));
			sendto(fd,buff,sizeof(buff), 0 ,(struct sockaddr *)&(user->addr) , sizeof(user->addr) ) ;
		}

	}
	mysql_free_result(res);
	
	mysql_init(&mysql);
	if ( !mysql_real_connect(&mysql,DB_HOST, DB_USERNAME, DB_PWD, DB_USERDB,0,NULL,0) )
		printf( "Error connecting to database: %s\n",mysql_error(&mysql));
	memset(query,0,128);
	sprintf(query,"delete from offlinemsg where uid=%d",user->uid);
	t = mysql_real_query(&mysql,query,(unsigned int)strlen(query));
	if( t )
		printf("Error making query: %s\n",mysql_error(&mysql));
		
	mysql_close(&mysql);
	return 0;
}

int isexistUser(USER *userTable,const char *name)
{
	USER *p;
	p=userTable->next;
	while( (p!=NULL) && strcmp(name,p->userInfo.name) )
		p=p->next;
	if(p==NULL)
		return 0;
	return -1;
}

int displayUser(USER *userTable)
{
	USER *p=userTable->next;
	if( p==NULL)
		return -1;
	write(STDOUT_FILENO,"***************User List Begin***************\n",sizeof("***************User List Begin***************\n"));
	while(p!=NULL)
	{
		userinfo(&p->userInfo);
		p=p->next;
	}
	write(STDOUT_FILENO,"****************User List End***************\n",sizeof("****************User List End***************\n"));
	return 0;
}


int userinfo(const struct USER_INFO *user)
{
	if(user==NULL)
		return -1;
	char buff[128]={0};
	sprintf(buff,"%s(%d)@%s:%u\n",user->name,user->uid,inet_ntoa(user->addr.sin_addr),(unsigned)ntohs(user->addr.sin_port) );
	write( STDOUT_FILENO,buff,sizeof(buff) );
	return 0;
}

//检测name是否在线
int isonline(USER *userTable,const char *name)
{
	USER *p=userTable->next;
	while( p!=NULL && strcmp(p->userInfo.name,name))
	{
		p=p->next;
	}
	if(p==NULL)
		return 0;
	else
		return 1;
}

//检测uid是否在friends中
int isfriends(int uid,const char *friends)
{
	char tmp[8],str_uid[8];
	int n=0,m=0,num=uid;
	int k = 0;
	int flag = 0;
	while(num)
	{
		tmp[n]=num%10+'0';
		n++;
		num=num/10;
	}
	tmp[n]=0;
	while(n>=0)
	{
		str_uid[m]=tmp[n-1];
		m++;
		n--;
	}
	str_uid[m-1]=44;
	str_uid[m]=0;

	for(n= 0; n < (int)strlen(friends); n++)
	{
		for(m = 0; m< (int)strlen(str_uid); m++)
		{
			if( str_uid[m] == friends[n + k] )
				k++;
			else
			{
				k=0;
				break;
			}
		}
        	if(k == (int)strlen(str_uid))
        	{
			flag = 1;
			break;
		}
	}
	return flag;
}

//检查用户帐号密码 buff中的格式为“用户名:密码”，验证成功后会吧把信息存放在userinfo中。
int checkUser(const char *buff,USER_INFO *userinfo)
{
	char name[16],pwd[16];
	memset(name,0,16);
	memset(pwd,0,16);
	int i=0;
	while( *(buff+i) !=':' )
		i++;	
	strncpy(name,buff,i);
	strcpy(pwd,buff+i+1);

	char query[128]={0};
//	query = "select pwd  from userinfo where name='qiang'";	
	sprintf(query,"select *  from %s where name='%s'",DB_USETABLE,name);
	
	MYSQL mysql;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int t;

	mysql_init(&mysql);
	if ( !mysql_real_connect(&mysql,DB_HOST, DB_USERNAME, DB_PWD, DB_USERDB,0,NULL,0) )
		printf( "Error connecting to database: %s\n",mysql_error(&mysql));
    
	t = mysql_real_query(&mysql,query,(unsigned int)strlen(query));
	if( t )
		printf("Error making query: %s\n",mysql_error(&mysql));
    
	res = mysql_store_result(&mysql);
	row = mysql_fetch_row(res);
	mysql_free_result(res);
	
	mysql_close(&mysql);
	
	if( !strcmp(row[2],pwd))
	{
		userinfo->uid=atoi(row[0]);
		memcpy(userinfo->name,row[1],strlen(row[1]) );
		memcpy(userinfo->friends,row[3],strlen(row[3]) );

		return 1;
	}
	else
		return 0;
}

//将uid插入到字符串friends中
int adduidtofriends(int uid,char *friends)
{
//	printf("%s\n",friends);
	
	char tmp[8],str_uid[8];
	int n=0,m=0,num=uid;
	while(num)
	{
		tmp[n]=num%10+'0';
		n++;
		num=num/10;
	}
	tmp[n]=0;
	while(n>=0)
	{
		str_uid[m]=tmp[n-1];
		m++;
		n--;
	}
	str_uid[m-1]=44;
	str_uid[m]=0;
	
	strncat(friends,str_uid,strlen(str_uid) );
	
//	printf("%s\n",friends);
	return 0;
}

//将uid从字符串friends中删除
int deluidfromfriends(int uid,char *friends)
{	
	char tmp[8],str_uid[8];
	int n=0,m=0,num=uid;
	int k = 0;
	while(num)
	{
		tmp[n]=num%10+'0';
		n++;
		num=num/10;
	}
	tmp[n]=0;
	while(n>=0)
	{
		str_uid[m]=tmp[n-1];
		m++;
		n--;
	}
	str_uid[m-1]=44;
	str_uid[m]=0;
	
	for(n= 0; n < (int)strlen(friends); n++)
	{
		for(m = 0; m< (int)strlen(str_uid); m++)
		{
			if( str_uid[m] == friends[n + k] )
				k++;
			else
			{
				k=0;
				break;
			}
		}
        	if(k == (int)strlen(str_uid))
			break;
	}
	
	char t[128]={0};
	char s[128]={0};
	strncpy(t,friends,n);
	strcpy(s,friends+n+strlen(str_uid) );
	strcpy(friends,strcat(t,s) );

	return 0;
}


//增加好友 buf的格式为"请求用户name:目标用户name"
//返回目标用户uid
int addfriend(const char *buf)
{
	int src_uid,dest_uid;
	char src_friends[128]={0};
	char dest_friends[128]={0};
	char dest_name[16]={0};
	char src_name[16]={0};
	int i=0;
	while( *(buf+i) !=':' )
		i++;	
	strncpy(src_name,buf,i);
	strcpy(dest_name,buf+i+1);
//	printf("%d\n",uid);
//	printf("%s\n",name);

	
	MYSQL mysql;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int t;
	char query[128];
	
	//首先取得目标用户uid
	memset(query,0,128);
	sprintf(query,"select *  from userinfo where name='%s'",dest_name);
	mysql_init(&mysql);
	if ( !mysql_real_connect(&mysql,DB_HOST, DB_USERNAME, DB_PWD, DB_USERDB,0,NULL,0) )
		printf( "Error connecting to database: %s\n",mysql_error(&mysql));
	t = mysql_real_query(&mysql,query,(unsigned int)strlen(query));
	if( t )
		printf("Error making query: %s\n",mysql_error(&mysql));
	res = mysql_store_result(&mysql);
	row = mysql_fetch_row(res);
	mysql_free_result(res);
	dest_uid=atoi(row[0]);
	strncpy(dest_friends,row[3],strlen(row[3]) );
	
	//取得请求用户friends和uid
	memset(query,0,128);
	sprintf(query,"select *  from userinfo where name='%s'",src_name);	
	mysql_init(&mysql);
	if ( !mysql_real_connect(&mysql,DB_HOST, DB_USERNAME, DB_PWD, DB_USERDB,0,NULL,0) )
		printf( "Error connecting to database: %s\n",mysql_error(&mysql));
	t = mysql_real_query(&mysql,query,(unsigned int)strlen(query));
	if( t )
		printf("Error making query: %s\n",mysql_error(&mysql));
	res = mysql_store_result(&mysql);
	row = mysql_fetch_row(res);
	mysql_free_result(res);
	src_uid=atoi(row[0]);
	strncpy(src_friends,row[3],strlen(row[3]) );
	
//	printf("src=%d dest=%d\n%s %s\n",src_uid,dest_uid,src_friends,dest_friends);
	
	adduidtofriends(dest_uid,src_friends);
	adduidtofriends(src_uid,dest_friends);
	
//	printf("src=%d dest=%d\n%s %s\n",src_uid,dest_uid,src_friends,dest_friends);
	
	//向数据库中增加好友信息
	memset(query,0,128);
	sprintf(query,"update userinfo set friends='%s' where uid=%d",src_friends,src_uid);	
	mysql_init(&mysql);
	if ( !mysql_real_connect(&mysql,DB_HOST, DB_USERNAME, DB_PWD, DB_USERDB,0,NULL,0) )
		printf( "Error connecting to database: %s\n",mysql_error(&mysql));
	t = mysql_real_query(&mysql,query,(unsigned int)strlen(query));
	if( t )
		printf("Error making query: %s\n",mysql_error(&mysql));
	
	memset(query,0,128);
	sprintf(query,"update userinfo set friends='%s' where uid=%d",dest_friends,dest_uid);
	mysql_init(&mysql);
	if ( !mysql_real_connect(&mysql,DB_HOST, DB_USERNAME, DB_PWD, DB_USERDB,0,NULL,0) )
		printf( "Error connecting to database: %s\n",mysql_error(&mysql));
	t = mysql_real_query(&mysql,query,(unsigned int)strlen(query));
	if( t )
		printf("Error making query: %s\n",mysql_error(&mysql));	
	
	mysql_close(&mysql);

	return dest_uid;
}

//删除好友 buf的格式为"请求用户name:目标用户name"
//返回目标用户uid
int delfriend(const char *buf)
{
	int src_uid,dest_uid;
	char src_friends[128]={0};
	char dest_friends[128]={0};
	char dest_name[16]={0};
	char src_name[16]={0};
	int i=0;
	while( *(buf+i) !=':' )
		i++;	
	strncpy(src_name,buf,i);
	strcpy(dest_name,buf+i+1);
//	printf("%d\n",uid);
//	printf("%s\n",name);

	MYSQL mysql;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int t;
	char query[128];
	
	//首先取得目标用户uid
	memset(query,0,128);
	sprintf(query,"select *  from userinfo where name='%s'",dest_name);
	mysql_init(&mysql);
	if ( !mysql_real_connect(&mysql,DB_HOST, DB_USERNAME, DB_PWD, DB_USERDB,0,NULL,0) )
		printf( "Error connecting to database: %s\n",mysql_error(&mysql));
	t = mysql_real_query(&mysql,query,(unsigned int)strlen(query));
	if( t )
		printf("Error making query: %s\n",mysql_error(&mysql));
	res = mysql_store_result(&mysql);
	row = mysql_fetch_row(res);
	mysql_free_result(res);
	dest_uid=atoi(row[0]);
	strncpy(dest_friends,row[3],strlen(row[3]) );
	
	//取得请求用户friends,uid
	memset(query,0,128);
	sprintf(query,"select *  from userinfo where name='%s'",src_name);	
	mysql_init(&mysql);
	if ( !mysql_real_connect(&mysql,DB_HOST, DB_USERNAME, DB_PWD, DB_USERDB,0,NULL,0) )
		printf( "Error connecting to database: %s\n",mysql_error(&mysql));
	t = mysql_real_query(&mysql,query,(unsigned int)strlen(query));
	if( t )
		printf("Error making query: %s\n",mysql_error(&mysql));
	res = mysql_store_result(&mysql);
	row = mysql_fetch_row(res);
	mysql_free_result(res);
	src_uid=atoi(row[0]);
	strncpy(src_friends,row[3],strlen(row[3]) );
	
//	printf("src=%d dest=%d\n%s %s\n",src_uid,dest_uid,src_friends,dest_friends);
	
	deluidfromfriends(dest_uid,src_friends);
	deluidfromfriends(src_uid,dest_friends);
	
//	printf("src=%d dest=%d\n%s %s\n",src_uid,dest_uid,src_friends,dest_friends);
	
	//向数据库中增加好友信息
	memset(query,0,128);
	sprintf(query,"update userinfo set friends='%s' where uid=%d",src_friends,src_uid);	
	mysql_init(&mysql);
	if ( !mysql_real_connect(&mysql,DB_HOST, DB_USERNAME, DB_PWD, DB_USERDB,0,NULL,0) )
		printf( "Error connecting to database: %s\n",mysql_error(&mysql));
	t = mysql_real_query(&mysql,query,(unsigned int)strlen(query));
	if( t )
		printf("Error making query: %s\n",mysql_error(&mysql));
	
	memset(query,0,128);
	sprintf(query,"update userinfo set friends='%s' where uid=%d",dest_friends,dest_uid);
	mysql_init(&mysql);
	if ( !mysql_real_connect(&mysql,DB_HOST, DB_USERNAME, DB_PWD, DB_USERDB,0,NULL,0) )
		printf( "Error connecting to database: %s\n",mysql_error(&mysql));
	t = mysql_real_query(&mysql,query,(unsigned int)strlen(query));
	if( t )
		printf("Error making query: %s\n",mysql_error(&mysql));	
	
	mysql_close(&mysql);
	
	return dest_uid;
}
//申请帐号，格式为"昵称:密码"
int signup(const char *buf)
{
	char name[16]={0};
	char pwd[16]={0};
	int i=0;
	while( *(buf+i) !=':' )
		i++;	
	strncpy(name,buf,i);
	strncpy(pwd,buf+i+1,strlen(buf)-i-1);
	MYSQL mysql;

	int t;
	char query[128]={0};
	memset(query,0,128);
	sprintf(query,"insert into %s values(NULL,'%s','%s','0,')",DB_USETABLE,name,pwd);	
	mysql_init(&mysql);
	if ( !mysql_real_connect(&mysql,DB_HOST, DB_USERNAME, DB_PWD, DB_USERDB,0,NULL,0) )
		printf( "Error connecting to database: %s\n",mysql_error(&mysql));
	t = mysql_real_query(&mysql,query,(unsigned int)strlen(query));
	if( t )
		printf("Error making query: %s\n",mysql_error(&mysql));

	mysql_close(&mysql);
	return 0;
}

//通知上线消息
int noticeUsers(int fd,USER *userTable,const USER_INFO *user,const char *friends)
{

	USER *p;
	MESSAGE msg;
	memset( &msg,0,sizeof(MESSAGE) );
	msg.type=1;
	memcpy( &msg.content,user,sizeof(USER_INFO) );
	
	char buff[1024]={0};
	memcpy( buff,&msg,sizeof(MESSAGE) );
	
	p=userTable->next;
	while(p != NULL)
	{	
		if( isfriends(p->userInfo.uid,friends) )				
		{
//			userinfo(&p->userInfo);
			sendto( fd,buff,sizeof(buff), 0 ,(struct sockaddr *)&(p->userInfo.addr) , sizeof(p->userInfo.addr) ) ;
//			sendoffmsg(fd,msgList,&(p->userInfo));
		}
		p=p->next;
	}

	return 0;
}

int senduserTable(USER *userTable,int connfd,const char *friends)
{
	USER_INFO user;
	USER *p;
	char buff[1024]={0};
	char end[]="endofuser";

	int n=0;
	int i=2;
	
	MYSQL mysql;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int t;
	char query[128];	

	while( i!=strlen(friends))
	{
		if( !strcmp(friends,"0,") )
			break;
			
		n=atoi(friends+i);
		while(*(friends+i)!=',' && i<strlen(friends))
			i++;
		i++;
		//找出好友
		memset(query,0,128);
		sprintf(query,"select *  from userinfo where uid='%d'",n);
		mysql_init(&mysql);
		if ( !mysql_real_connect(&mysql,DB_HOST, DB_USERNAME, DB_PWD, DB_USERDB,0,NULL,0) )
			printf( "Error connecting to database: %s\n",mysql_error(&mysql));
		t = mysql_real_query(&mysql,query,(unsigned int)strlen(query));
		if( t )
			printf("Error making query: %s\n",mysql_error(&mysql));
		res = mysql_store_result(&mysql);
		row = mysql_fetch_row(res);
		mysql_free_result(res);
		memset(&user,0,sizeof(USER_INFO));
		user.uid=atoi(row[0]);
		strncpy(user.name,row[1],strlen(row[1]) );
		
		p=userTable;
		if( isonline(userTable,user.name) )
		{
			user.online=1;
			while(p != NULL && strcmp(user.name,p->userInfo.name))
			{	
				p=p->next;
			}
			memcpy( &user,&(p->userInfo),sizeof(USER_INFO) );
		}		
		else
			user.online=0;
		
		memcpy(buff,&user,sizeof(USER_INFO) );
		write( connfd,buff,sizeof(USER_INFO) );
		
//		printf("send user %s\n",p->userInfo.name);
	}


	write( connfd,end,strlen(end) );
	
	return 0;
}
