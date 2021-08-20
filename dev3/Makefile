mybgp: bgp_peer.o svr.o main.o
	gcc -o bgp_open bgp_peer.o svr.o main.o
bgp_peer.o:	bgp_peer.c bgp.h
	gcc -c bgp_peer.c
main.o:	main.c bgp.h
	gcc -c main.c
svr.o:	svr.c bgp.h
	gcc -c svr.c

IMG := yamaguchi-bgp:test
docker-build:
	docker build -t $(IMG) .
docker-run-test: docker-build
	docker run --rm -it $(IMG) bash
