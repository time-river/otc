/*
 * (C) 2021 by river <river@vvl.me>
 *
 * Parse IPv4/IPv6 list and do callback.
 *
 */

#include <string.h>
#include <alloca.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "acl.h"
#include "log.h"

static int parse_and_load_acl(struct acl_meta *meta, uint8_t family, load_fn_t fn) {
	size_t addr_len = family == AF_INET ? sizeof(in_addr_t) : sizeof(struct in6_addr);
	FILE *fp = fopen(meta->filename, "r");
	char *line = NULL;
	size_t len = 0;
	ssize_t nread;

	if (fp == NULL) {
		pr_perror("open '%s' failed", filename);
		return -1;
	}

	while ((nread = getline(&line, &len, fp) != -1)) {
		struct addr_info addr;

		if (line == NULL)
			continue;

		line[strlen(line)-1] = '\0';
		addr.bits = inet_net_pton(family, line, addr.v4, addr_len);
		if (addr.bits <= 0) {
			pr_warn("'%s' has wrong format!", line);
			continue;
		}

		if (fn(meta, &addr) != 0) {
			free(line);
			return -1;
		}
	}

	free(line);
	return 0;
}

static int load_default_ipv4_acl(struct acl_meta *meta, load_fn_t fn) {
	struct addr_info ip4[] = {
		{ .bits =  8, .v4 = { 0x7f, 0x00, 0x00, 0x00 }, }, /* 127.0.0.0/8 */
		{ .bits = 16, .v4 = { 0xa9, 0xfe, 0x00, 0x00 }, }, /* 169.254.0.0/16 */
		{ .bits = 32, .v4 = { 0xff, 0xff, 0xff, 0xff }, }, /* 255.255.255.255/32 */
		{ .bits = 16, .v4 = { 0xc0, 0xa8, 0x00, 0x00 }, }, /* 192.168.0.0/16 */
	};

	for (int i = 0; i < sizeof(ip4)/sizeof(ip4[0]); i++) {
		if (fn(meta, ip4+i) != 0)
			return -1;
	}

	return 0;
}

int load_ipv4_acl(struct acl_meta *meta, load_fn_t fn) {
	int retval = load_default_ipv4_acl(meta, fn);

	return retval == 0 ? parse_and_load_acl(meta, AF_INET, fn) : retval;
}

static int load_default_ipv6_acl(struct acl_meta *meta, load_fn_t fn) {
	struct addr_info ip6[] = {
		{ .bits = 128, .v6 = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } }, /* ::1 */
		{ .bits =   7, .v6 = { 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* fc00::/7 */
		{ .bits =  10, .v6 = { 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* fe80::/10 */
	};

	for (int i = 0; i < sizeof(ip6)/sizeof(ip6[0]); i++) {
		if (fn(meta, ip6+i) != 0)
			return -1;
	}

	return 0;
}

int load_ipv6_acl(struct acl_meta *meta, load_fn_t fn) {
	int retval = load_default_ipv6_acl(meta, fn);

	return retval == 0 ? parse_and_load_acl(meta, AF_INET6, fn) : retval;
}
