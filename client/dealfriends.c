/////////////////////////////////////////////
//     filename: 	dealfriends.c                     //
//     created date:  2010/03/02               //
//     by hackqiang                                   //
////////////////////////////////////////////

#include "client.h"
#include "dealfriends.h"

extern GSList *userlist;
extern char currentname[16];
extern int udp_send_fd;
extern struct sockaddr_in server_addr;

void dealname(GtkWidget *widget, gpointer data)
{
	DEALFRIENDS_DATA *deal_data=(DEALFRIENDS_DATA *)data;
	MESSAGE msg;
	char buff[1024]={0};
	memset( &msg,0,sizeof(MESSAGE) );
	msg.type=deal_data->type;
//	printf("msg.type=%d\n",msg.type);

	char dest_name[16]={0};
	const gchar  *str1=gtk_entry_get_text( (GtkEntry *)deal_data->e1 );
	strncpy(dest_name,str1,strlen(str1));

	strcat(msg.content.message,currentname);
	strcat(msg.content.message,":");
	strcat(msg.content.message,dest_name);
	
	memcpy(buff,&msg,sizeof(msg));
	sendto( udp_send_fd,buff,sizeof(buff), 0 ,(struct sockaddr *)&(server_addr) , sizeof(server_addr) );
	
	gtk_widget_destroy(deal_data->e2);
}

void dealfriends_dialog(gpointer data)
{

	DEALFRIENDS_DATA *deal_data=data;
	GtkWidget *button_ok,*button_cancel;
	GtkWidget *editor1;
	GtkWidget *label_info;
	GtkWidget *deal_dialog;
	deal_dialog=gtk_dialog_new();
	editor1 = gtk_entry_new();
	char *title;
	if(deal_data->type==3)
		title="添加好友";
	if(deal_data->type==4)
		title="删除好友";
		
	gtk_window_set_title(GTK_WINDOW(deal_dialog),title);
	
	deal_data->e1=editor1;
	deal_data->e2=deal_dialog;

        button_ok = gtk_button_new_with_label("确认");
        button_cancel = gtk_button_new_with_label("取消");
        
        label_info=gtk_label_new("请输入操作好友的用户名");
   
   	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (deal_dialog)->vbox),label_info, TRUE, TRUE, 0);
 	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (deal_dialog)->vbox),editor1, TRUE, TRUE, 0);

         gtk_box_pack_start (GTK_BOX (GTK_DIALOG (deal_dialog)->action_area),button_cancel, TRUE, TRUE, 0);
         gtk_box_pack_start (GTK_BOX (GTK_DIALOG (deal_dialog)->action_area),button_ok, TRUE, TRUE, 0);

	 g_signal_connect(G_OBJECT(button_ok), "clicked",G_CALLBACK(dealname),(gpointer)deal_data);
	 g_signal_connect(G_OBJECT(button_cancel), "clicked",G_CALLBACK(cancel),(gpointer)deal_dialog);

	

	gtk_widget_show_all(deal_dialog);
	
	gtk_dialog_run(GTK_DIALOG(deal_dialog));

}
