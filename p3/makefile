#########################
TARGET = main
#########################

DEBUG=-Wall -g
RELEASE=

CFLAGS=$(DEBUG)
DFLAGS=
LDFLAGS=-lm

CC=mpicc

OBJS=$(TARGET).o

#############################################################

main: $(OBJS)
	$(CC) $(CFLAGS) $(DFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

c: clean
	
clean:
	rm -rf $(OBJS) $(TARGET)