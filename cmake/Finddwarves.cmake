# - Try to find pahole
# pahole is used to generate btf

include (FindPackageHandleStandardArgs)

find_path (DWARVES_CMD
	NAMES
	  pahole
	PATHES
	  /usr/bin
	  /usr/sbin
	  /usr/local/bin
	  /usr/local/sbin
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS (dwarves
	"Please install the dwarves package"
	DWARVES_CMD
)
