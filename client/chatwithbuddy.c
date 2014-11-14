/////////////////////////////////////////////
//     filename: 	chatwithbuddy.c               //
//     created date:  2010/03/02               //
//     by hackqiang                                   //
////////////////////////////////////////////

#include "client.h"
#include "login.h"
#include "chatwithbuddy.h"

	extern char currentname[16];
	extern GSList *userlist;
	extern int udp_send_fd;
	extern struct sockaddr_in server_addr;
	

USER_INFO *finduserinfo(gchar *text)
{
	GSList *tmp = userlist;
	USER_INFO *userinfo=(USER_INFO *)tmp->data;
	while ( tmp != NULL && strcmp(text,userinfo->name) )
	{
		userinfo=(USER_INFO *)tmp->data;
		tmp = g_slist_next(tmp);
	}
	return userinfo;
}



int sendtobuddy(USER_INFO *user,const gchar *msg_text)
{
	MESSAGE msg;
	char buff[1024]={0};
	memset( &msg,0,sizeof(MESSAGE) );

	if(user->online==1)
	{
		msg.type=2;
//		printf("currentname=%s\n",currentname);
		sprintf(msg.content.message,"%s:\n%s\n",currentname,msg_text);
		memcpy( buff,&msg,sizeof(MESSAGE) );
		sendto( udp_send_fd,buff,sizeof(buff), 0 ,(struct sockaddr *)&(user->addr) , sizeof(user->addr) ) ;
	}
	else//离线消息，格式为"目标用户uid,正常消息"
	{
		msg.type=5;
		sprintf(msg.content.message,"%d,%s:\n%s\n",user->uid,currentname,msg_text);
		memcpy( buff,&msg,sizeof(MESSAGE) );
		sendto( udp_send_fd,buff,sizeof(buff), 0 ,(struct sockaddr *)&(server_addr) , sizeof(server_addr) ) ;
	}
//	GtkWidget *dialog_info;
//	dialog_info=gtk_message_dialog_new(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"to %s",user->name);
//	gtk_dialog_run(GTK_DIALOG(dialog_info));
//	gtk_widget_destroy(dialog_info);

	return 0;
}



void on_send(GtkWidget *widget, gpointer data)
{

	SEND_DATA *send_data=(SEND_DATA *)data;
	GtkTextIter start,end;
	
	time_t td;
	char timebuff[64]={0};
	
	gchar *send_text;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(send_data->Send_buffer),&start,&end);
	send_text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(send_data->Send_buffer),&start,&end,FALSE);

	if(strcmp(send_text,"")!=0)
	{
		if( !sendtobuddy(send_data->userinfo,send_text) )
		{
			gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(send_data->Send_buffer),&start,&end);
			gtk_text_buffer_delete(GTK_TEXT_BUFFER(send_data->Send_buffer),&start,&end);
			char buff[1024]={0};
			time(&td);
			strftime(timebuff,sizeof(timebuff),"%H:%M:%S",localtime(&td));
			sprintf(buff,"%s %s:\n%s\n",timebuff,currentname,send_text);
			send_data->userinfo->chatlog=g_string_append(send_data->userinfo->chatlog,buff); 
			gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(send_data->Rcv_buffer),&start,&end);
			gtk_text_buffer_delete(GTK_TEXT_BUFFER(send_data->Rcv_buffer),&start,&end);
			gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(send_data->Rcv_buffer),&start,&end);
			gtk_text_buffer_insert(GTK_TEXT_BUFFER(send_data->Rcv_buffer),&end,send_data->userinfo->chatlog->str,strlen(send_data->userinfo->chatlog->str));
//			printf("%s chatlog=%s\n",send_data->userinfo->name,send_data->userinfo->chatlog->str);
        	}
		else
		{
			gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(send_data->Rcv_buffer),&start,&end);
			gtk_text_buffer_insert(GTK_TEXT_BUFFER(send_data->Rcv_buffer),&end,"消息不能为空.\n",strlen("消息不能为空.\n"));
		}
	}
	else
	{
 		gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(send_data->Rcv_buffer),&start,&end);
		gtk_text_buffer_insert(GTK_TEXT_BUFFER(send_data->Rcv_buffer),&end,"消息不能为空.\n",strlen("消息不能为空.\n"));
        }
	free(send_text);
	gtk_widget_destroy(send_data->window);
}


void on_close(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(widget);
}
void on_cancel(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy( (GtkWidget *)data );
}

