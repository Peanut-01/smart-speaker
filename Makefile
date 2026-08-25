Target = main
Object = main.o select.o player.o link.o socket.o device.o

CFLAGS = -Wall -g

LIBS = -ljson -lasound

$(Target):$(Object)
	gcc $(Object) -o $(Target) $(CFLAGS) $(LIBS)

clean:
	rm -f *.o main