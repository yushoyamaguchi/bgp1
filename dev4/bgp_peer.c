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
	op->id=inet_addr("10.255.1.1"); //myaddr使った形に変える
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

void asPathSet(struct path_attr_aspath *aspath){
    aspath->flags=0x50; //属性長2オクテットで固定
    aspath->type_code=ATTR_ASPATH;
    aspath->length=htons(6);
    aspath->seg.segment_type=AS_SEQUENCE;
    aspath->seg.number_of_as=1;
    aspath->seg.as2[0]=htons(1);    //as番号は1
}

void originSet(struct path_attr_origin *origin){
    origin->flags=0x40;
    origin->type_code=ATTR_ORIGIN;
    origin->length=1;
    origin->origin=ORIGIN_INCOMPLETE;
}

void nextHopSet(struct path_attr_nexthop *nh){
    nh->flags=0x40;
    nh->type_code=ATTR_NEXTHOP;
    nh->length=4;
    nh->nexthop=inet_addr("10.255.1.1");    //nexthopは自分自身のアドレス10.255.1.1
}

void medSet(struct path_attr_med *med){
    med->flags=0x80;    //optional
    med->type_code=ATTR_MED;
    med->length=4;
    med->med=0;
}

void nlriSet(struct bgp_nlri *nlri,int subnet_mask,u_int32_t addr){
    nlri->subnet_mask=subnet_mask;
    //サブネットマスクに合わせて必要な情報をip_addrに入れる
    //addrにはtcp/ipの仕様(ビッグエンディアン)に並べ替え済みの値が格納されている。
    uint8_t *read_addr;
    read_addr=&addr;
    int mask_calc=subnet_mask;
    int i=0;
    while(mask_calc>BYTE_SIZE){
        nlri->ip_addr[i]=*read_addr;
        read_addr=read_addr+sizeof(uint8_t);
        i++;
        mask_calc-=BYTE_SIZE;
    }
}


void bgpUpdateSet(struct bgp_update *update){
    int i;
    uint8_t *look_place=update->contents;
	for(i=0;i<MARKER_NUM;i++){
		update->marker[i]=0xff;
	}
    update->type=TYPE_UPDATE;
    update->withdrawn_len=0;
    look_place++;   //total path attr lengthの分
    struct path_attr_origin origin;
    struct path_attr_aspath aspath;
    struct path_attr_nexthop nexthop;
    struct path_attr_med med;
    originSet(&origin);
    memcpy(look_place,&origin,sizeof(origin));
    look_place=look_place+sizeof(origin);
    asPathSet(&aspath);
    memcpy(look_place,&aspath,sizeof(aspath));
    look_place=look_place+sizeof(aspath);
    nextHopSet(&nexthop);
    memcpy(look_place,&nexthop,sizeof(nexthop));
    look_place=look_place+sizeof(nexthop);
    medSet(&med);
    memcpy(look_place,&med,sizeof(med));
    look_place=look_place+sizeof(med);
    update->contents[0]=((look_place-update->contents)/sizeof(uint8_t))-1;





    //nlriやる    


    
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

void bgp_process_established(struct Peer *p,char *bgp_msg,int sock){
    struct bgp_hd keep;
    memcpy(&keep,bgp_msg,sizeof(keep));
    if(keep.type==TYPE_KEEP){
        p->state=Established;
        memset(&keep,0,sizeof(keep));
        bgpKeepSet(&keep);
        write(sock,&keep,BGP_HD_LEN);
    }
    else if(keep.type==TYPE_UPDATE){
        struct bgp_update update;
        memcpy(&update,bgp_msg,sizeof(update));
    }  
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
			bgp_process_established(p, bgp_msg,sock);
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
