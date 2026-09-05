Target=asr
Objects=alsa.o main.o

LDLIBS=-lasound

$(Target): $(Objects)
	gcc $(Objects) -o $(Target) $(LDLIBS)

clean:
	rm -f $(Target) *.o
