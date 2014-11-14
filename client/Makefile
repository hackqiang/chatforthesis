#################################
#     filename:  Makefile                            #
#     created date:  2009/12/15                #
#     by hackqiang                                    #
#################################

TARGETS=client
CFLAGS=`pkg-config --cflags gtk+-2.0` -g -Wall
LDFLAGS=`pkg-config --libs gtk+-2.0 gthread-2.0`   -g -Wall

.PHONY: all clean
all: $(TARGETS)
clean:
	rm -f *.o ${TARGETS} ./chatlog/*.log
install:
	ln -sf /home/qiang/thesis/client/client /home/qiang/桌面/GLchat

${TARGETS}:chatwithbuddy.o client.o dealfriends.o login.o signup.o
chatwithbuddy.o: chatwithbuddy.c chatwithbuddy.h client.h ../config.h
client.o:client.c client.h ../config.h
dealfriends.o:dealfriends.c dealfriends.h client.h ../config.h
login.o:login.c login.h client.h ../config.h
signup.o:signup.c signup.h client.h ../config.h
