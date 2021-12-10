cmake_minimum_required (VERSION 3.10)

# set the project name
project (otc)

set (CMAKE_MODULE_PATH
	${CMAKE_MODULE_PATH}
	${CMAKE_CURRENT_SOURCE_DIR}/cmake
)

find_package (libbpf REQUIRED)
find_package (eBPF REQUIRED)
find_package (zlib REQUIRED)
find_package (libelf REQUIRED)
find_package (libmnl REQUIRED)
find_package (libnftnl REQUIRED)

# set package's include path
include_directories (include)

add_executable (otc
	include/generate/ebpf_kern.skel.h
	src/main.c
	src/nft.c
	src/param.c
	src/acl.c
	src/addr.c
)

# set dynamic libraries
target_link_libraries (otc bpf)
target_link_libraries (otc z)
target_link_libraries (otc elf)
target_link_libraries (otc mnl)
target_link_libraries (otc nftnl)
target_link_libraries (otc resolv)
