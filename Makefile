# Directories
SRCDIR := src
FRTDIR := $(SRCDIR)/frontend
BCKDIR := $(SRCDIR)/backend
ASMDIR := $(SRCDIR)/assemble
INCDIR := $(SRCDIR)/include
BUILDDIR := build
TESTDIR := test
TESTSRC := my_test

# Sources and generated files
MAIN_SRC := $(SRCDIR)/tiny_main.c

FRONT_SRCS := $(FRTDIR)/e_proc.c   \
		$(FRTDIR)/e_tac.c	\

FRONT_INCS := $(INCDIR)/e_proc.h   \
		$(INCDIR)/e_tac.h \
		$(INCDIR)/e_custom.h \
		$(INCDIR)/e_internal.h \
		$(INCDIR)/e_config.h
LEX_FRONT_SRC := $(FRTDIR)/e.l
YACC_FRONT_SRC := $(FRTDIR)/e.y

BACK_SRCS := $(BCKDIR)/o_wrap.c \
 		$(BCKDIR)/o_reg.c

BACK_INCS := $(INCDIR)/o_reg.h \
		$(INCDIR)/o_wrap.h \
		$(INCDIR)/o_riscv.h

# Shared library sources
LIB_CUSTOM_SRC := $(FRTDIR)/e_custom.c
LIB_INTERNAL_SRC := $(FRTDIR)/e_internal.c
LIB_RISCV_SRC := $(BCKDIR)/o_riscv.c

# Shared library outputs
LIB_CUSTOM := $(BUILDDIR)/libcustom.so
LIB_INTERNAL := $(BUILDDIR)/libinternal.so
LIB_RISCV := $(BUILDDIR)/libriscv.so
LIBS := $(LIB_INTERNAL) $(LIB_RISCV) $(LIB_CUSTOM)

# Build outputs
LEX_FRONT_C := $(BUILDDIR)/e.l.c
YACC_FRONT_C := $(BUILDDIR)/e.y.c
YACC_FRONT_H := $(BUILDDIR)/e.y.h
YACC_FRONT_OUT := $(BUILDDIR)/e.y.output

COMPILER_OUT := $(BUILDDIR)/e

# Tools and flags
LEX	:= flex
YACC   := bison
CC	 := gcc
CFLAGS := -I$(INCDIR) -g -fPIC
LOAD_SO := -L$(BUILDDIR) -linternal -lriscv -lcustom

.PHONY: all clean test

all: $(BUILDDIR) $(LIBS) $(COMPILER_OUT)

# Ensure build directory exists
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Build shared libraries
$(LIB_CUSTOM): $(LIB_CUSTOM_SRC) $(INCDIR)/e_custom.h
	$(CC) $(CFLAGS) -shared -o $@ $<

$(LIB_INTERNAL): $(LIB_INTERNAL_SRC) $(INCDIR)/e_internal.h
	$(CC) $(CFLAGS) -shared -o $@ $<

$(LIB_RISCV): $(LIB_RISCV_SRC) $(INCDIR)/o_riscv.h
	$(CC) $(CFLAGS) -shared -o $@ $^

# Build main target
$(COMPILER_OUT): $(FRONT_SRCS) $(BACK_SRCS) $(LEX_FRONT_SRC) $(YACC_FRONT_SRC) $(FRONT_INCS) $(BACK_INCS) $(LIBS)| $(BUILDDIR)
	$(LEX) -o $(LEX_FRONT_C) $(LEX_FRONT_SRC)
	$(YACC) -d -v -o $(YACC_FRONT_C) $(YACC_FRONT_SRC)
	$(CC) $(CFLAGS) -o $@ $(LEX_FRONT_C) $(YACC_FRONT_C) $(FRONT_SRCS) $(BACK_SRCS) $(MAIN_SRC) $(LOAD_SO)

clean:
	rm -rf $(BUILDDIR) $(TESTDIR)/*.s $(TESTDIR)/*.x $(TESTDIR)/*.o

test: all
	export LD_LIBRARY_PATH=$(BUILDDIR):$$LD_LIBRARY_PATH; \
	./$(COMPILER_OUT) ./$(TESTDIR)/$(TESTSRC).c