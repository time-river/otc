# - Try to find llvm
# llvm is used to generate eBPF code

include (FindPackageHandleStandardArgs)

find_path (LLVM_CMD
	NAMES
	  llc
	  opt
	  llvm-dis
	  llvm-objcopy
	PATHES
	  /usr/bin
	  /usr/sbin
	  /usr/local/bin
	  /usr/local/sbin
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS (llvm
	"Please install the llvm package"
	LLVM_CMD
)
