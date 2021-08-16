#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "bgp.h"

#include<stdio.h>
#include<stdlib.h>

int exec_server();
int exec_client(char *addr);


int main(int argc,char **argv)
{
  char addr[127];
  int mode=-1;
  printf("Server : input 0\n");
  printf("Client : input 1\n");
  scanf("%d",&mode);
  if(mode==0){
    exec_server();
  }
  else if(mode==1){
    printf("input ip address");
    scanf("%s", addr);
    exec_client(addr);
  }

  return 0;
}