server: server.c sockFunc.o pthreadFunc.o mysqlFunc.o cJSON.o backStageFunc.o
	gcc cJSON.o sockFunc.o pthreadFunc.o mysqlFunc.o backStageFunc.o server.c -o server -lpthread -lm -lmysqlclient -g
backStageFunc.o: backStageFunc.c
	gcc -c backStageFunc.c -lmysqlclient
sockFunc.o: sockFunc.c
	gcc -c sockFunc.c
pthreadFunc.o: pthreadFunc.c
	gcc -c pthreadFunc.c -l pthread
mysqlFunc.o: mysqlFunc.c
	gcc -c mysqlFunc.c -lmysqlclient
cJSON.o: cJSON.c
	gcc -c cJSON.c -lm
