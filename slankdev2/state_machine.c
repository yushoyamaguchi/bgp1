enum state {
	ESTB,
	OPEN,
	OPENSENT,
	ACTIVE,
	..
};

struct peer {
	enum state state;
};

int main() {
	struct peer p = ...;
	char buf[4096];
	read(fd, buf, sizeof(buf));
	while (..) {
		char bgp_msg_buf[4096];
		bgp_msg_buf <- buf ..;
		bgp_process(p, bgp_msg);
	}
}

int bgp_process(p, bgp_msg) {
	switch (p->state) {
		case ESTB:
			bgp_process_estb(p, bgp_msg);
			break;
		default:
			break;
	}
}

int bgp_process_estb(p, bgp_msg) {
}
