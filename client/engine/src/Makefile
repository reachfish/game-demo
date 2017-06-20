CC=g++
CFLAGS=-Wall -I../include
TARGET=../../_engine
SRCS=main.cpp net_stream.cpp io_service.cpp message.cpp msg_handle.cpp
LDFLAGS = -L../lib
DLIBS = -lpthread
OBJS = $(SRCS:.cpp=.o)

$(TARGET):$(OBJS)
	$(CC)  -o $@ $(OBJS) $(LDFLAGS) $(DLIBS)


%.o:%.cpp
	echo $<
	$(CC)  $(CFLAGS) -o $@ -c $<

.PHONY:clean
clean:
	rm -rf $(TARGET) $(OBJS)
