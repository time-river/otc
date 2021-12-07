/*
 * (C) 2021 by river <river@vvl.me>
 *
 * Create netfilter rules to redirect IP messages.
 *
 * The followings code is equal to:
 *
 *   nft 'create table inet <name>'
 *   nft 'add chain inet <name> input { type nat hook prerouting priority dstnat ; }'
 *   nft 'add rule inet <name> input meta l4proto { tcp, udp } meta mark == <mark> tproxy to :<port> counter'
 *
 * Note:
 *   use `nft --debug all` option to show all nft steps.
 *
 * TODO:
 * - check netlink recv msg
 * - use anonymous set
 * - better log
 */

#include <time.h>
#include <netinet/in.h>

#include <libmnl/libmnl.h>
#include <libnftnl/table.h>
#include <libnftnl/chain.h>
#include <libnftnl/rule.h>
#include <libnftnl/set.h>
#include <libnftnl/expr.h>

#include <linux/netfilter.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter_ipv4.h>

#include "common.h"
#include "nft.h"
#include "log.h"

static struct nftnl_expr *alloc_counter(union nft_expr_arg *_) {
	return nftnl_expr_alloc("counter");
}

static struct nftnl_expr *alloc_tproxy(union nft_expr_arg *arg) {
	struct nftnl_expr *e;

	e = nftnl_expr_alloc("tproxy");
	if (e == NULL)
		return NULL;

	nftnl_expr_set_u32(e, NFTNL_EXPR_TPROXY_FAMILY, arg->tproxy_family);
	nftnl_expr_set_u32(e, NFTNL_EXPR_TPROXY_REG_PORT, arg->tproxy_reg_port);
	return e;
}

static struct nftnl_expr *alloc_immediate(union nft_expr_arg *arg) {
	struct nftnl_expr *e;

	e = nftnl_expr_alloc("immediate");
	if (e == NULL)
		return NULL;

	nftnl_expr_set_u32(e, NFTNL_EXPR_IMM_DREG, arg->imm_dreg);
	nftnl_expr_set(e, NFTNL_EXPR_IMM_DATA, &arg->imm_data, sizeof(arg->imm_data));
	return e;
}

static struct nftnl_expr *alloc_cmp(union nft_expr_arg *arg) {
	struct nftnl_expr *e;

	e = nftnl_expr_alloc("cmp");
	if (e == NULL)
		return NULL;

	nftnl_expr_set_u32(e, NFTNL_EXPR_CMP_SREG, arg->cmp_sreg);
	nftnl_expr_set_u32(e, NFTNL_EXPR_CMP_OP, arg->cmp_op);
	nftnl_expr_set(e, NFTNL_EXPR_CMP_DATA, &arg->cmp_data, sizeof(arg->cmp_data));
	return e;
}

static struct nftnl_expr *alloc_lookup(union nft_expr_arg *arg) {
	struct nftnl_expr *e;

	e = nftnl_expr_alloc("lookup");
	if (e == NULL)
		return NULL;

	nftnl_expr_set_str(e, NFTNL_EXPR_LOOKUP_SET, arg->lookup_set_name);
	nftnl_expr_set_u32(e, NFTNL_EXPR_LOOKUP_SET_ID, arg->lookup_set_id);
	nftnl_expr_set_u32(e, NFTNL_EXPR_LOOKUP_SREG, arg->lookup_sreg);
	return e;
}

static struct nftnl_expr *alloc_meta(union nft_expr_arg *arg) {
	struct nftnl_expr *e;

	e = nftnl_expr_alloc("meta");
	if (e == NULL)
		return NULL;

	nftnl_expr_set_u32(e, NFTNL_EXPR_META_KEY, arg->meta_key);
	nftnl_expr_set_u32(e, NFTNL_EXPR_META_DREG, arg->meta_dreg);
	return e;
}

