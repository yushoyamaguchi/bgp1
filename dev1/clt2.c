#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "bgp.h"

void bgpOpenSet(struct bgp_open *op,struct in_addr *addr){
	int i;
	for(i=0;i<MARKER_NUM;i++){
		op->marker[i]=0xff;
	}
	op->len=29;
	op->type=1;
	op->version=4;
	op->myas=1;
	op->holdtime=180;
	op->id=addr->s_addr;
	op->opt_len=0;
}

int exec_client(char *ip_addr) {
int sock;
struct sockaddr_in clt;
struct hostent *hp;
struct in_addr addr;
int len;
char buf[32];
char inp[32];
int buf_len;



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

printf("Input something    ctrl-C to end\n");
while (fgets(inp, sizeof(inp), stdin)!= NULL){
  	len=BGP_OPEN_LEN;
	  bgpOpenSet(&op,&addr);
	  write(sock,&op,len);
    memset(inp, 0, sizeof(inp));
    printf("Input something    ctrl-C to end\n");

}
 close(sock);
 return 0;
}