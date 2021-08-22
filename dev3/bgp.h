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

struct bgp_hd
  {
    uint8_t marker[16];
    uint16_t len;
    uint8_t type;
  };


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
  };


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
  };  

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
	struct Peer peers[256];
};






#endif