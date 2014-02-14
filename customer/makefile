all: lyepop chat
lyepop: lyepop.c cJSON.o pthreadFunc.o sockFunc.o login.o friend.o main.o
	gcc $^ -o lyepop -lpthread -lm -lnotify `pkg-config --cflags --libs gtk+-2.0`
chat: chatadd.c cJSON.o pthreadFunc.o sockFunc.o
	gcc $^ -o chat -lpthread -lm -lnotify `pkg-config --cflags --libs gtk+-2.0`
cJSON.o: cJSON.c
	gcc -c $< -lm
pthreadFunc.o: pthreadFunc.c
	gcc -c $< -lpthread
sockFunc.o: sockFunc.c
	gcc -c $<
login.o: login.c
	gcc -c $< -lm -lnotify -lpthread `pkg-config --cflags --libs gtk+-2.0`
friend.o: friend.c
	gcc -c $< -lm -lnotify -lpthread `pkg-config --cflags --libs gtk+-2.0`
main.o: main.c
	gcc -c $< -o main.o `pkg-config --cflags --libs gtk+-2.0` 
