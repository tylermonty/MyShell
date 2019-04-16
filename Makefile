# choose your compiler
CC=gcc
#CC=gcc -Wall

mysh: sh.o get_path.o which.o where.o cd.o pwd.o list.o pid.o kill.o printenv.o watchuser.o main.c
	$(CC) -pthread -g main.c sh.o get_path.o which.o where.o cd.o pwd.o list.o pid.o kill.o printenv.o watchuser.o -o mysh
#	$(CC) -g main.c sh.o get_path.o bash_getcwd.o -o mysh

sh.o: sh.c sh.h
	$(CC) -g -c sh.c

which.o: which.c
	$(CC) -g -c which.c

where.o: where.c
	$(CC) -g -c where.c

cd.o: cd.c
	$(CC) -g -c cd.c

pwd.o: pwd.c
	$(CC) -g -c pwd.c

list.o: list.c
	$(CC) -g -c list.c

pid.o: pid.c
	$(CC) -g -c pid.c

kill.o: kill.c
	$(CC) -g -c kill.c

printenv.o: printenv.c
	$(CC) -g -c printenv.c

watchuser.o: watchuser.c
	$(CC) -g -c watchuser.c

get_path.o: get_path.c get_path.h
	$(CC) -g -c get_path.c

clean:
	rm -rf sh.o get_path.o which.o where.o cd.o pwd.o list.o pid.o kill.o printenv.o  watchuser.o mysh
