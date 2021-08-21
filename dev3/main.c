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

int exec_peer(char *addr);


int main(int argc,char **argv)
{
  char addr[127];

  printf("input ip address");
  scanf("%s", addr);
  exec_peer(addr);
  printf("end\n");


  return 0;
}