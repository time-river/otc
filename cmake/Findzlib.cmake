# - Try to find libz

include (FindPackageHandleStandardArgs)

find_library (LIBZ_LIBRARIES
	NAMES
	  z
	PATHS
	  ENV LIBRARY_PATH
	  ENV LD_LIBRARY_PATH
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS (zlib
	"Please install the libz package"
	LIBZ_LIBRARIES
)
