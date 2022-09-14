all : fileDir lib server client

fileDir:
	@mkdir -p bin
	@mkdir -p bin/server_files
	@mkdir -p bin/lib

jsoncpp: src/lib/json/jsoncpp.cpp src/lib/json/json/json.h src/lib/json/json/json-forwards.h
	g++ -std=c++11 src/lib/json/jsoncpp.cpp -c -o bin/lib/jsoncpp.o

libjsoncpp.a: bin/lib/jsoncpp.o
		ar rcs bin/lib/libjsoncpp.a bin/lib/jsoncpp.o

lib: jsoncpp libjsoncpp.a

server_lib: src/server.cpp src/include/server.h 
	g++ -std=c++11 src/server.cpp -c -o bin/lib/server.o

service_thread_lib: src/service_thread.cpp src/include/service_thread.h 
	g++ -std=c++11 src/service_thread.cpp -c -o bin/lib/service_thread.o

libserver.a: bin/lib/server.o bin/lib/service_thread.o
		ar rcs bin/lib/libserver.a bin/lib/server.o bin/lib/service_thread.o

server_main: src/server_main.cpp src/include/server.h
	g++ -std=c++11 src/server_main.cpp bin/lib/libserver.a bin/lib/libjsoncpp.a -lpthread -o bin/Server.out


client_lib: src/client.cpp src/include/client.h 
	g++ -std=c++11 src/client.cpp -c -o bin/lib/client.o

libclient.a: bin/lib/client.o
		ar rcs bin/lib/libclient.a bin/lib/client.o

client_main: src/client_main.cpp src/include/client.h 
	g++ -std=c++11 src/client_main.cpp bin/lib/libclient.a bin/lib/libjsoncpp.a -lpthread -o bin/Client.out

server: service_thread_lib server_lib libserver.a server_main

client: client_lib libclient.a client_main

clean : 
	rm -f -r bin/Client.out
	rm -f -r bin/Server.out
	rm -f -r bin/lib

