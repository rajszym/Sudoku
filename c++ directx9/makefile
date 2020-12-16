#**********************************************************#
#file     makefile
#author   Rajmund Szymanski
#date     10.12.2020
#brief    x86/x64 makefile.
#**********************************************************#

PROJECT    ?= sudoku
CROSS      ?=
DEFS       ?= WINDOWS UNICODE STATIC
DIRS       ?=
INCS       ?=
LIBS       ?= d3d9 d3dx9
KEYS       ?=

#----------------------------------------------------------#

KEYS       += *

#----------------------------------------------------------#

AS         := $(CROSS)gcc -x assembler-with-cpp
CC         := $(CROSS)gcc
CXX        := $(CROSS)g++
FOR        := $(CROSS)gfortran
COPY       := $(CROSS)objcopy
DUMP       := $(CROSS)objdump
SIZE       := $(CROSS)size
LD         := $(CROSS)g++
AR         := $(CROSS)ar
RES        := $(CROSS)windres

RM         ?= rm -f

#----------------------------------------------------------#

DTREE       = $(foreach d,$(foreach k,$(KEYS),$(wildcard $1$k)),$(dir $d) $(call DTREE,$d/))

VPATH      := $(sort $(call DTREE,) $(foreach d,$(DIRS),$(call DTREE,$d/)))

#----------------------------------------------------------#

INC_DIRS   := $(sort $(dir $(foreach d,$(VPATH),$(wildcard $d*.h $d*.hh $d*.hpp))))
LIB_DIRS   := $(sort $(dir $(foreach d,$(VPATH),$(wildcard $dlib*.a))))
OBJ_SRCS   :=              $(foreach d,$(VPATH),$(wildcard $d*.o))
AS_SRCS    :=              $(foreach d,$(VPATH),$(wildcard $d*.s))
C_SRCS     :=              $(foreach d,$(VPATH),$(wildcard $d*.c))
CC_SRCS    :=              $(foreach d,$(VPATH),$(wildcard $d*.cc))
CXX_SRCS   :=              $(foreach d,$(VPATH),$(wildcard $d*.cpp))
FOR_SRCS   :=              $(foreach d,$(VPATH),$(wildcard $d*.f))
RES_SRCS   :=              $(foreach d,$(VPATH),$(wildcard $d*.rc))
LIB_SRCS   :=     $(notdir $(foreach d,$(VPATH),$(wildcard $dlib*.a)))
ifeq ($(strip $(PROJECT)),)
PROJECT    := $(firstword $(notdir $(CURDIR)))
endif

#----------------------------------------------------------#

EXE        := $(PROJECT).exe
LIB        := lib$(PROJECT).a
LSS        := $(PROJECT).lss
MAP        := $(PROJECT).map

OBJS       := $(AS_SRCS:%.s=%.o)
OBJS       += $(C_SRCS:%.c=%.o)
OBJS       += $(CC_SRCS:%.cc=%.o)
OBJS       += $(CXX_SRCS:%.cpp=%.o)
OBJS       += $(FOR_SRCS:%.f=%.o)
OBJS       += $(RES_SRCS:%.rc=%.o)
DEPS       := $(OBJS:.o=.d)
LSTS       := $(OBJS:.o=.lst)

#----------------------------------------------------------#

