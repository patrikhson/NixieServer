CC=g++
CFLAGS=

NixieServer:	NixieServer.o
		g++ -o NixieServer NixieServer.o -lwiringPi

%.o:	%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)
