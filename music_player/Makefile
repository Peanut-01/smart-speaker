Target = main
Object = main.o select.o player.o link.o socket.o device.o

CFLAGS = -Wall -I/root/alsa-arm-install/include -I/root/json-arm-install/include

LIBS = -ljson -lasound -L/root/alsa-arm-install/lib -L/root/json-arm-install/lib

CC=aarch64-none-linux-gnu-gcc

$(Target):$(Object)
	$(CC) $(Object) -o $(Target) $(CFLAGS) $(LIBS)

clean:
	rm -f *.o main


# Target = main
# Object = main.o select.o player.o link.o socket.o device.o

# CFLAGS = -Wall

# LIBS = -ljson -lasound

# CC = gcc

# $(Target):$(Object)
# 	$(CC) $(Object) -o $(Target) $(CFLAGS) $(LIBS)

# clean:
# 	rm -f *.o main