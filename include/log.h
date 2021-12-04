#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

#define pr_err(fmt, ...)	fprintf(stderr, "ERROR: " fmt, ##__VA_ARGS__)
#define pr_warn(fmt, ...)	fprintf(stdout, "WARN: " fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...)	fprintf(stdout, "INFO: " fmt, ##__VA_ARGS__)

#endif /* __LOG_H__ */
