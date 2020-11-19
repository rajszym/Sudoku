#**********************************************************#
#file     makefile
#author   Rajmund Szymanski
#date     19.11.2020
#brief    x86/x64 makefile.
#**********************************************************#

CROSS      :=

#----------------------------------------------------------#

PROJECT    ?= sudoku
DEFS       ?= CONSOLE UNICODE USE_STATIC
DIRS       ?=
INCS       ?=
LIBS       ?=
KEYS       ?=

#----------------------------------------------------------#

LIBS       +=#advapi32 gdi32 gdiplus msimg32 ole32 shlwapi user32 wtsapi32 comctl32 ddraw d3d9 d3dx9 d2d1 dwrite
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

INC_DIRS   := $(sort $(dir $(foreach d,$(VPATH),$(wildcard $d*.h $d*.hpp))))
LIB_DIRS   := $(sort $(dir $(foreach d,$(VPATH),$(wildcard $dlib*.a $d*.ld))))
OBJ_SRCS   :=              $(foreach d,$(VPATH),$(wildcard $d*.o))
AS_SRCS    :=              $(foreach d,$(VPATH),$(wildcard $d*.s))
C_SRCS     :=              $(foreach d,$(VPATH),$(wildcard $d*.c))
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
OBJS       += $(CXX_SRCS:%.cpp=%.o)
OBJS       += $(FOR_SRCS:%.f=%.o)
OBJS       += $(RES_SRCS:%.rc=%.o)
DEPS       := $(OBJS:.o=.d)
LSTS       := $(OBJS:.o=.lst)

#----------------------------------------------------------#

COMMON_F    = -O2 -s
ifneq ($(filter CONSOLE,$(DEFS)),)
COMMON_F   += -mconsole
else
ifneq ($(filter WINDOWS,$(DEFS)),)
COMMON_F   += -mwindows
endif
endif
ifneq ($(filter USE_STATIC,$(DEFS)),)
COMMON_F   += -static
endif
ifneq ($(filter USE_LTO,$(DEFS)),)
COMMON_F   += -flto
endif
ifneq ($(filter UNICODE,$(DEFS)),)
COMMON_F   += -municode -D_UNICODE
else
ifneq ($(filter unicode,$(MAKECMDGOALS)),)
COMMON_F   += -municode -D_UNICODE -DUNICODE
endif
endif
COMMON_F   += -Wall -Wextra -Wshadow -Wpedantic
COMMON_F   +=#-MD -MP
COMMON_F   +=#-Wa,-amhls=$(@:.o=.lst)
COMMON_F   +=#-g -ggdb

AS_FLAGS    =
C_FLAGS     = -std=gnu2x
CXX_FLAGS   = -std=gnu++2a -Wzero-as-null-pointer-constant
FOR_FLAGS   = -cpp
LD_FLAGS    =#-Wl,-Map=$(MAP)

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

all : $(EXE) print_exe_size

unicode : all

lib : $(LIB) print_size

flash: all
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

.PHONY : all unicode lib clean flash

-include $(DEPS)
