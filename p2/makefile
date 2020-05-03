#########################
TARGET = main
#########################

DEBUG=-Wall -O0 -g
RELEASE=-Wall -O3

CFLAGS=$(DEBUG)
DFLAGS=
LDFLAGS=

CC=mpicc

OBJS=$(TARGET).o

#############################################################

main: $(OBJS)
	$(CC) $(CFLAGS) $(DFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

c: clean
	
clean:
	rm -rf $(OBJS) $(TARGET)