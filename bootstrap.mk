#-------------------------------------------------------------------------------
# Bootstrapping cmake
#-------------------------------------------------------------------------------
# Copyright (c) 2015 Serge Aleynikov
# Date: 2014-08-12
#-------------------------------------------------------------------------------

PROJECT  := $(shell sed -n '/^project/{s/^project. *\([a-zA-Z0-9]\+\).*/\1/p; q}' CMakeLists.txt)
VERSION  := $(shell sed -n '/^project/{s/^.\+VERSION \+//; s/[^\.0-9]\+//; p; q}' CMakeLists.txt)

HOSTNAME := $(shell hostname)

# Options file is either: .cmake-args.$(HOSTNAME) or .cmake-args
OPT_FILE    := .cmake-args.$(HOSTNAME)
ifeq "$(wildcard $(OPT_FILE))" ""
    OPT_FILE:= .cmake-args
    ifeq "$(wildcard $(OPT_FILE))" ""
        OPT_FILE:="/dev/null"
    endif
endif

#-------------------------------------------------------------------------------
# Default target
#-------------------------------------------------------------------------------
all:
	@echo
	@echo "Run: make bootstrap [toolchain=gcc|clang|intel] [verbose=true] \\"
	@echo "                    [generator=ninja|make] [build=Debug|Release]"
	@echo
	@echo "To customize variables for cmake, create a local file with VAR=VALUE pairs:"
	@echo "  '.cmake-args.$(HOSTNAME)' or '.cmake-args'"
	@echo ""
	@echo "There are three sets of variables present there:"
	@echo "  1. DIR:BUILD=...   - Build directory"
	@echo "     DIR:INSTALL=... - Install directory"
	@echo
	@echo "     They may contain macros:"
	@echo "         @PROJECT@   - name of current project (from CMakeList.txt)"
	@echo "         @VERSION@   - project version number  (from CMakeList.txt)"
	@echo "         @BUILD@     - build type (from command line)"
	@echo "         \$${...}      - environment variable"
	@echo
	@echo "  2. ENV:VAR=...     - Environment var set before running cmake"
	@echo
	@echo "  3. VAR=...         - Variable passed to cmake with -D prefix"
	@echo

toolchain   ?= gcc
build       ?= Debug
BUILD       := $(shell echo $(build) | tr 'A-Z' 'a-z')

# Function that replaces variables in a given entry in a file 
# E.g.: $(call substitute,ENTRY,FILENAME)
substitute   = $(shell sed -n '/^$(1)=/{s!$(1)=!!; s!/\+$$!!;   \
                                       s!@PROJECT@!$(PROJECT)!; \
                                       s!@project@!$(PROJECT)!; \
                                       s!@VERSION@!$(VERSION)!; \
                                       s!@version@!$(VERSION)!; \
                                       s!@BUILD@!$(BUILD)!;     \
                                       s!@build@!$(BUILD)!;     \
                                       p; q}' $(2) 2>/dev/null)
BLD_DIR     := $(call substitute,DIR:BUILD,$(OPT_FILE))
ROOT_DIR    := $(dir $(abspath include))
DEF_BLD_DIR := $(ROOT_DIR:%/=%)/build
DIR         := $(if $(BLD_DIR),$(BLD_DIR),$(DEF_BLD_DIR))
PREFIX      := $(call substitute,DIR:INSTALL,$(OPT_FILE))
prefix      := $(if $(PREFIX),$(PREFIX),/usr/local)
generator   ?= make

#-------------------------------------------------------------------------------
# info target
#-------------------------------------------------------------------------------
info:
	@echo "PROJECT:   $(PROJECT)"
	@echo "HOSTNAME:  $(HOSTNAME)"
	@echo "VERSION:   $(VERSION)"
	@echo "OPT_FILE:  $(OPT_FILE)"
	@echo "BLD_DIR:   $(BLD_DIR)"
	@echo "build:     $(BUILD)"
	@echo "prefix:    $(prefix)"
	@echo "generator: $(generator)"

variables   := $(filter-out toolchain=% generator=% build=% verbose=%,$(MAKEOVERRIDES))
makevars    := $(variables:%=-D%)

envvars     += $(shell sed -n '/^ENV:/{s/^ENV://;p}' $(OPT_FILE) 2>/dev/null)
makevars    += $(patsubst %,-D%,$(shell sed -n '/^...:/!p' $(OPT_FILE) 2>/dev/null))

#-------------------------------------------------------------------------------
# bootstrap target
#-------------------------------------------------------------------------------
bootstrap: $(DIR)
    ifeq "$(generator)" ""
		@echo -e "\n\e[1;31mBuild tool not specified!\e[0m\n" && false
    else ifeq "$(shell which $(generator) 2>/dev/null)" ""
		@echo -e "\n\e[1;31mBuild tool $(generator) not found!\e[0m\n" && false
    endif
	@echo "Options file.....: $(OPT_FILE)"
	@echo "Build directory..: $(DIR)"
	@echo "Install directory: $(prefix)"
	@echo "Build type.......: $(BUILD)"
	@echo "Command-line vars: $(variables)"
	@echo -e "\n-- \e[1;37mUsing $(generator) generator\e[0m\n"
	$(envvars) cmake -H. -B$(DIR) \
        $(if $(findstring $(generator),ninja),-GNinja,-G"Unix Makefiles") \
        $(if $(findstring $(verbose),true on 1),-DCMAKE_VERBOSE_MAKEFILE=true) \
        -DTOOLCHAIN=$(toolchain) \
        -DCMAKE_USER_MAKE_RULES_OVERRIDE=$(ROOT_DIR:%/=%)/build-aux/CMakeInit.txt \
        -DCMAKE_INSTALL_PREFIX=$(prefix) \
        -DCMAKE_BUILD_TYPE=$(build) $(makevars)
	@rm -f build install
	@ln -s $(DIR) build
	@ln -s $(prefix) install
	@echo "PROJECT   := $(PROJECT)"             >  $(DIR)/cache.mk
	@echo "VERSION   := $(VERSION)"             >> $(DIR)/cache.mk
	@echo "OPT_FILE  := $(abspath $(OPT_FILE))" >> $(DIR)/cache.mk
	@echo "generator := $(generator)"           >> $(DIR)/cache.mk
	@echo "build     := $(BUILD)"               >> $(DIR)/cache.mk
	@echo "DIR       := $(DIR)"                 >> $(DIR)/cache.mk
	@echo "prefix    := $(prefix)"              >> $(DIR)/cache.mk

$(DIR):
	@mkdir -p $@

.PHONY: all bootstrap info
