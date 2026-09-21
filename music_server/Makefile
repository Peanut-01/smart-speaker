Target = main
Object = main.o server.o database.o
CXX = g++

$(Target): $(Object)
	$(CXX) -o $(Target) $(Object) -levent -lmysqlclient -ljsoncpp


clean:
	rm -f $(Target) $(Object)
