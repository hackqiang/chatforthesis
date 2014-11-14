/////////////////////////////////////////////
//     filename: 	login.h                              //
//     created date:  2010/03/02               //
//     by hackqiang                                   //
////////////////////////////////////////////

#ifndef LOGIN_H
#define LOGIN_H

typedef struct LOGIN_DATA
{
	GtkWidget *e1;
	GtkWidget *e2;
	GtkWidget *e3;
	char name[16];
	char pwd[16];
}LOGIN_DATA;


int updatefriends();
int recvuserList(int tcpfd);
void getnamepwd(GtkWidget *widget, gpointer data);
void cancel(GtkWidget *widget, gpointer data);


#endif
