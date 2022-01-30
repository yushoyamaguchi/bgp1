mybgp: bgp_peer.o main.o
	gcc -o mybgp bgp_peer.o main.o -ljansson
bgp_peer.o:	bgp_peer.c bgp.h
	gcc -c bgp_peer.c -ljansson
main.o:	main.c bgp.h
	gcc -c main.c


IMG := yamaguchi-bgp:test
docker-build:
	docker build -t $(IMG) .
docker-run-test: docker-build
	docker run --rm -it $(IMG) bash
