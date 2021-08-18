bgp_open: clt.o svr.o main.o
	gcc -o bgp_open clt.o svr.o main.o
clt.o:	clt.c bgp.h
	gcc -c clt.c
main.o:	main.c bgp.h
	gcc -c main.c
svr.o:	svr.c bgp.h
	gcc -c svr.c

IMG := yamaguchi-bgp:test
docker-build:
	docker build -t $(IMG) .
docker-run-test: docker-build
	docker run --rm -it $(IMG) bash
