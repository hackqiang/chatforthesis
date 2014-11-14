/////////////////////////////////////////////
//     filename: 	client.c                              //
//     created date:  2010/03/02               //
//     by hackqiang                                   //
////////////////////////////////////////////


#include "client.h"
#include "login.h"


int flag=0;					//登陆标志
GSList *userlist=NULL;			//在线用户列表
char currentname[16]={0};		//当前登陆用户
struct sockaddr_in server_addr;	//服务器地址信息
struct sockaddr_in listen_addr;		//客户端监听地址信息
unsigned short int PORT;		//UDP使用的监听端口
int udp_send_fd;				//发送数据使用的套接字句柄
int udp_listen_fd;				//监听使用的字句柄

GtkWidget *clist;
GtkWidget *window;
GtkWidget *item_addfriends,*item_delfriends,*item_savechatlog,*item_login;
GtkStatusIcon *trayIcon;

void savechatlog(gpointer data)
{
	FILE *chatlog;
	char file_name[32]={0};
	GSList* tmp = userlist;
	USER_INFO *p;
	while(tmp)
	{
		p=(USER_INFO *)tmp->data;
		memset(file_name,0,32);
		sprintf(file_name,"./chatlog/%s.log",p->name);
		chatlog=fopen(file_name,"a+");
		fprintf(chatlog,"%s", p->chatlog->str);
		fflush(chatlog);
		fclose(chatlog);
		tmp = g_slist_next(tmp);
	}
	GtkWidget *dialog_info;
	dialog_info=gtk_message_dialog_new(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"导出成功");
	gtk_dialog_run(GTK_DIALOG(dialog_info));
	gtk_widget_destroy(dialog_info);

}

int updatefriends()
{
	int index=0;	
	gchar *drink[128][1];
	GSList *tmp = userlist;
	char buff[24]={0};
	USER_INFO *p;
	while(tmp)
	{
		p=(USER_INFO *)tmp->data;
		drink[index][0]=g_malloc0(24*sizeof(gchar));
		if(p->online==0)
		{
			if(p->count==0)
			{
				sprintf(buff,"%s (offline)",p->name);
				memcpy(drink[index][0],buff,strlen(buff));
			}
			else
			{
				sprintf(buff,"%s (offline) (%d)",p->name,p->count);
				memcpy(drink[index][0],buff,strlen(buff));
			}
		index++;
		}
		tmp = g_slist_next(tmp);
	}
	tmp = userlist;
	while(tmp)
	{
		p=(USER_INFO *)tmp->data;
		drink[index][0]=g_malloc0(24*sizeof(gchar));
		if(p->online==1)
		{
			if(p->count==0)
			{
				memcpy(drink[index][0],p->name,strlen(p->name));
			}
			else
			{
				sprintf(buff,"%s (%d)",p->name,p->count);
				memcpy(drink[index][0],buff,strlen(buff));
			}
		index++;
		}
		
		tmp = g_slist_next(tmp);
	}	

	index--;
	gtk_clist_clear( (GtkCList *) clist);
	for( ; index >=0 ; index-- )
	{
//		printf("check %d %s\n",index,drink[index][0]);
		gtk_clist_append( (GtkCList *) clist, drink[index]);
	}

	gtk_widget_show(clist);
	return 0;
}

