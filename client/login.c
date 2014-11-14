/////////////////////////////////////////////
//     filename: 	login.c                              //
//     created date:  2010/03/02               //
//     by hackqiang                                   //
////////////////////////////////////////////


#include "client.h"
#include "login.h"

extern int flag;
extern GSList *userlist;
extern struct sockaddr_in server_addr;
extern struct sockaddr_in listen_addr;
extern unsigned short int PORT;	
extern GtkWidget *clist;
extern char currentname[16];
extern int udp_listen_fd;
extern GtkWidget *window;
extern GtkWidget *item_addfriends,*item_delfriends,*item_savechatlog,*item_login;
extern GtkStatusIcon *trayIcon;

void initlisten(void)
{
	memset(&listen_addr,0,sizeof(listen_addr));	
	listen_addr.sin_family=AF_INET;
	listen_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	listen_addr.sin_port=htons(PORT);
	udp_listen_fd=socket(AF_INET,SOCK_DGRAM,0);
	if( bind(udp_listen_fd,(struct sockaddr *)&listen_addr,sizeof(listen_addr)) )
		perror("bind");	
	
	socklen_t len;
	char buff[1024]={0};
	struct sockaddr_in recvaddr;
	len=sizeof(recvaddr);
	ARGS *args;
	while( 1 )
	{
		args=g_malloc0(sizeof(ARGS));
		memset(buff,0,1024);	
		recvfrom(udp_listen_fd,buff,1024,0,(struct sockaddr *)&recvaddr,&len);
		memcpy(args->buff,buff,1024);
		g_thread_create( (GThreadFunc)dealmessage,args,FALSE,NULL);
	}
}

int recvuserList(int tcpfd)
{

	int n;
	char recv_buff[1024]={0};
	USER_INFO *user;
	
	n=read(tcpfd,recv_buff,sizeof(USER_INFO));
	while( strncmp(recv_buff,"endofuser",9) )
	{
		user=g_malloc0(sizeof(USER_INFO));
		memcpy(user,recv_buff,sizeof(USER_INFO));
		
		user->chatlog=g_string_new("chat log:\n");
		user->count=0;
		userlist=g_slist_append(userlist,(gpointer)user);
		memset(recv_buff,0,1024);
		n=read(tcpfd,recv_buff,sizeof(USER_INFO));	
	}
	close(tcpfd);
	updatefriends();
	return 0;
}


void getnamepwd(GtkWidget *widget, gpointer data)
{
	LOGIN_DATA *info=(LOGIN_DATA *)data;
	const gchar  *str1=gtk_entry_get_text( (GtkEntry *)info->e1 );
	const gchar  *str2=gtk_entry_get_text( (GtkEntry *)info->e2 );
	char name[16]={0};
	char pwd[16]={0};
	strncpy(name,str1,strlen(str1));
	strncpy(pwd,str2,strlen(str2));

	/////通讯	
	int tcpfd,n;
	socklen_t len;
	len=sizeof(server_addr);

	char send_buff[33]={0};
	char recv_buff[64]={0};
	strcat(send_buff,name);
	strcat(send_buff,":");
	strcat(send_buff,pwd);
	
	tcpfd=socket(AF_INET,SOCK_STREAM,0);
	connect(tcpfd,(struct sockaddr *)&server_addr,len);
	write(tcpfd,send_buff,strlen(send_buff));
	n=read(tcpfd,recv_buff,64 );

	GtkWidget *dialog_info;
	if( !strncmp(recv_buff,"success@",8) )
	{
		PORT=atoi(recv_buff+8);
		g_thread_create( (GThreadFunc)initlisten,NULL,FALSE,NULL);
		recvuserList(tcpfd);		
		dialog_info=gtk_message_dialog_new(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"登陆成功");
		gtk_dialog_run(GTK_DIALOG(dialog_info));
		gtk_widget_destroy(dialog_info);
		flag=1;
		gtk_widget_set_sensitive(item_addfriends,TRUE);
		gtk_widget_set_sensitive(item_delfriends,TRUE);
		gtk_widget_set_sensitive(item_savechatlog,TRUE);
		gtk_widget_set_sensitive(item_login,FALSE);
		memset(currentname,0,16);
		strncpy(currentname,name,strlen(name));
		gtk_window_set_title(GTK_WINDOW(window), currentname);
		char traybuf[32]={0};
		sprintf(traybuf,"%s-GLchat",currentname);
		gtk_status_icon_set_tooltip (trayIcon, traybuf);
//		printf("currentname=%s\n",currentname);
		gtk_widget_destroy(info->e3);
	}
	else
	{
		dialog_info=gtk_message_dialog_new(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"登陆失败");
		gtk_dialog_run(GTK_DIALOG(dialog_info));
		gtk_widget_destroy(dialog_info);
	}	
	
}


void getnamepwd_dialog(GtkWidget *widget,gpointer data)
{
	LOGIN_DATA *login_data=g_malloc0(sizeof(LOGIN_DATA));
	GtkWidget *hbox_name,*hbox_pwd;
	GtkWidget *button_ok,*button_cancel;
	GtkWidget *editor1,*editor2;
	GtkWidget *label_name,*label_pwd;
	GtkWidget *getnamepwd_dialog;
	
	getnamepwd_dialog=gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(getnamepwd_dialog),"登陆");
	editor1 = gtk_entry_new();
        editor2 = gtk_entry_new();
        gtk_entry_set_visibility( (GtkEntry *)editor2, FALSE );
        login_data->e1=editor1;
        login_data->e2=editor2;
        login_data->e3=getnamepwd_dialog;
        button_ok = gtk_button_new_with_label("登陆");
        button_cancel = gtk_button_new_with_label("取消");
        label_name=gtk_label_new("用户昵称：");
        label_pwd=gtk_label_new("用户密码：");
   	
   	hbox_name=gtk_hbox_new(FALSE, 0);
   	hbox_pwd=gtk_hbox_new(FALSE, 0);
   	
   	gtk_box_pack_start (GTK_BOX(hbox_name),label_name,FALSE,FALSE,5);
   	gtk_box_pack_start (GTK_BOX(hbox_name),editor1,FALSE,FALSE,5);
   	gtk_box_pack_start (GTK_BOX(hbox_pwd),label_pwd,FALSE,FALSE,5);
   	gtk_box_pack_start (GTK_BOX(hbox_pwd),editor2,FALSE,FALSE,5);
   	
 	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (getnamepwd_dialog)->vbox),hbox_name, TRUE, TRUE, 0);
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG (getnamepwd_dialog)->vbox),hbox_pwd, TRUE, TRUE, 0);
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG (getnamepwd_dialog)->action_area),button_cancel, TRUE, TRUE, 0);
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG (getnamepwd_dialog)->action_area),button_ok, TRUE, TRUE, 0);

	g_signal_connect(G_OBJECT(button_ok), "clicked",G_CALLBACK(getnamepwd),login_data);
	g_signal_connect(G_OBJECT(button_cancel), "clicked",G_CALLBACK(cancel),(gpointer)getnamepwd_dialog);


	gtk_widget_show_all(getnamepwd_dialog);
	gtk_dialog_run(GTK_DIALOG(getnamepwd_dialog));

}