COMMON_F    = -O2 -s
ifneq ($(filter CONSOLE,$(DEFS)),)
DEFS       := $(DEFS:CONSOLE=)
COMMON_F   += -mconsole
else
ifneq ($(filter WINDOWS,$(DEFS)),)
DEFS       := $(DEFS:WINDOWS=)
COMMON_F   += -mwindows
endif
endif
ifneq ($(filter STATIC,$(DEFS)),)
DEFS       := $(DEFS:STATIC=)
COMMON_F   += -static
endif
ifneq ($(filter USE_LTO,$(DEFS)),)
DEFS       := $(DEFS:USE_LTO=)
COMMON_F   += -flto
endif
ifneq ($(filter UNICODE,$(DEFS)),)
DEFS       += _UNICODE
COMMON_F   += -municode
else
ifneq ($(filter unicode,$(MAKECMDGOALS)),)
DEFS       += _UNICODE UNICODE
COMMON_F   += -municode
endif
endif
COMMON_F   += -Wall -Wextra -Wshadow -Wpedantic -Wsign-conversion
COMMON_F   += -MD -MP
COMMON_F   += -Wa,-amhls=$(@:.o=.lst)
ifneq ($(filter DEBUG,$(DEFS)),)
COMMON_F   += -g -ggdb
else
ifeq  ($(filter NDEBUG,$(DEFS)),)
DEFS       += NDEBUG
endif
endif

AS_FLAGS    =
C_FLAGS     = -std=gnu2x
CXX_FLAGS   = -std=gnu++2a -Wzero-as-null-pointer-constant
FOR_FLAGS   = -cpp
LD_FLAGS    = -Wl,-Map=$(MAP)

#----------------------------------------------------------#

DEFS_F     := $(DEFS:%=-D%)
LIBS       += $(LIB_SRCS:lib%.a=%)
LIBS_F     := $(LIBS:%=-l%)
OBJS_ALL   := $(sort $(OBJ_SRCS) $(OBJS))
INC_DIRS   += $(INCS:%=%/)
INC_DIRS_F := $(INC_DIRS:%=-I%)
LIB_DIRS_F := $(LIB_DIRS:%=-L%)

AS_FLAGS   += $(COMMON_F) $(DEFS_F) $(INC_DIRS_F)
C_FLAGS    += $(COMMON_F) $(DEFS_F) $(INC_DIRS_F)
CXX_FLAGS  += $(COMMON_F) $(DEFS_F) $(INC_DIRS_F)
FOR_FLAGS  += $(COMMON_F) $(DEFS_F) $(INC_DIRS_F)
LD_FLAGS   += $(COMMON_F)

GENERATED   = $(EXE) $(LIB) $(LSS) $(MAP) $(DEPS) $(LSTS) $(OBJS)

#----------------------------------------------------------#

all : $(LSS) print_exe_size

unicode : all

lib : $(LIB) print_size

run : all
	$(info Starting the program...)
ifneq ($(OS),Windows_NT)
	@chmod 777 ./$(EXE)
endif
	@./$(EXE)

clean :
	$(info Removing all generated output files)
	$(RM) $(GENERATED)

%.o : %.s
	$(info Assembling file: $<)
	$(AS) $(AS_FLAGS) -c $< -o $@

%.o : %.c
	$(info Compiling file: $<)
	$(CC) $(C_FLAGS) -c $< -o $@

%.o : %.cc
	$(info Compiling file: $<)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

%.o : %.cpp
	$(info Compiling file: $<)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

%.o : %.f
	$(info Compiling file: $<)
	$(FOR) $(FOR_FLAGS) -c $< -o $@

%.o : %.rc
	$(info Compiling file: $<)
	$(RES) $< $@

$(OBJS) : $(MAKEFILE_LIST)

$(EXE) : $(OBJS_ALL)
	$(info Linking target: $(EXE))
	$(LD) $(LD_FLAGS) $(OBJS_ALL) $(LIBS_F) $(LIB_DIRS_F) -o $@

$(LIB) : $(OBJS_ALL)
	$(info Building library: $(LIB))
	$(AR) -r $@ $?

$(LSS) : $(EXE)
	$(info Creating extended listing: $(LSS))
	$(DUMP) --demangle -S $< > $@

print_size : $(OBJS_ALL)
	$(info Size of modules:)
	$(SIZE) -B -t --common $(OBJS_ALL)

print_exe_size : $(EXE)
	$(info Size of target file:)
	$(SIZE) -B $(EXE)

.PHONY : all unicode lib run clean

-include $(DEPS)