static int nft_create_rule(struct nft_meta *meta, struct mnl_nlmsg_batch *batch) {
	struct nlmsghdr *nlh;
	struct nftnl_rule *r;
	struct nftnl_expr *e;
	struct nft_expr_fn funcs[] = {
		/* meta l4proto */
		{ .fn = alloc_meta,	.arg = {
						.meta_key = NFT_META_L4PROTO,
						.meta_dreg = NFT_REG_1,
					}, },
		/* { tcp, udp } */
		{ .fn = alloc_lookup,	.arg = {
						.lookup_set_name = meta->set_name,
						.lookup_set_id = meta->set_id,
						.lookup_sreg = NFT_REG_1,
					}, },
		/* meta mark */
		{ .fn = alloc_meta,	.arg = {
						.meta_key = NFT_META_MARK,
						.meta_dreg = NFT_REG32_00,
					}, },
		/* == <mark> */
		{ .fn = alloc_cmp,	.arg = {
						.cmp_sreg = NFT_REG32_00,
						.cmp_op = NFT_CMP_EQ,
						.cmp_data = meta->mark,
					}, },
		/* <port> */
		{ .fn = alloc_immediate, .arg = {
						.imm_dreg = NFT_REG_2,
						.imm_data = meta->tproxy_port,
					}, },
		/* tproxy to : */
		{ .fn = alloc_tproxy,	.arg = {
						.tproxy_family = NFPROTO_UNSPEC,
						.tproxy_reg_port = NFT_REG_2,
					}, },
		/* counter */
		{ .fn = alloc_counter,	.arg = {}, },
	};

	r = nftnl_rule_alloc();
	if (r == NULL)
		return -1;

	nftnl_rule_set_u32(r, NFTNL_RULE_FAMILY, meta->family);
	nftnl_rule_set_str(r, NFTNL_RULE_TABLE, meta->table_name);
	nftnl_rule_set_str(r, NFTNL_RULE_CHAIN, meta->chain_name);

	for (size_t i = 0; i < ARRAY_SIZE(funcs); i++) {
		e = funcs[i].fn(&funcs[i].arg);
		if (e == NULL) {
			nftnl_rule_free(r);
			return -1;
		}
		nftnl_rule_add_expr(r, e);
	}

	if (meta->verbose) {
		nftnl_rule_fprintf(stdout, r, 0, 0);
		fprintf(stdout, "\n\n");
	}

	nlh = nftnl_rule_nlmsg_build_hdr(mnl_nlmsg_batch_current(batch),
			NFT_MSG_NEWRULE, meta->family,
			NLM_F_CREATE|NLM_F_ACK, (*meta->seq)++);
	nftnl_rule_nlmsg_build_payload(nlh, r);
	nftnl_rule_free(r);
	mnl_nlmsg_batch_next(batch);

	return 0;
}

static struct nftnl_set_elem *alloc_set_elem(void *data, size_t size) {
	struct nftnl_set_elem *e;

	e = nftnl_set_elem_alloc();
	if (e == NULL)
		return NULL;

	nftnl_set_elem_set(e, NFTNL_SET_ELEM_KEY, data, size);
	return e;
}

static int nft_create_set_elem(struct nft_meta *meta, struct mnl_nlmsg_batch *batch) {
	struct nlmsghdr *nlh;
	struct nftnl_set *s;
	struct nftnl_set_elem *e;

	s = nftnl_set_alloc();
	if (s == NULL)
		return -1;

	nftnl_set_set_str(s, NFTNL_SET_TABLE, meta->table_name);
	nftnl_set_set_u32(s, NFTNL_SET_ID, meta->set_id);
	nftnl_set_set_str(s, NFTNL_SET_NAME, meta->set_name);

	for (size_t i = 0; i < meta->set_elem_len; i++) {
		e = alloc_set_elem(meta->set_elems+i, sizeof(meta->set_elems[i]));
		if (e == NULL) {
			nftnl_set_free(s);
			return -1;
		}

		nftnl_set_elem_add(s, e);
	}

	if (meta->verbose) {
		nftnl_set_fprintf(stdout, s, 0, 0);
		fprintf(stdout, "\n\n");
	}

	nlh = nftnl_set_elem_nlmsg_build_hdr(mnl_nlmsg_batch_current(batch),
			NFT_MSG_NEWSETELEM, meta->family,
			NLM_F_CREATE|NLM_F_ACK, (*meta->seq)++);
	nftnl_set_elems_nlmsg_build_payload(nlh, s);
	nftnl_set_free(s);
	mnl_nlmsg_batch_next(batch);

	return 0;
}

