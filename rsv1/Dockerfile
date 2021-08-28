FROM ubuntu:focal
RUN apt-get -y update
RUN apt-get -y install iproute2 iputils-ping net-tools build-essential tcpdump libjansson-dev

WORKDIR /work
COPY ./Makefile /work/Makefile
COPY ./main.c   /work/main.c
COPY ./svr.c    /work/svr.c
COPY ./bgp.h    /work/bgp.h
COPY ./bgp_peer.c    /work/bgp_peer.c
RUN make mybgp

CMD ["/bin/bash"]
