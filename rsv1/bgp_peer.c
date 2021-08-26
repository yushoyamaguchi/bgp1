#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <jansson.h>


#include "bgp.h"



void bgpOpenSet(struct bgp_open *op,struct in_addr *myaddr){
	int i;
	for(i=0;i<MARKER_NUM;i++){
		op->marker[i]=0xff;
	}
	op->len=htons(29);
	op->type=TYPE_OPEN;
	op->version=VERSION;
	op->myas=htons(1);
	op->holdtime=htons(180);
	op->id=inet_addr("10.255.1.1");
	op->opt_len=0;
}

void bgpKeepSet(struct bgp_hd *keep){
	int i;
	for(i=0;i<MARKER_NUM;i++){
		keep->marker[i]=0xff;
	}
	keep->len=htons(BGP_HD_LEN);
	keep->type=TYPE_KEEP;
}

void bgp_process_open_sent(struct Peer *p,char *bgp_msg,int sock){
    struct bgp_open_opt open;
    struct bgp_hd keep;
    memcpy(&open,bgp_msg,sizeof(open));
    if(open.type==TYPE_OPEN){
        p->state=OpenConfirm;
        memset(&keep,0,sizeof(keep));
        bgpKeepSet(&keep);
        write(sock,&keep,BGP_HD_LEN);
    }
}  

void bgp_process_open_confirm(struct Peer *p,char *bgp_msg,int sock){
    struct bgp_hd keep;
    memcpy(&keep,bgp_msg,sizeof(keep));
    if(keep.type==TYPE_KEEP){
        p->state=Established;
        memset(&keep,0,sizeof(keep));
        bgpKeepSet(&keep);
        write(sock,&keep,BGP_HD_LEN);
    }    
}  

void bgp_process_established(struct Peer *p,char *bgp_msg){
    
}  

void bgp_process(struct Peer *p,char *bgp_msg,int sock) {
	switch (p->state) {
		case OpenSent:
			bgp_process_open_sent(p, bgp_msg,sock);
			break;
        case OpenConfirm:
			bgp_process_open_confirm(p, bgp_msg,sock);
			break;
        case Established:
			bgp_process_established(p, bgp_msg);
			break;        
		default:
			break;
	}
}



int exec_peer(char *ip_addr) {
    int sock;
    struct sockaddr_in clt;
    struct hostent *hp;
    struct in_addr myaddr;
    int len;

    int buf_len;
    struct bgp_open op;
    struct bgp_open_opt op_recieve;
    struct bgp_hd keep;
    struct Peer peer;
    fd_set rfds;

    char buf[4096];
    char bgpmsg_buf[4096];

        if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("client : socket");
        exit(1);
    }

    bzero(&clt,sizeof(clt));
    clt.sin_family=AF_INET;
    clt.sin_addr.s_addr=inet_addr(ip_addr);
    clt.sin_port=htons(179);


    if (connect(sock, (struct sockaddr *)&clt,sizeof(clt))  ==-1) {
        perror("client : connect");
        close(sock);
        exit(1);
    }
    
    printf("connect\n");
    memset(&op,0,sizeof(op));
    bgpOpenSet(&op,&myaddr);
    write(sock,&op,BGP_OPEN_LEN);
    peer.state=OpenSent;

    while(1){
        memset(buf,0,sizeof(buf));
        read(sock,buf,sizeof(buf));
        //複数メッセージが入ってるパターンに対応させる
        memcpy(bgpmsg_buf,buf,sizeof(buf));
        bgp_process(&peer,bgpmsg_buf,sock);

    }



    close(sock);
    return 0;
}