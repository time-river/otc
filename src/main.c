#include "generated/ebpf_kern.skel.h"
#include "param.h"

int main(int argc, char *argv[]) {
	if (parse_options(argc, argv) != 0)
		return -1;

	return 0;
}
