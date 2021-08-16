#include <stdio.h>
#include <stdlib.h>

#ifndef _BGP_H_
#define _BGP_H_

#define BGP_HD_LEN 19
#define BGP_OPEN_LEN 29
#define MARKER_NUM 16

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

#endif