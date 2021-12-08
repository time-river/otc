#ifndef __ACL_H__
#define __ACL_H__

#include <netinet/in.h>

#include <bpf/bpf.h>

#include "ebpf.h"

struct addr_info {
	union {
		struct bpf_lpm_trie_key key;
		struct {
			int bits;
			union {
				char v4[sizeof(in_addr_t)];
				char v6[sizeof(struct in6_addr)];
			};
		};
	};
};

struct acl_meta {
	char *filename;
	int fd;
	struct trie_val *val;
};

typedef int (*load_fn_t)(struct acl_meta *meta, struct addr_info *ip);

int load_ipv4_acl(struct acl_meta *meta, load_fn_t fn);
int load_ipv6_acl(struct acl_meta *meta, load_fn_t fn);

#endif /* __ACL_H__ */
