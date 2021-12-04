# - Try to find libnftnl
# libnftnl is used in nft

include (FindPackageHandleStandardArgs)

find_path (LIBNFTNL_INCLUDE_DIRS
	NAMES
	  table.h
	  chain.h
	  rule.h
	  set.h
	  expr.h
	PATH_SUFFIXES
	  libnftnl
	PATHS
	  ENV CPATH
 )

find_library (LIBNFTNL_LIBRARIES
	NAMES
	  nftnl
	PATHS
	  ENV LIBRARY_PATH
	  ENV LD_LIBRARY_PATH
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS (libnftnl
	"Please install the libnftnl package"
	LIBNFTNL_INCLUDE_DIRS
	LIBNFTNL_LIBRARIES
)
