# - Try to find libelf
# used in libbpf

include (FindPackageHandleStandardArgs)

find_library (LIBELF_LIBRARIES
	NAMES
	  elf
	PATH_SUFFIXES
	  libelf
	PATHS
	  ENV LIBRARY_PATH
	  ENV LD_LIBRARY_PATH
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS (libelf
	"Please install the libelf development package"
	LIBELF_LIBRARIES
)
