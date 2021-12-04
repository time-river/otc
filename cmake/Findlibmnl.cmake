# - Try to find libmnl
# libmnl is used in nft

include (FindPackageHandleStandardArgs)

find_path (LIBMNL_INCLUDE_DIRS
	NAMES
	  libmnl.h
	PATH_SUFFIXES
	  libmnl
	PATHS
	  ENV CPATH
)

find_library (LIBMNL_LIBRARIES
	NAMES
	  mnl
	PATHS
	  ENV LIBRARY_PATH
	  ENV LD_LIBRARY_PATH
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS (libmnl
	"Please install the libmnl package"
	LIBMNL_INCLUDE_DIRS
	LIBMNL_LIBRARIES
)
