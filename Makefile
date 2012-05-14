CC=gcc


all: a5

a5: mysched.o myqueue.o
	$(CC) mytest.c mysched.o myqueue.o mythread-new3.a -o mytest

mysched.o: mysched.c mysched.h mythread-new3.a myqueue.h futex.h futex_inline.h mycond.h mymutex.h
myqueue.o: myqueue.h myqueue.c mythread-new3.a

clean:
	rm -rf mysched.o myqueue.o mytest
