all: a4Server1 a4Server2 a4Server3 a4Client1 a4Client2 a4Client3

a4Server1: a4Server1.c
	gcc -o a4Server1 a4Server1.c -lpthread -l sqlite3

a4Client1: a4Client1.c
	gcc -o a4Client1 a4Client1.c -lpthread -l sqlite3
 
a4Server2: a4Server2.c
	gcc -o a4Server2 a4Server2.c -lpthread -l sqlite3

a4Client2: a4Client2.c
	gcc -o a4Client2 a4Client2.c -lpthread -l sqlite3

a4Server3: a4Server3.c
	gcc -o a4Server3 a4Server3.c -lpthread -l sqlite3

a4Client3: a4Client3.c
	gcc -o a4Client3 a4Client3.c -lpthread -l sqlite3