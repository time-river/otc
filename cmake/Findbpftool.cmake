# - Try to find bpftool
# bpftool is used to generate `vmlinux.h` and skeleton header

include (FindPackageHandleStandardArgs)

find_path (BPFTOOL_CMD
	NAMES
	  bpftool
	PATHES
	  /usr/bin
	  /usr/sbin
	  /usr/local/bin
	  /usr/local/sbin
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS (bpftool
	"Please install the bpftool package"
	BPFTOOL_CMD
)
