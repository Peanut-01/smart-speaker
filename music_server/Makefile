Target = main
Object = main.o server.o database.o player.o
CXX = g++

CXXFLAGS = -levent -lmysqlclient -ljsoncpp -std=c++11

$(Target): $(Object)
	$(CXX) -o $(Target) $(Object) $(CXXFLAGS)


clean:
	rm -f $(Target) $(Object)
