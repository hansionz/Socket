.PHONY:all
all:dict_client dict_server

dict_client:dict_client.cc tcp_client.hpp tcp_socket.hpp
	g++ -o $@ -std=c++11 $^
dict_server:dict_server.cc tcp_pthread_server.hpp  tcp_socket.hpp
	g++ -o $@ -std=c++11 $^ -lpthread
.PHONY:clean
clean:
	rm -f dict_client dict_server
