test: test.c mysqlFunc.o
	gcc test.c mysqlFunc.o -o test -lmysqlclient -g
mysqlFunc.o: mysqlFunc.c
	gcc -c mysqlFunc.c -lmysqlclient
