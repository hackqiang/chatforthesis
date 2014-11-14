/////////////////////////////////////////////
//     filename: 	chatwithbuddy.h               //
//     created date:  2010/03/02               //
//     by hackqiang                                   //
////////////////////////////////////////////


#ifndef CHATWUTHBUDDY_H
#define CHATWUTHBUDDY_H

typedef struct SEND_DATA
{
	GtkWidget *Send_textview;
	GtkWidget *Rcv_textview;
	GtkTextBuffer *Send_buffer;
	GtkTextBuffer *Rcv_buffer;
	USER_INFO *userinfo;
	GtkWidget *window;
}SEND_DATA;

typedef struct MUTIL_AGRS
{
	USER_INFO *userinfo;
	GtkTextBuffer *Send_buffer;
	GtkTextBuffer *Rcv_buffer;
}MUTIL_AGRS;


#endif