void dealmessage(ARGS *args)
{

	MESSAGE *msg=(MESSAGE *)(args->buff);;
	USER_INFO *user;
	GSList *tmp;
	int i;
	char t_name[16]={0};

	time_t td;
	char timebuff[64]={0};
	
	gdk_threads_enter();

	if( 0 == msg->type )
	{
		tmp = userlist;
		user=(USER_INFO *)tmp->data;
		while ( tmp != NULL && strcmp(msg->content.userinfo.name,user->name) )
		{
			user=(USER_INFO *)tmp->data;
			tmp = g_slist_next(tmp);
		}
//		userlist=g_slist_remove(userlist,(gconstpointer)user);
		user->online=0;
		memset(&user->addr,0,sizeof(struct sockaddr_in));			
		updatefriends();
//		fprintf(stdout," System Message : %s offline\n",user->name);
	}
	else if( 1 == msg->type)
	{

		tmp = userlist;
		user=(USER_INFO *)tmp->data;
		while ( tmp != NULL && strcmp(msg->content.userinfo.name,user->name) )
		{
			user=(USER_INFO *)tmp->data;
			tmp = g_slist_next(tmp);
		}
		if( !strcmp(msg->content.userinfo.name,user->name) )
		{
			user->online=1;
			memset(&user->addr,0,sizeof(struct sockaddr_in));
			memcpy(&user->addr,&(msg->content.userinfo.addr),sizeof(struct sockaddr_in));
		}
		else
		{
			user=g_malloc0(sizeof(USER_INFO));
			memcpy(user,&(msg->content.userinfo),sizeof(USER_INFO));
			user->chatlog=g_string_new("chat log:\n");
			user->count=0;
			userlist=g_slist_append(userlist,(gpointer)user);	
		}
		updatefriends();
//		fprintf(stdout," System Message : %s online\n",msg->content.userinfo.name);

	}
	else if( 2==msg->type )
	{
		time(&td);
		strftime(timebuff,sizeof(timebuff),"%H:%M:%S",localtime(&td));
		i=0;
		while( msg->content.message[i] !=':' )
			i++;
		memset(t_name,0,16);
		strncpy(t_name,msg->content.message,i);
		tmp = userlist;
		user=(USER_INFO *)tmp->data;
		while ( tmp != NULL && strcmp(t_name,user->name) )
		{
			user=(USER_INFO *)tmp->data;
			tmp = g_slist_next(tmp);
		}			
		g_string_append_printf(user->chatlog,"%s %s",timebuff,msg->content.message);
		user->count++;
		
		updatefriends();
		
		gtk_status_icon_set_from_file(trayIcon, "./source/trayico1.png");
		gtk_status_icon_set_blinking (trayIcon, TRUE);
		
//		fprintf(stdout,"%s : %s\n",user->name,msg->content.message);
//		printf("from=%s\n",t_name);
//		printf("%s  chatlog=%s\n",user->name,user->chatlog->str);
	}
	else if( 7==msg->type )//系统消息
	{
//		GtkWidget *dialog_info;
//		dialog_info=gtk_message_dialog_new(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"%s",msg->content.message);
//		gtk_dialog_run(GTK_DIALOG(dialog_info));
//		gtk_widget_destroy(dialog_info);
	}
	
	gdk_threads_leave();

}


static void trayView(GtkMenuItem *item, gpointer window)
{
    gtk_widget_show(GTK_WIDGET(window));
    gtk_window_deiconify(GTK_WINDOW(window));   
}


static void trayIconActivated(GObject *trayIcon, gpointer window)
{
    gtk_widget_show(GTK_WIDGET(window));
    gtk_window_deiconify(GTK_WINDOW(window));
}

static void trayIconPopup(GtkStatusIcon *status_icon, guint button, guint32 activate_time, gpointer popUpMenu)
{
    gtk_menu_popup(GTK_MENU(popUpMenu), NULL, NULL, gtk_status_icon_position_menu, status_icon, button, activate_time);
}

static gboolean window_state_event (GtkWidget *widget, GdkEventWindowState *event, gpointer trayIcon)
{
    if(event->changed_mask == GDK_WINDOW_STATE_ICONIFIED && (event->new_window_state == GDK_WINDOW_STATE_ICONIFIED || event->new_window_state == (GDK_WINDOW_STATE_ICONIFIED | GDK_WINDOW_STATE_MAXIMIZED)))
    {
        gtk_widget_hide (GTK_WIDGET(widget));
        gtk_status_icon_set_visible(GTK_STATUS_ICON(trayIcon), TRUE);
        gtk_status_icon_set_blinking (trayIcon, FALSE);
    }
    else if(event->changed_mask == GDK_WINDOW_STATE_WITHDRAWN && (event->new_window_state == GDK_WINDOW_STATE_ICONIFIED || event->new_window_state == (GDK_WINDOW_STATE_ICONIFIED | GDK_WINDOW_STATE_MAXIMIZED)))
    {
    	gtk_status_icon_set_from_file(trayIcon, "./source/trayico2.png");
        gtk_status_icon_set_visible(GTK_STATUS_ICON(trayIcon), FALSE);
    }
    return TRUE;
}

void cancel(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy((GtkWidget *)data);
}


void selection_made( GtkWidget *clist,gint row,gint column,GdkEventButton *event,gpointer data )
{
	gchar *text;
	gtk_clist_get_text(GTK_CLIST(clist), row, column, &text);

	char buff[30]={0};
	char name[16]={0};
	memset(buff,0,30);
	strncpy(buff,text,strlen(text));
	int j=0;
	while(buff[j]!=' ' && j<strlen(text) )
		j++;
	strncpy(name,buff,j);
	USER_INFO *userinfo=finduserinfo(name);
	userinfo->count=0;
	updatefriends();
	chatwithbuddy_dialog(name);
}

