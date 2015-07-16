all:	udpEsp8266 udpServer
CC=g++

udpEsp8266: socketUdpEsp8266Ap/udpEsp.cpp
	$(CC) -o udpEsp8266 socketUdpEsp8266Ap/udpEsp.cpp -I.
	
udpServer: socketUdpServer/udpServer.cpp
	$(CC) -o udpServer socketUdpServer/udpServer.cpp -I.

clean:
	rm udpEsp8266
	rm udpServer
