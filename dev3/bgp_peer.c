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



int exec_peer(char *ip_addr) {
    int sock;
    struct sockaddr_in clt;
    struct hostent *hp;
    struct in_addr myaddr;
    int len;
    char buf[32];
    char inp[32];
    int buf_len;
    struct bgp_open op;
    struct bgp_open_opt op_recieve;
    struct bgp_hd keep;
    struct Peer peer;
    fd_set rfds;

    struct timeval tv;



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

    /*memset(&op,0,sizeof(op));
    bgpOpenSet(&op,&myaddr);
    write(sock,&op,BGP_OPEN_LEN);*/

    printf("ctrl-C to end\n");

    do{
        FD_ZERO(&rfds);
        FD_SET(0,&rfds);
        FD_SET(sock,&rfds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        if(select(sock+1,&rfds,NULL,NULL,&tv)>0) {
            if(FD_ISSET(0,&rfds)) { /* 標準入力から入力があったなら */
                if(fgets(inp, sizeof(inp), stdin)==NULL) {
                    printf("kk\n");
                    close(sock);
                    exit(0);
                    return 0;
                }

            }
            if(FD_ISSET(sock,&rfds)) { /* ソケットから受信したなら */
                //printf("cccc\n");
                memset(&op_recieve,0,sizeof(op_recieve));
                read(sock,&op_recieve,sizeof(op_recieve));
                if(op_recieve.type==TYPE_OPEN){
                    memset(&op,0,sizeof(op));
                    bgpOpenSet(&op,&myaddr);
                    write(sock,&op,BGP_OPEN_LEN);

                    //keepalive
                    /*memset(&keep,0,sizeof(keep));
                    bgpKeepSet(&keep);
                    write(sock,&keep,BGP_HD_LEN);*/
                }
                else if(op_recieve.type==TYPE_KEEP){
                    //keepalive
                    memset(&keep,0,sizeof(keep));
                    bgpKeepSet(&keep);
                    write(sock,&keep,BGP_HD_LEN);
                }

            
            }
        }
    }while(1);

    printf("aa\n");
    close(sock);
    return 0;
}