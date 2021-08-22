#include<stdlib.h>
struct bgp {
	int asn;
	struct bgp_peer peers[256];
};

struct bgp_peer {
	int remote_asn;
	int timer_connect;
	enum bgp_state state;
};

enum bgp_state{
	established
};

struct config {
	//...
};
