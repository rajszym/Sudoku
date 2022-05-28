#----------------------------------------------------------#

PROJECT    := sudoku
BUILD      := # build folder name
DEFS       := WINDOWS UNICODE
INCS       :=
SRCS       := sudoku.cpp
LIBS       := -ld2d1 -ldwrite

#----------------------------------------------------------#

GCC        := # toolchain path
OPTF       := # optimization level
STDC       := 11
STDCXX     := 20

############################################################

PROJECT    := $(firstword $(PROJECT) $(notdir $(CURDIR)))
BUILD      := $(firstword $(BUILD) build)

#----------------------------------------------------------#

ifneq ($(filter CLANG,$(DEFS)),)
AS         := $(GCC)clang
CC         := $(GCC)clang
CXX        := $(GCC)clang++
COPY       := $(GCC)llvm-objcopy
DUMP       := $(GCC)llvm-objdump
SIZE       := $(GCC)llvm-size
LD         := $(GCC)clang++
AR         := $(GCC)llvm-ar
else
AS         := $(GCC)gcc
CC         := $(GCC)gcc
CXX        := $(GCC)g++
FC         := $(GCC)gfortran
COPY       := $(GCC)objcopy
DUMP       := $(GCC)objdump
SIZE       := $(GCC)size
LD         := $(GCC)g++
AR         := $(GCC)ar
endif
RES        := $(GCC)windres

RM         ?= rm -f

#----------------------------------------------------------#

ifneq ($(OS),Windows_NT)
ELF        := $(BUILD)/$(PROJECT)
else
ELF        := $(BUILD)/$(PROJECT).exe
endif
LIB        := $(BUILD)/lib$(PROJECT).a
DMP        := $(BUILD)/$(PROJECT).dmp
LSS        := $(BUILD)/$(PROJECT).lss
MAP        := $(BUILD)/$(PROJECT).map

SRCS       := $(foreach s,$(SRCS),$(realpath $s))
OBJS       := $(SRCS:%=$(BUILD)%.o)
DEPS       := $(OBJS:.o=.d)

#----------------------------------------------------------#

ifneq (clean,$(MAKECMDGOALS))
COMMON_F   += -MD -MP
ifeq  ($(filter CLANG,$(DEFS)),)
COMMON_F   +=#-Wa,-amhls=$(@:.o=.lst)
endif
AS_FLAGS   += -xassembler-with-cpp
C_FLAGS    += -Wlogical-op
CXX_FLAGS  += -Wlogical-op
F_FLAGS    += -cpp
LD_FLAGS   += -Wl,-Map=$(MAP),--gc-sections
ifneq ($(filter ISO,$(DEFS)),)
$(info Using iso)
DEFS       := $(DEFS:ISO=)
C_FLAGS    += -std=c$(STDC:20=2x)
CXX_FLAGS  += -std=c++$(STDCXX:20=2a)
else
C_FLAGS    += -std=gnu$(STDC:20=2x)
CXX_FLAGS  += -std=gnu++$(STDCXX:20=2a)
endif
ifneq ($(filter CLANG,$(DEFS)),)
$(info Using clang)
else
DEFS       := $(DEFS:CLANG=)
COMMON_F   += -s
ifneq ($(filter CONSOLE,$(DEFS)),)
$(info Using console)
DEFS       := $(DEFS:CONSOLE=)
COMMON_F   += -mconsole
endif
ifneq ($(filter WINDOWS,$(DEFS)),)
$(info Using windows)
DEFS       := $(DEFS:WINDOWS=)
COMMON_F   += -mwindows
endif
ifneq ($(filter STATIC,$(DEFS)),)
$(info Using static)
DEFS       := $(DEFS:STATIC=)
COMMON_F   += -static
endif
ifneq ($(filter LTO,$(DEFS)),)
$(info Using lto)
DEFS       := $(DEFS:LTO=)
COMMON_F   += -flto
endif
endif
ifneq ($(filter UNICODE,$(DEFS)),)
$(info Using unicode)
DEFS       += _UNICODE
COMMON_F   += -municode
else
ifneq ($(filter unicode,$(MAKECMDGOALS)),)
$(info Using unicode)
DEFS       += _UNICODE UNICODE
COMMON_F   += -municode
endif
endif
ifneq ($(filter NOWARNINGS,$(DEFS)),)
$(info Using nowarnings)
DEFS       := $(DEFS:NOWARNINGS=)
COMMON_F   += -Wall
else
COMMON_F   += -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wsign-conversion
CXX_FLAGS  += -Wzero-as-null-pointer-constant
endif
ifneq ($(filter DEBUG,$(DEFS)),)
$(info Using debug)
COMMON_F   += -O$(OPTF) -g -ggdb
DEFS       := $(DEFS:NDEBUG=)
DEFS       := $(DEFS:MINSIZE=)
else
ifneq ($(filter MINSIZE,$(DEFS)),)
$(info Using minsize)
DEFS       := $(DEFS:MINSIZE=)
COMMON_F   += -Os
else
COMMON_F   += -O$(OPTF)
endif
ifeq  ($(filter NDEBUG,$(DEFS)),)
DEFS       += NDEBUG
endif
endif
endif

