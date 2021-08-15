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


while (fgets(inp, sizeof(inp), stdin)!= NULL){
    len=strlen(inp)+2;
    write(sock,inp,len);
    memset(buf, 0, sizeof(buf));
    buf_len = read(sock, buf, sizeof(buf));
    printf("%s",  buf);
    memset(inp, 0, sizeof(inp));

}
 close(sock);
 return 0;
}
