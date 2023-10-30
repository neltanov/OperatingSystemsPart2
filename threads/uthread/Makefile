main: main.o libuthread.so
	gcc -o main main.o -L. -luthread -Wl,-rpath,.
main.o: main.c
	gcc -c main.c
libuthread.so: uthread.o
	gcc -shared -o libuthread.so uthread.o
uthread.o: uthread.c
	gcc -c -fPIC uthread.c
clean:
	rm  -f  *.o  *.so main
