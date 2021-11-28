#!/usr/bin/make

CLANG			:=	clang
LLC			:=	llc
OPT			:=	opt
LLVM_DIS		:=	llvm-dis
LLVM_OBJCOPY		:=	llvm-objcopy
BTF_PAHOLE		:=	pahole

NOSTDINC_FLAGS	:= -nostdinc -isystem $(shell $(CC) -print-file-name=include) -I /usr/include -I include -I ../include
BTF_LLC_PROBE := $(shell $(LLC) -march=bpf -mattr=help 2>&1 | grep dwarfris)
BTF_PAHOLE_PROBE := $(shell $(BTF_PAHOLE) --help 2>&1 | grep BTF)
BTF_OBJCOPY_PROBE := $(shell $(LLVM_OBJCOPY) --help 2>&1 | grep -i 'usage.*llvm')
BTF_LLVM_PROBE := $(shell echo "int main() { return 0; }" | \
				  $(CLANG) -target bpf -O2 -g -c -x c - -o ./llvm_btf_verify.o; \
				  readelf -S ./llvm_btf_verify.o 2>/dev/null | grep BTF; \
				  /bin/rm -f ./llvm_btf_verify.o)

BPF_EXTRA_CFLAGS	:=
LLC_EXTRA_FLAGS		:=

ifneq ($(BTF_LLVM_PROBE),)
	# if llvm support btf, just add `-g`
	BPF_EXTRA_CFLAGS += -g
else
	# otherwise generate info from dwarfris
ifneq ($(and $(BTF_LLC_PROBE),$(BTF_PAHOLE_PROBE),$(BTF_OBJCOPY_PROBE)),)
	BPF_EXTRA_CFLAGS += -g
	LLC_EXTRA_FLAGS += -mattr=dwarfris
	DWARF2BTF = y
endif
endif

CLANG_PKG	:= $(shell command -v clang > /dev/null; echo $$?)
ifneq ($(CLANG_PKG),0)
  $(error "no clang, install clang, echo ${CLANG_PKG}")
endif

LLVM_PKG	:= $(shell command -v llc > /dev/null; echo $$?)
ifneq ($(LLVM_PKG),0)
  $(error "no llvm, install llvm")
endif

LIBBPF_PKG	:= $(shell pkg-config --exists libbpf; echo $$?)
ifneq ($(LIBBPF_PKG),0)
  $(error "no libbpf or pkg-config, install pkgconfig and libbpf-dev or libbpf-devel")
endif

BPFTOOL_PKG	:= $(shell command -v bpftool > /dev/null; echo $$?)
ifneq ($(BPFTOOL_PKG),0)
  $(error "no bpftool, install bpftool")
endif

DWARVES_PKG	:= $(shell command -v pahole > /dev/null; echo $$?)
ifneq ($(DWARVES_PKG),0)
  $(warning "no dwarves, recommend to install dwarves")
endif

ARCH	:= __$(shell uname -m)__

vmlinux.h:
	mkdir -p include/generated
	bpftool btf dump file /sys/kernel/btf/vmlinux format c > include/generated/vmlinux.h

ebpf_kern.o: ebpf_kern.c vmlinux.h
	$(CLANG) -target bpf	\
		$(NOSTDINC_FLAGS) $(BPF_EXTRA_CFLAGS)	\
		-D$(ARCH) -O2 -emit-llvm -Xclang -disable-llvm-passes -c $< -o - |	\
		$(OPT) -O2 -mtriple=bpf-pc-linux | $(LLVM_DIS) |	\
		$(LLC) $(LLC_EXTRA_FLAGS) -march=bpf -filetype=obj -o $@
ifeq ($(DWARF2BTF),y)
	$(BTF_PAHOLE) -J $@
endif

ebpf_kern.skel.h: ebpf_kern.o
	mkdir -p ../include/generated
	bpftool gen skeleton $< > ../include/generated/$@

clean:
	rm -rf include/generated ../include/generated ebpf_kern.o

all: ebpf_kern.skel.h

.DEFAULT_GOAL	:= ebpf_kern.skel.h # set default target
.PHONY:	all clean
