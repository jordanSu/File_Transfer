lab2: lab2.o tcp.o udp.o
	gcc -o lab2 lab2.o tcp.o udp.o
tcp.o: tcp.c
	gcc -c tcp.c
lab2.o: lab2.c tcp.o udp.o
	gcc -c lab2.c
udp.o: udp.c
	gcc -c udp.c
clean:
	rm -rf *.o

