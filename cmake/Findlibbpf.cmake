# - Try to find libbpf

include (FindPackageHandleStandardArgs)

find_path (LIBBPF_INCLUDE_DIRS
	NAMES
	  bpf/bpf.h
	  bpf/btf.h
	  bpf/libbpf.h
	PATHS
	  ENV CPATH
)

find_library (LIBBPF_LIBRARIES
	NAMES
	  bpf
	PATHS
	  ENV LIBRARY_PATH
	  ENV LD_LIBRARY_PATH
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS (libbpf
	"Please install the libbpf development package"
	LIBBPF_INCLUDE_DIRS
	LIBBPF_LIBRARIES
)
