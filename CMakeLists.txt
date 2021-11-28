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

# set package's include path
include_directories (include)

add_executable (otc
	include/generate/ebpf_kern.skel.h
	src/main.c
)

# set dynamic libraries
target_link_libraries (otc bpf)
target_link_libraries (otc z)
target_link_libraries (otc elf)