void about(gpointer data)
{

	gtk_show_about_dialog (NULL,
                       "program-name", "GLchat v1.00\nCopyLeft 2010 for Qiang's Thesis\nBy hackqiang",
                       "title" ,"About GLchat",
                       NULL);
}

void quit(gpointer data)
{
	if(flag==1)
	{
	USER_INFO *p;
	MESSAGE msg;
	memset( &msg,0,sizeof(MESSAGE) );
	msg.type=0;
	memcpy( &msg.content.userinfo.name,currentname,strlen(currentname) );
	char buff[1024]={0};
	memcpy( buff,&msg,sizeof(MESSAGE) );

	GSList* tmp = userlist;
	while(tmp)
	{
		p=(USER_INFO *)tmp->data;
		if(p->online)
			sendto( udp_send_fd,buff,sizeof(buff), 0 ,(struct sockaddr *)&(p->addr) , sizeof(p->addr) ) ;
//		printf("off send to %s\n",p->name);
		tmp = g_slist_next(tmp);
	}
	sendto( udp_send_fd,buff,sizeof(buff), 0 ,(struct sockaddr *)&(server_addr) , sizeof(server_addr) ) ;
	}
	g_slist_free(userlist);
	close(udp_send_fd);
	close(udp_listen_fd);
//	printf("quit\n");
	gtk_main_quit();
}

GdkPixbuf *create_pixbuf(const gchar * filename)
{
   GdkPixbuf *pixbuf;
   GError *error = NULL;
   pixbuf = gdk_pixbuf_new_from_file(filename, &error);
   if(!pixbuf) {
      fprintf(stderr, "%s\n", error->message);
      g_error_free(error);
   }

   return pixbuf;
}

