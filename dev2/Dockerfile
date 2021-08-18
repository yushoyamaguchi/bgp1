FROM ubuntu:focal
RUN apt -y update
RUN apt -y install iproute2 iputils-ping net-tools build-essential tcpdump

WORKDIR /work
COPY ./Makefile /work/Makefile
COPY ./main.c   /work/main.c
COPY ./clt.c    /work/clt.c
COPY ./svr.c    /work/svr.c
COPY ./bgp.h    /work/bgp.h
COPY ./bgp_peer.c    /work/bgp_peer.c
RUN make bgp_open

CMD ["/bin/bash"]
