#################################
#     filename:  Makefile                            #
#     created date:  2009/12/15                #
#     by hackqiang                                    #
#################################

TARGETS=server
CFLAGS	= -g -Wall 
LDFLAGS = -pthread -lmysqlclient

.PHONY: all clean
all: $(TARGETS)
clean:
	rm -f *.o $(TARGETS) *.log
install:
	sudo cp server /usr/local/bin/chatserver

${TARGETS}: server.o common.o
common.o: common.c common.h ../config.h
server.o: server.c server.h ../config.h

1:
	gcc  1.c -o 1 -pthread -lmysqlclient