#----------------------------------------------------------#

DEFS_F     := $(DEFS:%=-D%)
INCS_F     := $(INCS:%=-I%)
LIB_DIRS_F := $(LIB_DIRS:%=-L%)
SCRIPT_F   := $(SCRIPT:%=-T%)

AS_FLAGS   += $(COMMON_F) $(DEFS_F) $(INCS_F)
C_FLAGS    += $(COMMON_F) $(DEFS_F) $(INCS_F)
CXX_FLAGS  += $(COMMON_F) $(DEFS_F) $(INCS_F)
F_FLAGS    += $(COMMON_F) $(DEFS_F) $(INCS_F)
LD_FLAGS   += $(COMMON_F) $(LIB_DIRS_F)

#----------------------------------------------------------#

$(info Using '$(MAKECMDGOALS)')

all : $(ELF) $(DMP) $(LSS) print_elf_size

unicode : all

lib : $(LIB) print_size

$(OBJS) : $(MAKEFILE_LIST)

$(BUILD)/%.S.o : /%.S
	$(info $<)
	mkdir -p $(dir $@)
	$(AS) $(AS_FLAGS) -c $< -o $@

$(BUILD)/%.s.o : /%.s
	$(info $<)
	mkdir -p $(dir $@)
	$(AS) $(AS_FLAGS) -c $< -o $@

$(BUILD)/%.c.o : /%.c
	$(info $<)
	mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) -c $< -o $@

$(BUILD)/%.cc.o : /%.cc
	$(info $<)
	mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(BUILD)/%.cpp.o : /%.cpp
	$(info $<)
	mkdir -p $(dir $@)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(BUILD)/%.f.o : /%.f
	$(info $<)
	mkdir -p $(dir $@)
	$(FC) $(F_FLAGS) -c $< -o $@

$(BUILD)/%.rc.o : /%.rc
	$(info $<)
	mkdir -p $(dir $@)
	$(RES) $< $@

$(ELF) : $(OBJS)
	$(info $@)
	$(LD) $(LD_FLAGS) $(OBJS) $(LIBS) -o $@
ifneq ($(OS),Windows_NT)
	@chmod 777 $@
endif

$(LIB) : $(OBJS)
	$(info $@)
	$(AR) -r $@ $?

$(DMP) : $(ELF)
	$(info $@)
	$(DUMP) -Ctx $< > $@

$(LSS) : $(ELF)
	$(info $@)
	$(DUMP) -CS $< > $@

print_size : $(OBJS)
	$(info Size of modules:)
	$(SIZE) -B -t --common $?

print_elf_size : $(ELF)
	$(info Size of target:)
	$(SIZE) -B $<

clean :
	$(info Removing all generated output files)
	$(RM) -Rd $(BUILD)

run : all
	$(info Running the target...)
	@$(ELF)

.PHONY : all unicode lib clean run

-include $(DEPS)