int main( int argc,gchar *argv[] )
{
	DEALFRIENDS_DATA *deal_add=g_malloc0(sizeof(DEALFRIENDS_DATA));
	DEALFRIENDS_DATA *deal_del=g_malloc0(sizeof(DEALFRIENDS_DATA));
	deal_add->type=3;
	deal_del->type=4;

	GtkWidget *vbox;
	GtkWidget *scrolled_window;
	GtkWidget *menu_bar;
	GtkWidget *menu1,*item_signup,*item_exit,*root_menu1;
	GtkWidget *menu2,*root_menu2;
	GtkWidget *menu3,*item_help,*item_about,*root_menu3;
	GtkWidget *sep1,*sep2;

	gchar *titles[1]={"好友列表"};
	if (!g_thread_supported())
		g_thread_init(NULL);
	gtk_init(&argc, &argv);
		
	udp_send_fd=socket(AF_INET,SOCK_DGRAM,0);
	char *addr=SERVER_IP;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	inet_aton(addr,&server_addr.sin_addr);
	server_addr.sin_port=htons(SERVER_PORT);	

	sep1 = gtk_separator_menu_item_new();
	sep2 = gtk_separator_menu_item_new();
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_usize(GTK_WIDGET(window), 300, 500);
//	gtk_window_set_title(GTK_WINDOW(window), "not login");
	gtk_window_set_icon(GTK_WINDOW(window), create_pixbuf ("./source/icon.png"));
	gtk_signal_connect(GTK_OBJECT(window),"destroy",G_CALLBACK (quit), NULL);
 
	vbox=gtk_vbox_new(FALSE, 6);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show(vbox);
	
    trayIcon  = gtk_status_icon_new_from_file ("./source/trayico2.png");
    //set popup traymenu for tray icon
    GtkWidget *traymenu, *menuItemView, *menuItemExit;
    traymenu = gtk_menu_new();
    menuItemView = gtk_menu_item_new_with_label ("打开GLchat");
    menuItemExit = gtk_menu_item_new_with_label ("退出");
    g_signal_connect (G_OBJECT (menuItemView), "activate", G_CALLBACK (trayView), window);
    g_signal_connect (G_OBJECT (menuItemExit), "activate", G_CALLBACK (quit), NULL);
    gtk_menu_shell_append (GTK_MENU_SHELL (traymenu), menuItemView);
    gtk_menu_shell_append (GTK_MENU_SHELL (traymenu), menuItemExit);
    gtk_widget_show_all (traymenu);

    g_signal_connect(GTK_STATUS_ICON (trayIcon), "activate", GTK_SIGNAL_FUNC (trayIconActivated), window);
    g_signal_connect(GTK_STATUS_ICON (trayIcon), "popup-menu", GTK_SIGNAL_FUNC (trayIconPopup), traymenu);
    gtk_status_icon_set_visible(trayIcon, FALSE); //set icon initially invisible

    g_signal_connect (G_OBJECT (window), "window-state-event", G_CALLBACK (window_state_event), trayIcon);
    
    gtk_status_icon_set_blinking (trayIcon, TRUE);

	//菜单1开始
	menu1 = gtk_menu_new ();
	item_login = gtk_menu_item_new_with_label ("登陆");
	item_signup = gtk_menu_item_new_with_label ("注册");
	item_exit = gtk_menu_item_new_with_label ("退出");

	gtk_menu_append (GTK_MENU (menu1), item_login);
	gtk_menu_append (GTK_MENU (menu1), item_signup);
	gtk_menu_append(GTK_MENU(menu1), sep1);
	gtk_menu_append (GTK_MENU (menu1), item_exit);  
	gtk_widget_show (item_login);
	gtk_widget_show (item_signup);
	gtk_widget_show (sep1);
	gtk_widget_show (item_exit);
	g_signal_connect_swapped (G_OBJECT (item_login), "activate", G_CALLBACK (getnamepwd_dialog),NULL);
	g_signal_connect_swapped (G_OBJECT (item_signup), "activate", G_CALLBACK (signup_dialog),NULL);
	g_signal_connect_swapped (G_OBJECT (item_exit), "activate",G_CALLBACK (quit),(gpointer) "quit");


	root_menu1 = gtk_menu_item_new_with_label ("系统");
	gtk_widget_show (root_menu1);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (root_menu1), menu1);

	//菜单2开始
	menu2 = gtk_menu_new ();
	item_addfriends = gtk_menu_item_new_with_label ("添加好友");
	item_delfriends = gtk_menu_item_new_with_label ("删除好友");
	item_savechatlog = gtk_menu_item_new_with_label ("导出聊天记录");
	gtk_menu_append (GTK_MENU (menu2), item_addfriends);
	gtk_menu_append (GTK_MENU (menu2), item_delfriends);
	gtk_menu_append(GTK_MENU(menu2), sep2);
	gtk_menu_append (GTK_MENU (menu2), item_savechatlog);
	gtk_widget_show(item_addfriends);
	gtk_widget_show(item_delfriends);
	gtk_widget_show(sep2);
	gtk_widget_show(item_savechatlog);
	g_signal_connect_swapped (G_OBJECT (item_addfriends), "activate", G_CALLBACK (dealfriends_dialog),(gpointer)deal_add);
	g_signal_connect_swapped (G_OBJECT (item_delfriends), "activate", G_CALLBACK (dealfriends_dialog),(gpointer)deal_del);
	g_signal_connect_swapped (G_OBJECT (item_savechatlog), "activate", G_CALLBACK (savechatlog),NULL);

    	root_menu2 = gtk_menu_item_new_with_label ("好友");
	gtk_widget_show (root_menu2);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (root_menu2), menu2);
 
 	//菜单3开始
 	menu3 = gtk_menu_new ();
	item_help = gtk_menu_item_new_with_label ("帮助");
	item_about = gtk_menu_item_new_with_label ("关于..");
	gtk_menu_append (GTK_MENU (menu3), item_help);
	gtk_menu_append (GTK_MENU (menu3), item_about);
	gtk_widget_show(item_help);
	gtk_widget_show(item_about);
	g_signal_connect_swapped (G_OBJECT (item_about), "activate", G_CALLBACK (about),NULL);
	g_signal_connect_swapped (G_OBJECT (item_help), "activate", G_CALLBACK (about),NULL);

    	root_menu3 = gtk_menu_item_new_with_label ("帮助");
	gtk_widget_show (root_menu3);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (root_menu3), menu3);

	/* 创建一个菜单栏以包含菜单，并将它加到主窗口 */
	menu_bar = gtk_menu_bar_new ();
	gtk_box_pack_start (GTK_BOX (vbox), menu_bar, FALSE, FALSE, 2); //////////
	gtk_widget_show (menu_bar);   
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), root_menu1);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), root_menu2);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), root_menu3);
   
	gtk_widget_get_display(item_addfriends);
   
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
	gtk_widget_show (scrolled_window);

	clist = gtk_clist_new_with_titles( 1, titles);
	gtk_signal_connect(GTK_OBJECT(clist), "select_row",GTK_SIGNAL_FUNC(selection_made),NULL);

	gtk_clist_set_column_width (GTK_CLIST(clist), 0, 150);
	gtk_container_add(GTK_CONTAINER(scrolled_window), clist);

	gtk_widget_show(clist);
	gtk_widget_show(window);

	gtk_widget_set_sensitive(item_addfriends,FALSE);
	gtk_widget_set_sensitive(item_delfriends,FALSE);
	gtk_widget_set_sensitive(item_savechatlog,FALSE);
	
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	return(0);
}