static int nft_create_set(struct nft_meta *meta, struct mnl_nlmsg_batch *batch) {
	struct nlmsghdr *nlh;
	struct nftnl_set *s;

	s = nftnl_set_alloc();
	if (s == NULL)
		return -1;

	nftnl_set_set_u32(s, NFTNL_SET_FAMILY, meta->family);
	nftnl_set_set_str(s, NFTNL_SET_TABLE, meta->table_name);
	nftnl_set_set_u32(s, NFTNL_SET_ID, meta->set_id);
	nftnl_set_set_str(s, NFTNL_SET_NAME, meta->set_name);
	nftnl_set_set_u32(s, NFTNL_SET_KEY_LEN, sizeof(*meta->set_elems));
	/* inet protocol type, see nftables/include/datatype.h */
	nftnl_set_set_u32(s, NFTNL_SET_KEY_TYPE, 12);

	if (meta->verbose) {
		nftnl_set_fprintf(stdout, s, 0, 0);
		fprintf(stdout, "\n\n");
	}

	nlh = nftnl_set_nlmsg_build_hdr(mnl_nlmsg_batch_current(batch),
			NFT_MSG_NEWSET, meta->family,
			NLM_F_CREATE|NLM_F_ACK, (*meta->seq)++);
	nftnl_set_nlmsg_build_payload(nlh, s);
	nftnl_set_free(s);
	mnl_nlmsg_batch_next(batch);

	return 0;
}

static int nft_create_chain(struct nft_meta *meta, struct mnl_nlmsg_batch *batch) {
	struct nlmsghdr *nlh;
	struct nftnl_chain *c;

	c = nftnl_chain_alloc();
	if (c == NULL)
		return -1;

	nftnl_chain_set_str(c, NFTNL_CHAIN_TABLE, meta->table_name);
	nftnl_chain_set_str(c, NFTNL_CHAIN_NAME, meta->chain_name);
	nftnl_chain_set_str(c, NFTNL_CHAIN_TYPE, meta->chain_type);
	nftnl_chain_set_u32(c, NFTNL_CHAIN_HOOKNUM, meta->chain_hooknum);
	nftnl_chain_set_u32(c, NFTNL_CHAIN_PRIO, meta->chain_prio);

	if (meta->verbose) {
		nftnl_chain_fprintf(stdout, c, 0, 0);
		fprintf(stdout, "\n\n");
	}

	nlh = nftnl_chain_nlmsg_build_hdr(mnl_nlmsg_batch_current(batch),
			NFT_MSG_NEWCHAIN, meta->family,
			NLM_F_CREATE|NLM_F_ACK, (*meta->seq)++);
	nftnl_chain_nlmsg_build_payload(nlh, c);
	nftnl_chain_free(c);
	mnl_nlmsg_batch_next(batch);

	return 0;
}

static int nft_operate_table(struct nft_meta *meta, struct mnl_nlmsg_batch *batch, bool create) {
	struct nlmsghdr *nlh;
	struct nftnl_table *t;
	enum nf_tables_msg_types type = create ? NFT_MSG_NEWTABLE : NFT_MSG_DELTABLE;
	uint16_t flags = create ? NLM_F_CREATE|NLM_F_ACK : NLM_F_ACK;

	t = nftnl_table_alloc();
	if (t == NULL)
		return -1;

	nftnl_table_set_u32(t, NFTNL_TABLE_FAMILY, meta->family);
	nftnl_table_set_str(t, NFTNL_TABLE_NAME, meta->table_name);

	if (meta->verbose) {
		nftnl_table_fprintf(stdout, t, 0, 0);
		fprintf(stdout, "\n\n");
	}

	nlh = nftnl_table_nlmsg_build_hdr(mnl_nlmsg_batch_current(batch),
			type, meta->family,
			flags, (*meta->seq)++);
	nftnl_table_nlmsg_build_payload(nlh, t);
	nftnl_table_free(t);
	mnl_nlmsg_batch_next(batch);

	return 0;
}

