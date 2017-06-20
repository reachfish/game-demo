CC=g++  
CFLAGS= -g -Wall -I../include -I/usr/local/include
TARGET=../../_engine
SRCS=main.cpp lua_handle.cpp net_stream.cpp msg_handle.cpp message.cpp db_handle.cpp
LDFLAGS = -L../lib -L/usr/local/lib
DLIBS = -llua -levent
OBJS = $(SRCS:.cpp=.o)

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(DLIBS)


%.o:%.cpp
	echo $<
	$(CC)  $(CFLAGS) -o $@ -c $<

.PHONY:clean
clean:
	rm -rf $(TARGET) $(OBJS)
