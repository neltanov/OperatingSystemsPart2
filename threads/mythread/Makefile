main: main.o libmythread.so
	gcc -o main main.o -L. -lmythread -Wl,-rpath,.
main.o: main.c
	gcc -c main.c
libmythread.so: mythread.o
	gcc -shared -o libmythread.so mythread.o
mythread.o: mythread.c
	gcc -c -fPIC mythread.c
clean:
	rm  -f  *.o  *.so main