static int nft_create_table(struct nft_meta *meta, struct mnl_nlmsg_batch *batch) {
	return nft_operate_table(meta, batch, true);
}

static int nft_setup_cb(struct nft_meta *meta, struct mnl_nlmsg_batch *batch) {
	if (nft_create_table(meta, batch) != 0) {
		fprintf(stderr, "nft_create_table failed\n");
		return -1;
	}

	if (nft_create_chain(meta, batch) != 0) {
		fprintf(stderr, "nft_create_chain failed\n");
		return -1;
	}

	if (nft_create_set(meta, batch) != 0) {
		fprintf(stderr, "nft_create_set failed\n");
		return -1;
	}

	if (nft_create_set_elem(meta, batch) != 0) {
		fprintf(stderr, "nft_create_set failed\n");
		return -1;
	}

	if (nft_create_rule(meta, batch) != 0) {
		fprintf(stderr, "nft_create_rule failed\n");
		return -1;
	}

	return 0;
}

static int nft_msg(struct nft_meta *meta, nft_msg_cb_t cb) {
	unsigned seq, portid;

	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct mnl_nlmsg_batch *batch;
	struct mnl_socket *nl;
	int retval = -1;

	seq = time(NULL);
	meta->seq = &seq;

	batch = mnl_nlmsg_batch_start(buf, sizeof(buf));

	nftnl_batch_begin(mnl_nlmsg_batch_current(batch), seq++);
	mnl_nlmsg_batch_next(batch);

	if (cb(meta, batch) < 0)
		return -1;

	nftnl_batch_end(mnl_nlmsg_batch_current(batch), seq++);
	mnl_nlmsg_batch_next(batch);

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		pr_perror("mnl_socket_open");
		return -1;
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		pr_perror("mnl_socket_bind");
		goto out;
	}

	portid = mnl_socket_get_portid(nl);

	if (mnl_socket_sendto(nl, mnl_nlmsg_batch_head(batch),
				mnl_nlmsg_batch_size(batch)) < 0) {
		pr_perror("mnl_socket_sendto");
		goto out;
	}

	mnl_nlmsg_batch_stop(batch);

	retval = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (retval > 0) {
		retval = mnl_cb_run(buf, retval, 0, portid, NULL, NULL);
		if (retval <= 0)
			break;
		retval = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (retval == -1)
		pr_perror("error");

out:
	mnl_socket_close(nl);
	return retval;
}

int nft_setup(struct nft *nft) {
	uint8_t protocols[] = { IPPROTO_TCP, IPPROTO_UDP };
	struct nft_meta meta  = {
		.family		= nft->family,
		.table_name	= nft->table_name,
		.chain_name	= "input",
		.chain_type	= "nat",
		.chain_hooknum	= NF_INET_PRE_ROUTING,
		.chain_prio	= NF_IP_PRI_NAT_DST,
		.set_name	= nft->set_name,
		.set_elems	= protocols,
		.set_elem_len	= ARRAY_SIZE(protocols),
		.set_id		= 0, /* kernel will check the value, just set zero here */
		.mark		= nft->mark,
		.tproxy_port	= nft->tproxy_port,

		.verbose	= nft->verbose,
	};

	return nft_msg(&meta, nft_setup_cb);
}


static int nft_delete_table(struct nft_meta *meta, struct mnl_nlmsg_batch *batch) {
	return nft_operate_table(meta, batch, false);
}

int nft_destroy(struct nft *nft) {
	struct nft_meta meta = {
		.family		= nft->family,
		.table_name	= nft->table_name,
	};

	return nft_msg(&meta, nft_delete_table);
}