void chatwithbuddy_dialog(gchar *text)
{
	SEND_DATA *send_data=g_malloc0(sizeof(SEND_DATA));

	USER_INFO *userinfo=finduserinfo(text);
	
	send_data->userinfo=userinfo;

	MUTIL_AGRS *mutil_args=g_malloc0(sizeof(MUTIL_AGRS));
	mutil_args->userinfo=userinfo;
	
	GtkWidget *Send_textview,*Rcv_textview;
	GtkTextBuffer *Send_buffer,*Rcv_buffer;
	GtkWidget *window,*Send_scrolled_win,*Rcv_scrolled_win;
	GtkWidget *vbox;
	GtkWidget *Button_Box,*SendButton,*CloseButton;
	GtkWidget *hseparator;


	////////////////////////////////主窗口///////////////////////////////
	char chat_title[32]={0};
	sprintf(chat_title,"与%s聊天",text);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window),"delete_event",G_CALLBACK(on_close),(gpointer)window);
	gtk_window_set_title(GTK_WINDOW(window),chat_title);
	gtk_container_set_border_width(GTK_CONTAINER(window),1);
	gtk_widget_set_size_request(window,500,400);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
        /////////////////////////Send_text view//////////////////////////////
	Send_textview = gtk_text_view_new();/*生成text view*/
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(Send_textview),GTK_WRAP_WORD);
	gtk_text_view_set_justification(GTK_TEXT_VIEW(Send_textview),GTK_JUSTIFY_LEFT);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(Send_textview),TRUE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(Send_textview),TRUE);
        gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(Send_textview),5);
	gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(Send_textview),5);
	gtk_text_view_set_pixels_inside_wrap(GTK_TEXT_VIEW(Send_textview),5);
	gtk_text_view_set_left_margin(GTK_TEXT_VIEW(Send_textview),10);
	gtk_text_view_set_right_margin(GTK_TEXT_VIEW(Send_textview),10);
	Send_buffer =  gtk_text_view_get_buffer(GTK_TEXT_VIEW(Send_textview));
        /////////////////////////Rcv_text view//////////////////////////////
	Rcv_textview = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(Rcv_textview),GTK_WRAP_WORD);
	gtk_text_view_set_justification(GTK_TEXT_VIEW(Rcv_textview),GTK_JUSTIFY_LEFT);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(Rcv_textview),TRUE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(Rcv_textview),TRUE);
        gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(Rcv_textview),5);
	gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(Rcv_textview),5);
	gtk_text_view_set_pixels_inside_wrap(GTK_TEXT_VIEW(Rcv_textview),5);
        gtk_text_view_set_left_margin(GTK_TEXT_VIEW(Rcv_textview),10);
	gtk_text_view_set_right_margin(GTK_TEXT_VIEW(Rcv_textview),10);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(Rcv_textview),FALSE);
	Rcv_buffer =  gtk_text_view_get_buffer(GTK_TEXT_VIEW(Rcv_textview));
	GtkTextIter start,end;
    	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(Rcv_buffer),&start,&end);
    	gtk_text_buffer_delete(GTK_TEXT_BUFFER(Rcv_buffer),&start,&end);
    	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(Rcv_buffer),&start,&end);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(Rcv_buffer),&end,userinfo->chatlog->str,strlen(userinfo->chatlog->str));

	send_data->Send_textview=Send_textview;
	send_data->Rcv_textview=Rcv_textview;
	send_data->Rcv_buffer=Rcv_buffer;
	send_data->Send_buffer=Send_buffer;
	send_data->window=window;
	mutil_args->Rcv_buffer=Rcv_buffer;
	mutil_args->Send_buffer=Send_buffer;

	
	Send_scrolled_win = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(Send_scrolled_win),Send_textview);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(Send_scrolled_win),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	Rcv_scrolled_win = gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(Rcv_scrolled_win,400,200);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(Rcv_scrolled_win),Rcv_textview);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(Rcv_scrolled_win),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);	
	vbox = gtk_vbox_new(FALSE,0);
	
	SendButton = gtk_button_new_with_label("发送");
	g_signal_connect(G_OBJECT(SendButton),"clicked",G_CALLBACK(on_send),(gpointer)send_data);
	CloseButton = gtk_button_new_with_label("关闭");
	g_signal_connect(G_OBJECT(CloseButton),"clicked",G_CALLBACK(on_cancel),(gpointer)window);
	Button_Box = gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(Button_Box),1);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(Button_Box),GTK_BUTTONBOX_END);
	gtk_container_set_border_width(GTK_CONTAINER(Button_Box),2);

	hseparator = gtk_hseparator_new();

	gtk_container_add(GTK_CONTAINER(vbox),Rcv_scrolled_win);
	gtk_container_add(GTK_CONTAINER(vbox),hseparator);
	gtk_container_add(GTK_CONTAINER(vbox),Send_scrolled_win);
	gtk_container_add(GTK_CONTAINER(vbox),Button_Box);
	gtk_box_pack_start(GTK_BOX(Button_Box),CloseButton,TRUE,TRUE,5);
	gtk_box_pack_start(GTK_BOX(Button_Box),SendButton,TRUE,TRUE,5);
	gtk_container_add(GTK_CONTAINER(window),vbox);   

	gtk_widget_show_all(window);

}
