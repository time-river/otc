# - to generate skeleton header
# This likes the third library, the customizing building command
# defined in `ebpf/Makefile`.

include (FindPackageHandleStandardArgs)

# bpftool is used to to generate `vmlinux.h` and skeleton header
find_package (bpftool REQUIRED)
# clang is used to generate llvm code
find_package (clang REQUIRED)
# clang is used to generate eBPF code
find_package (llvm REQUIRED)
# pahole is used to generate btf
find_package (dwarves)

SET (EBPF_GENERATED_FILE
	${CMAKE_CURRENT_SOURCE_DIR}/ebpf/include
	${CMAKE_CURRENT_SOURCE_DIR}/ebpf/ebpf_kern.o
	${CMAKE_CURRENT_SOURCE_DIR}/include/generate/ebpf_kern.skel.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/generated
)

add_custom_command (
	OUTPUT
	  ${EBPF_GENERATED_FILE}
	COMMAND
	  make all
	WORKING_DIRECTORY
	  ${CMAKE_CURRENT_SOURCE_DIR}/ebpf
	DEPENDS
	  ${CMAKE_CURRENT_SOURCE_DIR}/include/ebpf.h
	  ${CMAKE_CURRENT_SOURCE_DIR}/ebpf/ebpf_kern.c
	  ${CMAKE_CURRENT_SOURCE_DIR}/ebpf/Makefile
)

add_custom_target (ebpf
	DEPENDS
	  ${EBPF_GENERATED_FILE}
)

# clean generating files
set_property (
	TARGET
	  ebpf
	APPEND
	PROPERTY
	  ADDITIONAL_CLEAN_FILES ${EBPF_GENERATED_FILE}
)
