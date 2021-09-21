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


int flag_first0_else1=0;

char subnet_mask_8byte[5][128]={"0.0.0.0","255.0.0.0","255.255.0.0","255.255.255.0","255.255.255.255"};



void bgp_open_set(struct bgp_open *op,struct in_addr *myaddr){
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

void bgp_keep_set(struct bgp_hd *keep){
	int i;
	for(i=0;i<MARKER_NUM;i++){
		keep->marker[i]=0xff;
	}
	keep->len=htons(BGP_HD_LEN);
	keep->type=TYPE_KEEP;
}


struct path_attr_aspath *as_path_set(struct BGP *bgp,int table_read){
    int num_of_path=0;
    while(bgp->table_entry[table_read].path[num_of_path]!=htons(PATH_INCOMPLETE)){
        num_of_path++;
    }
    struct path_attr_aspath *aspath;

    struct aspath_segment *aspath_seg;
    aspath_seg=malloc(sizeof(struct aspath_segment)+num_of_path*sizeof(uint16_t));
    aspath=malloc(4+sizeof(aspath_seg));

    aspath->seg[0].as2[1]=htons(1);
    aspath->flags=0x50; //属性長2オクテットで固定
    aspath->type_code=ATTR_ASPATH;
    int i=0;
    aspath->seg[0].segment_type=AS_SEQUENCE;
    aspath->seg[0].number_of_as=num_of_path+1;
    aspath->seg[0].as2[0]=htons((uint16_t)bgp->asn);
    for(i=1;i<=num_of_path;i++){
        aspath->seg[0].as2[i]=bgp->table_entry[table_read].path[i-1];
    }
    aspath->length=htons(2+sizeof(uint16_t)*(num_of_path+1));   //2はseg type,seg length

    return aspath;
}


void origin_set(struct path_attr_origin *origin){
    origin->flags=0x40;
    origin->type_code=ATTR_ORIGIN;
    origin->length=1;
    origin->origin=ORIGIN_INCOMPLETE;
}


void nexthop_set(struct BGP *bgp,int table_read,struct path_attr_nexthop *nh){
    nh->flags=0x40;
    nh->type_code=ATTR_NEXTHOP;
    nh->length=4;
    nh->nexthop=bgp->ip_addr;   
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




int bgp_update_set(struct BGP *bgp,struct bgp_update *update){
    int reading_table=bgp->num_of_table-1;
    int i;
    for(i=0;i<MARKER_NUM;i++){
		update->marker[i]=0xff;
	}
    uint8_t *look_place;
    look_place=update->contents;
    update->type=TYPE_UPDATE;
    update->withdrawn_len=htons(0);
    look_place+=2;  //total path attr lengthの分
    struct path_attr_origin origin;
    struct path_attr_aspath *aspath;
    struct path_attr_nexthop nexthop;
    struct path_attr_med med;
    struct bgp_nlri nlri;
    origin_set(&origin);
    memcpy(look_place,&origin,sizeof(origin));
    look_place=look_place+sizeof(origin);

    aspath=as_path_set(bgp,reading_table);
    memcpy(look_place,aspath,htons(aspath->length)+4);
    look_place=look_place+htons(aspath->length)+4;
    nexthop_set(bgp,reading_table,&nexthop);
    memcpy(look_place,&nexthop,sizeof(nexthop));
    look_place=look_place+sizeof(nexthop);

    uint16_t *path_attr_len=update->contents;
    *path_attr_len=htons((look_place-update->contents)-2);  //エラー起きたらここチェック
    //update->contents[0]=0;
    //update->contents[1]=(look_place-update->contents)-2;  //255以上の長さに対応できるように改良する
    //最後の2はtotal path attr lengthのバイト数

    nlriSet(&nlri,(int)(bgp->table_entry[reading_table].subnet_mask),bgp->table_entry[reading_table].addr);
    //nlriのaddr部分のサイズ計算
    int nlri_addr_len=(int)(bgp->table_entry[reading_table].subnet_mask)/BYTE_SIZE;
    memcpy(look_place,&nlri,nlri_addr_len+1);
    look_place=look_place+nlri_addr_len+1;
    update->len=htons(21+(look_place-update->contents));

    return 21+(look_place-(int)(update->contents));

}


int bgp_update_set_first(struct BGP *bgp,struct bgp_update *update,int reading_table){
    int i;
    for(i=0;i<MARKER_NUM;i++){
		update->marker[i]=0xff;
	}
    uint8_t *look_place=update->contents;
    update->type=TYPE_UPDATE;
    update->withdrawn_len=htons(0);
    look_place+=2;   //total path attr lengthの分
    struct path_attr_origin origin;
    struct path_attr_aspath *aspath;
    struct path_attr_nexthop nexthop;
    struct path_attr_med med;
    struct bgp_nlri nlri;
    origin_set(&origin);
    memcpy(look_place,&origin,sizeof(origin));
    look_place=look_place+sizeof(origin);
    aspath=as_path_set(bgp,reading_table);
    memcpy(look_place,aspath,htons(aspath->length)+4);
    look_place=look_place+htons(aspath->length)+4;
    nexthop_set(bgp,reading_table,&nexthop);
    memcpy(look_place,&nexthop,sizeof(nexthop));
    look_place=look_place+sizeof(nexthop);
    uint16_t *path_attr_len=update->contents;
    *path_attr_len=htons((look_place-update->contents)-2);  //エラー起きたらここチェック
    //update->contents[0]=0;
    //update->contents[1]=(look_place-update->contents)-2;  //255以上の長さに対応できるように改良する
    //最後の2はtotal path attr lengthのバイト数

    nlriSet(&nlri,(int)(bgp->table_entry[reading_table].subnet_mask),bgp->table_entry[reading_table].addr);
    //nlriのaddr部分のサイズ計算
    int nlri_addr_len=(int)(bgp->table_entry[reading_table].subnet_mask)/BYTE_SIZE;
    memcpy(look_place,&nlri,nlri_addr_len+1);
    look_place=look_place+nlri_addr_len+1;
    update->len=htons(21+(look_place-update->contents));


    return 21+(int)(look_place-update->contents);
}

void bgp_process_open_sent(struct Peer *p,char *bgp_msg,int sock){
    struct bgp_open_opt open;
    struct bgp_hd keep;
    memcpy(&open,bgp_msg,sizeof(open));
    if(open.type==TYPE_OPEN){
        p->state=OpenConfirm;
        memset(&keep,0,sizeof(keep));
        bgp_keep_set(&keep);
        write(sock,&keep,BGP_HD_LEN);
    }
}

void bgp_process_open_confirm(struct BGP *bgp,struct Peer *p,char *bgp_msg,int sock){
    struct bgp_hd keep;
    memcpy(&keep,bgp_msg,sizeof(keep));
    if(keep.type==TYPE_KEEP){
        p->state=Established;
        printf("established\n");
        memset(&keep,0,sizeof(keep));
        bgp_keep_set(&keep);
        write(sock,&keep,BGP_HD_LEN);
        struct bgp_update update;
        int i,update_size;
        for(i=0;i<bgp->num_of_table;i++){
            update_size=bgp_update_set_first(bgp,&update,i);
            write(sock,&update,update_size);
            memset(&update,0,update_size);
        }
    }
}


void as_path_table_write(struct BGP *bgp,struct bgp_update *update,uint8_t *read_packet){
    int i=0;
    uint8_t *reading;
    struct path_attr_aspath *aspath5;
    struct path_attr_aspath_short *aspath4;
    int k,path_serial_num=0;
    if(*read_packet==flag_5){
        aspath5=read_packet;   //AS2個目以降はみ出てる
        reading=aspath5->seg;
        while((uint8_t *)aspath5->seg + htons(aspath5->length)>reading){
            for(k=0;k<(int)aspath5->seg[i].number_of_as;k++){
                bgp->table_entry[bgp->num_of_table].path[path_serial_num]=aspath5->seg[i].as2[k];
                path_serial_num++;
            }
            i++;
            reading=reading+sizeof(struct aspath_segment);
        }
    }
    else if(*read_packet==flag_4){
        reading=aspath4->seg;
        aspath4=read_packet;
        while((uint8_t *)aspath4->seg + (aspath4->length)>reading){
            for(k=0;k<(int)aspath4->seg[i].number_of_as;k++){
                bgp->table_entry[bgp->num_of_table].path[path_serial_num]=aspath4->seg[i].as2[k];
                path_serial_num++;
            }
            i++;
            reading=reading+sizeof(struct aspath_segment);
        }
    }

    bgp->table_entry[bgp->num_of_table].path[i]=htons(PATH_INCOMPLETE);
}

void nexthop_table_write(struct BGP *bgp,struct bgp_update *update,uint8_t *read_packet){
    struct path_attr_nexthop *nexthop;
    nexthop=read_packet;
    bgp->table_entry[bgp->num_of_table].nexthop=nexthop->nexthop;
}

uint8_t* read_path_attr(struct BGP *bgp,struct bgp_update *update,uint8_t *read_packet){
    switch (*(read_packet+1)) {
		case ATTR_ORIGIN:
			return read_packet+sizeof(struct path_attr_origin);
        case ATTR_ASPATH:
			as_path_table_write(bgp,update,read_packet);
			if(*read_packet==0x50) return read_packet+4+(*(read_packet+3));
            else if(*read_packet==0x40) return read_packet+3+(*(read_packet+2));
        case ATTR_NEXTHOP:
			nexthop_table_write(bgp,update,read_packet);
			return read_packet+sizeof(struct path_attr_nexthop);
        case ATTR_MED:
			return read_packet+sizeof(struct path_attr_med);
		default:
			break;
	}
}

void nlri_table_write(struct BGP *bgp,struct bgp_update *update,uint8_t *read_packet){
    bgp->table_entry[bgp->num_of_table].subnet_mask=*read_packet;
    uint8_t sub_mask=*read_packet;
    read_packet++;
    uint8_t sub_calc=sub_mask;
    uint8_t *table_addr=&(bgp->table_entry[bgp->num_of_table].addr);
    while(sub_calc>=BYTE_SIZE){
        memcpy(table_addr,read_packet,sizeof(uint8_t));
        table_addr+=sizeof(uint8_t);
        read_packet+=sizeof(uint8_t);
        sub_calc-=BYTE_SIZE;
    }
    uint8_t subnet_rest=32-sub_mask;
    while(subnet_rest>0){
        memset(table_addr,0,sizeof(uint8_t));
        table_addr+=sizeof(uint8_t);
        subnet_rest-=BYTE_SIZE;
    }
    bgp->num_of_table++;
}

void nlri_table_write2(struct BGP *bgp,struct bgp_update *update,uint8_t *read_packet){
    bgp->table_entry[bgp->num_of_table].subnet_mask=*read_packet;
    uint8_t sub_mask=*read_packet;
    read_packet++;
    uint8_t sub_calc=sub_mask;
    uint8_t *table_addr=&(bgp->table_entry[bgp->num_of_table].addr);
    while(sub_calc>=BYTE_SIZE){
        memcpy(table_addr,read_packet,sizeof(uint8_t));
        table_addr+=sizeof(uint8_t);
        read_packet+=sizeof(uint8_t);
        sub_calc-=BYTE_SIZE;
    }
    uint8_t subnet_rest=32-sub_mask;
    while(subnet_rest>0){
        memset(table_addr,0,sizeof(uint8_t));
        table_addr+=sizeof(uint8_t);
        subnet_rest-=BYTE_SIZE;
    }
    bgp->num_of_table++;
}

void show_table(struct BGP *bgp){
    int i,k=0;
    printf("------------------------------------------");
    printf("\n");
    printf("best:addr    :mask   :nexthop    :aspath\n");
    printf("------------------------------------------");
    printf("\n");
    char addr[32];
    char *addr_buf;
    char nexthop[32];
    char *nexthop_buf;
    struct in_addr ip_addr;
    for(i=0;i<(bgp->num_of_table);i++){
        ip_addr.s_addr=bgp->table_entry[i].addr;
        addr_buf=inet_ntoa(ip_addr);
        memcpy(&addr,addr_buf,ADDR_STR_LEN);//エラー起こったらここチェック
        ip_addr.s_addr=bgp->table_entry[i].nexthop;
        nexthop_buf=inet_ntoa(ip_addr);
        memcpy(&nexthop,nexthop_buf,ADDR_STR_LEN);//エラー起こったらここチェック*/
        printf("%d  :%s  :%u   :%s :",bgp->table_entry[i].state.isBest,addr,bgp->table_entry[i].subnet_mask,nexthop);
        for(k=0;k<3;k++){
            printf(",%u",htons(bgp->table_entry[i].path[k]));
        }

        printf("\n");
        printf("------------------------------------------");
        printf("\n");

    }
    printf("\n");
}

int num_of_as(struct BGP *bgp,int reading){
    int i=0;
    while(bgp->table_entry[reading].path[i]!=PATH_INCOMPLETE){
        i++;
    }
    return i;
}

void best_path_selection(struct BGP *bgp,struct bgp_update *update,int table_tail){
    int i=1;
    int shortest_aspath_length=4096;
    int preliminary_shortest_num=0;
    int as_num=0;
    uint32_t dest_addr=bgp->table_entry[table_tail].addr;
    for(i=0;i<=table_tail;i++){
        if(bgp->table_entry[i].addr==dest_addr){
            as_num=num_of_as(bgp,i);
            if(as_num<shortest_aspath_length){
                preliminary_shortest_num=i;
                shortest_aspath_length=as_num;
            }
            bgp->table_entry[i].state.isBest=0;
        }
    }
    bgp->table_entry[preliminary_shortest_num].state.isBest=1;
}


void table_write(struct BGP *bgp,struct bgp_update *update,int length){
    uint8_t *read_packet;
    int path_attr_len=(int)(update->contents[1]);//2バイトの場合に非対応ver
    read_packet=(update->contents)+2;
    while(update->contents+2+path_attr_len>read_packet){
        read_packet=read_path_attr(bgp,update,read_packet);
    }
    printf("%p\n",read_packet);
    nlri_table_write(bgp,update,read_packet);
    printf("cuttent:%p,end:%p\n",read_packet,update->marker+length);
    /*struct in_addr addr;
    addr.s_addr=bgp->table_entry[bgp->num_of_table-1].addr;
    printf("%s\n",inet_ntoa(addr));*/
    best_path_selection(bgp,update,bgp->num_of_table-1);
    show_table(bgp);
}

void add_routing_table(struct BGP *bgp,struct bgp_table_entry *entry,int num){
    char command[128]="route add ";
    char *str_ptr;
    char buf[32];
    struct in_addr ip_addr;
    ip_addr.s_addr=entry[num].addr;
    str_ptr=inet_ntoa(ip_addr);
    memcpy(buf,str_ptr,ADDR_STR_LEN);//エラー起こったらここチェック
    strcat(command,"-net ");
    strcat(command,buf);
    memset(buf,0,ADDR_STR_LEN);
    strcat(command," netmask ");
    strcat(command,subnet_mask_8byte[entry[num].subnet_mask/8]);
    ip_addr.s_addr=entry[num].nexthop;
    str_ptr=inet_ntoa(ip_addr);
    memcpy(buf,str_ptr,ADDR_STR_LEN);//エラー起こったらここチェック
    strcat(command," gw ");
    strcat(command,buf);
    //printf("%s\n",command);
    if(system(command)==-1){
        printf("command error\n");
    }
}

void bgp_process_established(struct BGP *bgp, struct Peer *p,char *bgp_msg,int sock){
    struct bgp_hd keep;
    int len;
    memcpy(&keep,bgp_msg,sizeof(struct bgp_hd));
    len=htons(keep.len);
    if(keep.type==TYPE_KEEP){
        memset(&keep,0,sizeof(keep));
        bgp_keep_set(&keep);
        write(sock,&keep,BGP_HD_LEN);
        //show_table(bgp);
    }
    else if(keep.type==TYPE_UPDATE){
        struct bgp_update up_read;
        int update_size=0;
        printf("update recieved\n");
        int bgp_length=htons(keep.len);
        memcpy(&up_read,bgp_msg,bgp_length);
        if(up_read.withdrawn_len==0){
            table_write(bgp,&up_read,len);
            if(bgp->table_entry[bgp->num_of_table-1].state.isBest==1){
                if(bgp->table_entry[bgp->num_of_table-1].addr!=bgp->ip_addr){
                    add_routing_table(bgp,bgp->table_entry,bgp->num_of_table-1);
                }
                struct bgp_update update;
                update_size =bgp_update_set(bgp,&update);
                write(sock,&update,update_size);  
            }
        }
        else{
            //withdrawn
        }
    }
}

void bgp_process(struct BGP *bgp,struct Peer *p,char *bgp_msg,int sock) {
	switch (p->state) {
		case OpenSent:
			bgp_process_open_sent(p, bgp_msg,sock);
			break;
        case OpenConfirm:
			bgp_process_open_confirm(bgp,p, bgp_msg,sock);
			break;
        case Established:
			bgp_process_established(bgp,p, bgp_msg,sock);
			break;
		default:
			break;
	}
}

void json_config(struct BGP *bgp,json_t *json_object,json_error_t *jerror){
    json_object=json_load_file("config.json",0,jerror);
    if(json_object==NULL){
        printf("cannot read config json\n");
        exit(1);
    }
    char buf[128];
    bgp->asn=json_integer_value(json_object_get(json_object,"router bgp"));
    strcpy(buf,json_string_value(json_object_get(json_object,"router-id")));
    bgp->bgp_id=inet_addr(buf);
    strcpy(buf,json_string_value(json_object_get(json_object,"ip-address")));
    bgp->ip_addr=inet_addr(buf);

    json_t *neighbor_array;
    json_t *neighbor_object;
    int i=0;
    neighbor_object=malloc(sizeof(json_t));
    neighbor_array=json_object_get(json_object,"networks");
    json_array_foreach(neighbor_array,i,neighbor_object){
        strcpy(buf,json_string_value(json_object_get(neighbor_object,"address")));
        bgp->table_entry[bgp->num_of_table].addr=inet_addr(buf);
        bgp->table_entry[bgp->num_of_table].nexthop=inet_addr(buf);
        strcpy(buf,json_string_value(json_object_get(neighbor_object,"subnet_mask")));
        bgp->table_entry[bgp->num_of_table].subnet_mask=atoi(buf);
        bgp->table_entry[bgp->num_of_table].path[0]=htons(PATH_INCOMPLETE);
        bgp->table_entry[bgp->num_of_table].state.isBest=1;
        bgp->num_of_table++;
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
    struct BGP bgp;
    bgp.num_of_table=0;
    struct Peer peer;
    bgp.peers[0]=peer;
    fd_set rfds;


    json_t *json_object;
    json_error_t jerror;

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
    bgp_open_set(&op,&myaddr);
    write(sock,&op,BGP_OPEN_LEN);
    peer.state=OpenSent;
    json_config(&bgp,json_object,&jerror);

    while(1){
        memset(buf,0,sizeof(buf));
        read(sock,buf,sizeof(buf));
        //複数メッセージが入ってるパターンに対応させる
        memcpy(bgpmsg_buf,buf,sizeof(buf));
        bgp_process(&bgp,&peer,bgpmsg_buf,sock);

    }



    close(sock);
    return 0;
}
