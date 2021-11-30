#ifndef __NFT_H__
#define __NFT_H__

#include <stdint.h>
#include <stdbool.h>

struct nft_meta {
	/* used in nft */
	unsigned family;
	char *table_name;
	char *chain_name;
	char *chain_type;
	unsigned chain_hooknum;
	unsigned chain_prio;
	char *set_name;
	uint8_t *set_elems;
	int set_elem_len;
	int set_id;
	unsigned mark;
	unsigned short tproxy_port;

	/* used in netlink message */
	unsigned *seq;

	/* for debug */
	bool verbose;
};

union nft_expr_arg {
	struct {
		unsigned meta_key;
		unsigned meta_dreg;
	};
	struct {
		char *lookup_set_name;
		int lookup_set_id;
		unsigned lookup_sreg;
	};
	struct {
		unsigned cmp_sreg;
		unsigned cmp_op;
		unsigned cmp_data;
	};
	struct {
		unsigned imm_dreg;
		unsigned short imm_data;
	};
	struct {
		unsigned tproxy_family;
		unsigned tproxy_reg_port;
	};
};

typedef struct nftnl_expr *(*nft_expr_fn_t)(union nft_expr_arg *);

struct nft_expr_fn {
	nft_expr_fn_t fn;
	union nft_expr_arg arg;
};

struct mnl_nlmsg_batch;
typedef int (*nft_msg_cb_t)(struct nft_meta *, struct mnl_nlmsg_batch *);

struct nft {
	unsigned family;
	char *table_name;
	char *set_name;
	unsigned mark;
	unsigned short tproxy_port;
	bool verbose;
};

int nft_setup(struct nft *);
int nft_destroy(struct nft *);

#endif /* __NFT_H__ */
