#include <stdlib.h>


#ifndef _BGP_H_
#define _BGP_H_

#define BGP_HD_LEN 19
#define BGP_OPEN_LEN 29
#define MARKER_NUM 16
#define VERSION 4
#define TYPE_OPEN 1
#define TYPE_UPDATE 2
#define TYPE_NOTIF 3
#define TYPE_KEEP 4

#define ATTR_ORIGIN 1
#define ATTR_ASPATH 2
#define ATTR_NEXTHOP 3
#define ATTR_MED 4

#define ORIGIN_IGP 0
#define ORIGIN_EGP 1
#define ORIGIN_INCOMPLETE 2

#define AS_SEQUENCE 2
#define BYTE_SIZE 8

#define ADDR_STR_LEN 16

#define flag_5 0x50
#define flag_4 0x40

#define PATH_INCOMPLETE 0xffff

struct bgp_hd
  {
    uint8_t marker[16];
    uint16_t len;
    uint8_t type;
  }__attribute__((__packed__));


struct bgp_open
  {
    uint8_t marker[16];
    uint16_t len;
    uint8_t type;
    uint8_t version;
    uint16_t myas;
    uint16_t holdtime;
    uint32_t id;
    uint8_t opt_len;
  }__attribute__((__packed__));


struct bgp_open_opt
  {
    uint8_t marker[16];
    uint16_t len;
    uint8_t type;
    uint8_t version;
    uint16_t myas;
    uint16_t holdtime;
    uint32_t id;
    uint8_t opt_len;
    uint8_t opt[46];
  }__attribute__((__packed__));  

  
struct bgp_update
  {
    uint8_t marker[16];
    uint16_t len;
    uint8_t type;
    uint16_t withdrawn_len;
    uint8_t contents[64];
  }__attribute__((__packed__));  

struct path_attr_origin{
  uint8_t flags;
  uint8_t type_code;
  uint8_t length;
  uint8_t origin;
}__attribute__((__packed__));

struct aspath_segment{
  uint8_t segment_type;
  uint8_t number_of_as;
  uint16_t as2[1];  //可変長
}__attribute__((__packed__));


struct path_attr_aspath{
  uint8_t flags;
  uint8_t type_code;
  uint16_t length;
  struct aspath_segment seg[1]; //複数の場合もある

}__attribute__((__packed__));

struct path_attr_aspath_short{
  uint8_t flags;
  uint8_t type_code;
  uint8_t length;
  struct aspath_segment seg[1]; //複数の場合もある

}__attribute__((__packed__));

struct path_attr_nexthop{
  uint8_t flags;
  uint8_t type_code;
  uint8_t length;
  uint32_t nexthop;
}__attribute__((__packed__));

struct path_attr_med{
  uint8_t flags;
  uint8_t type_code;
  uint8_t length;
  uint32_t med;
}__attribute__((__packed__));

struct bgp_nlri{
  uint8_t subnet_mask;
  uint8_t ip_addr[4];
}__attribute__((__packed__));

struct route_state{
  _Bool isValid;
  _Bool isDampening;
  _Bool isRoutingTable;
  _Bool isSummaryOnly;
  _Bool isBest;
  _Bool isIBGP;
}__attribute__((__packed__));

struct bgp_table_entry{
  struct route_state state;
  uint32_t addr;
  uint8_t subnet_mask;
  uint32_t nexthop;
  uint16_t path[256];
}__attribute__((__packed__));


enum PeerState {
  Idle,
  Connect,
  Active,
  OpenSent,
  OpenConfirm,
  Established
} ;


struct Peer
{
  int remote_asn;
	int timer_connect;
  enum PeerState state;
  
};

struct BGP {
	int asn;
  uint32_t bgp_id;
  uint32_t ip_addr;
	struct Peer peers[256];
  struct bgp_table_entry table_entry[256];
  int num_of_table;
};






#endif