BIN   = csage
STLIB = libcsage.a
SHLIB = libcsage.so

CC     = gcc
LINKER = gcc

SRCDIR    = ./src
OBJDIR    = ./obj
SHADERDIR = ./shaders
LIBDIR    = ./lib

COMPILE_WITH = -DDEBUG_LEVEL=3
BUILD_WITH   =

WARNINGS = -Wall -Wextra -Wshadow -Wfloat-equal -Wpointer-arith -Wdangling-else -Wstrict-overflow=2 -Wrestrict \
           -Wstrict-aliasing -Wsuggest-attribute=noreturn -Wno-parentheses -Wno-missing-braces                 \
           -Wno-missing-field-initializers -Wno-unused-parameter -Wno-ignored-qualifiers
CFLAGS   = -std=c11 -march=native -Og -fstrict-aliasing -g2 -pedantic -ggdb -pipe $(WARNINGS) -I$(SRCDIR) \
           -isystem $(LIBDIR)/include -ftabstop=4 -include $(SRCDIR)/common.h $(COMPILE_WITH)
LUAFLAGS = -O0

LFLAGS   = -Wall -fuse-ld=gold -L$(LIBDIR) -Wl,-O3 -lm -lSDL2 -lvulkan
DEPFLAGS = -MT $@ -MMD -MP -MF $(OBJDIR)/$*.dep
STFLAGS  = -static-libgcc -static -D COMPILE_STATIC
SHFLAGS  = -fPIC -D COMPILE_SHARED

SRC  := $(wildcard $(SRCDIR)/util/*.c)   \
        $(wildcard $(SRCDIR)/gfx/*.c)     \
        $(wildcard $(SRCDIR)/entities/*.c) \
        $(wildcard $(SRCDIR)/*.c)
OBJ  := $(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEP  := $(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.dep)
GLSL := $(wildcard $(SHADERDIR)/*.vert) \
        $(wildcard $(SHADERDIR)/*.geom) \
        $(wildcard $(SHADERDIR)/*.tesc) \
        $(wildcard $(SHADERDIR)/*.tese) \
        $(wildcard $(SHADERDIR)/*.frag)
SPV  := $(GLSL:$(SHADERDIR)/%=$(SHADERDIR)/spirv/%)

PRECOMPILE  = mkdir -p $(@D)
POSTCOMPILE =

$(BIN): $(OBJ)
	@$(LINKER) -o $@ $(LFLAGS) $(OBJ)
	@echo "Linking complete"

$(OBJ): $(OBJDIR)/%.o: $(SRCDIR)/%.c
	@$(PRECOMPILE)
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -c $< -o $@
	@$(POSTCOMPILE)
	@echo "Compiled "$<" successfully"

-include $(DEP)
$(DEP): $(SRC)
	@$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $< -MM -MT $(@:.dep=.o) > $@

all: $(BIN)

.PHONY: static
static: CFLAGS += $(STFLAGS)
static: $(OBJ)
	@ar rcs $(STLIB) $(OBJ)

.PHONY: shared
shared: CFLAGS += $(SHFLAGS)
shared: $(OBJ)
	@$(LINKER) -o $(SHLIB) $(LFLAGS) -fPIC -shared $(OBJ)

.PHONY: spir-v
spir-v: $(SPV)
$(SPV): $(SHADERDIR)/spirv/%: $(SHADERDIR)/%
	@glslc -std=460 --target-env=vulkan1.2 -o $@ $<

.PHONY: valgrind
valgrind: BUILD_WITH += valgrind
valgrind: game

.PHONY: game
game: all
	@make spir-v -j12
	./$(BIN)
# 	@make shared -j12
# 	@$(BUILD_WITH) luajit $(LUAFLAGS) -e "CSAGE_DIM = $(DIM)  \
# 	                       dofile(\"src/csage.lua\")          \
# 	                       package.path = package.path ..      \
# 	                                      \";$(GAMEDIR)/?.lua\" \
# 	                       dofile(\"$(GAMEDIR)/game.lua\")"

.PHONY: test
test: COMPILE_WITH += -D RUN_TESTS
test: game
	@echo "Tests complete"

.PHONY: clean
clean:
	@rm -f $(OBJ)
	@echo "Cleanup complete"

.PHONY: remove
remove:	clean
	@rm -f ./$(BIN)
	@echo "Executable removed"
	@rm -f $(STLIB)
	@echo "Static library removed"
	@rm -f $(DEP)
	@echo "Dependency files removed"
	@rm -f $(SHADERDIR)/spirv/*
	@echo "Shader bytecode removed"
