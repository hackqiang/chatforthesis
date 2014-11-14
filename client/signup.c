/////////////////////////////////////////////
//     filename: 	signup.c                            //
//     created date:  2010/03/02               //
//     by hackqiang                                   //
////////////////////////////////////////////


#include "client.h"
#include "signup.h"


extern struct sockaddr_in server_addr;
extern int udp_send_fd;
extern GtkWidget *window;


void signup(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog_info;
	SIGNUP_DATA *info=(SIGNUP_DATA *)data;
	const gchar  *str1=gtk_entry_get_text( (GtkEntry *)info->e1 );
	const gchar  *str2=gtk_entry_get_text( (GtkEntry *)info->e2 );
	const gchar  *str3=gtk_entry_get_text( (GtkEntry *)info->e3 );
	char name[16]={0};
	char pwd1[16]={0};
	char pwd2[16]={0};
	strncpy(name,str1,strlen(str1));
	strncpy(pwd1,str2,strlen(str2));
	strncpy(pwd2,str3,strlen(str3));

	if(strcmp(pwd1,pwd2))
	{
		dialog_info=gtk_message_dialog_new(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"两次密码不一致！");
		gtk_dialog_run(GTK_DIALOG(dialog_info));
		gtk_widget_destroy(dialog_info);
	}
	char buff[33]={0};
	char send_buff[1024]={0};
	MESSAGE msg;
	memset(buff,0,33);
	memset(send_buff,0,1024);
	memset(&msg,0,sizeof(MESSAGE));
	strcat(buff,name);
	strcat(buff,":");
	strcat(buff,pwd1);
	
	msg.type=6;	
	memcpy(msg.content.message,buff,strlen(buff));
	memcpy(send_buff,&msg,sizeof(MESSAGE));
	sendto( udp_send_fd,send_buff,sizeof(send_buff), 0 ,(struct sockaddr *)&(server_addr) , sizeof(server_addr) ) ;
	gtk_widget_destroy(info->e4);
		
}


void signup_dialog(gpointer data)
{
	SIGNUP_DATA *signup_data=g_malloc0(sizeof(SIGNUP_DATA));
	GtkWidget *hbox_name,*hbox_pwd1,*hbox_pwd2;
	GtkWidget *button_ok,*button_cancel;
	GtkWidget *editor1,*editor2,*editor3;
	GtkWidget *label_name,*label_pwd1,*label_pwd2;
	GtkWidget *signup_window;
	
	signup_window=gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(signup_window),"申请帐号");
	editor1 = gtk_entry_new();
        editor2 = gtk_entry_new();
        editor3 = gtk_entry_new();
        gtk_entry_set_visibility( (GtkEntry *)editor2, FALSE );
        gtk_entry_set_visibility( (GtkEntry *)editor3, FALSE );
        signup_data->e1=editor1;
        signup_data->e2=editor2;
        signup_data->e3=editor3;
        signup_data->e4=signup_window;
        button_ok = gtk_button_new_with_label("申请");
        button_cancel = gtk_button_new_with_label("取消");
        label_name=gtk_label_new("用户昵称：");
        label_pwd1=gtk_label_new("用户密码：");
        label_pwd2=gtk_label_new("再次密码：");
   	
   	hbox_name=gtk_hbox_new(FALSE, 0);
   	hbox_pwd1=gtk_hbox_new(FALSE, 0);
   	hbox_pwd2=gtk_hbox_new(FALSE, 0);
   	
   	gtk_box_pack_start (GTK_BOX(hbox_name),label_name,FALSE,FALSE,5);
   	gtk_box_pack_start (GTK_BOX(hbox_name),editor1,FALSE,FALSE,5);
   	gtk_box_pack_start (GTK_BOX(hbox_pwd1),label_pwd1,FALSE,FALSE,5);
   	gtk_box_pack_start (GTK_BOX(hbox_pwd1),editor2,FALSE,FALSE,5);
   	gtk_box_pack_start (GTK_BOX(hbox_pwd2),label_pwd2,FALSE,FALSE,5);
   	gtk_box_pack_start (GTK_BOX(hbox_pwd2),editor3,FALSE,FALSE,5);
   	
 	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (signup_window)->vbox),hbox_name, TRUE, TRUE, 0);
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG (signup_window)->vbox),hbox_pwd1, TRUE, TRUE, 0);
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG (signup_window)->vbox),hbox_pwd2, TRUE, TRUE, 0);
         gtk_box_pack_start (GTK_BOX (GTK_DIALOG (signup_window)->action_area),button_cancel, TRUE, TRUE, 0);
         gtk_box_pack_start (GTK_BOX (GTK_DIALOG (signup_window)->action_area),button_ok, TRUE, TRUE, 0);

	 g_signal_connect(G_OBJECT(button_ok), "clicked",G_CALLBACK(signup),(gpointer)signup_data);
	 g_signal_connect(G_OBJECT(button_cancel), "clicked",G_CALLBACK(cancel),(gpointer)signup_window);


	gtk_widget_show_all(signup_window);
	gtk_dialog_run(GTK_DIALOG(signup_window));

}
