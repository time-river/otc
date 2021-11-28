# - Try to find clang
# clang is used to generate llvm code

include (FindPackageHandleStandardArgs)

find_path (CLANG_CMD
	NAMES
	  clang
	PATHES
	  /usr/bin
	  /usr/sbin
	  /usr/local/bin
	  /usr/local/sbin
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS (clang
	"Please install the clang package"
	CLANG_CMD
)
