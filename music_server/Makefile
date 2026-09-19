Target = main
Object = main.o server.o
CXX = g++

$(Target): $(Object)
	$(CXX) -o $(Target) $(Object) -levent


clean:
	rm -f $(Target) $(Object)
