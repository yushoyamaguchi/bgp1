tcp:	clt.o svr.o main.o
		gcc -o tcp clt.o svr.o main.o
clt.o:	clt.c bgp.h
					gcc -c clt.c
main.o:	main.c bgp.h
					gcc -c main.c
svr.o:	svr.c bgp.h
					gcc -c svr.c

