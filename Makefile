BIN   = csage
STLIB = libcsage.a
SHLIB = libcsage.so

CC     = gcc
LINKER = gcc

SRC_DIR    = ./src
BUILD_DIR  = ./build
SHADER_DIR = ./shaders
GFX_DIR    = ./gfx
LIB_DIR    = ./lib
TOOL_DIR   = ./tools

COMPILE_WITH = -DDEBUG_LEVEL=5

WARNINGS = -Wall -Wextra -Wshadow -Wpointer-arith -Wdangling-else -Wstrict-overflow=2 -Wrestrict                      \
           -Wstrict-aliasing=3 -Wno-missing-braces -Wno-unused-function -Wold-style-definition -Wold-style-declaration \
           -Wmissing-prototypes -Wstrict-prototypes -Wunsafe-loop-optimizations -Wbad-function-cast -Wmissing-noreturn  \
           -Wdisabled-optimization -Wno-unused-variable # Optimization: -Winline
CFLAGS = -std=c2x -march=native -Og -fstrict-aliasing -g2 -ggdb -pipe $(WARNINGS) -I$(SRC_DIR)           \
         -isystem$(LIB_DIR)/include -ftabstop=4 -include $(SRC_DIR)/common.h -include $(SRC_DIR)/config.h \
         $(COMPILE_WITH) -fstack-protector-strong -fstack-clash-protection -fno-omit-frame-pointer         \
         -fsanitize=address -fsanitize=undefined -fsanitize-address-use-after-scope

LFLAGS   = -L$(LIB_DIR) -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined -fsanitize-address-use-after-scope \
           -lm -lvulkan -Wl,-rpath,$(LIB_DIR) -lSDL3 -lfreetype
DEPFLAGS = -MT $@ -MMD -MF $(BUILD_DIR)/$*.dep
STFLAGS  = -static-libgcc -static -D COMPILE_STATIC
SHFLAGS  = -fPIC -D COMPILE_SHARED

SRC := $(wildcard $(SRC_DIR)/util/*.c) \
       $(wildcard $(SRC_DIR)/gfx/*.c)   \
       $(wildcard $(SRC_DIR)/gfx/ui/*.c) \
       $(wildcard $(SRC_DIR)/map/*.c)     \
       $(wildcard $(SRC_DIR)/maths/*.c)    \
       $(wildcard $(SRC_DIR)/lang/*.c)      \
       $(wildcard $(SRC_DIR)/entities/*.c)   \
       $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEP := $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.dep)
GLSL := $(wildcard $(SHADER_DIR)/*.vert) \
        $(wildcard $(SHADER_DIR)/*.geom) \
        $(wildcard $(SHADER_DIR)/*.tesc) \
        $(wildcard $(SHADER_DIR)/*.tese) \
        $(wildcard $(SHADER_DIR)/*.frag)
SPV := $(GLSL:$(SHADER_DIR)/%=$(SHADER_DIR)/spirv/%)

PRECOMPILE  = mkdir -p $(@D)
POSTCOMPILE =

$(BIN): $(OBJ)
	@$(LINKER) -o $@ $(LFLAGS) $(OBJ)
	@echo "Linking complete"

$(OBJ): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(PRECOMPILE)
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -c $< -o $@
	@$(POSTCOMPILE)
	@echo "Compiled "$<" successfully"

-include $(OBJ:.o=.dep)

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
$(SPV): $(SHADER_DIR)/spirv/%: $(SHADER_DIR)/%
	@glslc -std=460 --target-env=vulkan1.3 -O -o $@ $<

.PHONY: spright
spright:
	@$(TOOL_DIR)/spright/spright -i $(GFX_DIR)/sprites/spright.conf

.PHONY: valgrind
valgrind: BUILD_WITH += valgrind
valgrind: game

.PHONY: game
game:
	@make -j12
	@make spir-v -j12
	./$(BIN)

.PHONY: lang
lang:
	@make COMPILE_WITH='-DDEBUG_LEVEL=5 -DTESTING -DTESTING_LANG' -j12
	./$(BIN)
	@echo "Tests complete"

.PHONY: test
test:
	@make COMPILE_WITH='-DDEBUG_LEVEL=5 -DTESTING -DTESTING_UTIL' -j12
	./$(BIN)
	@echo "Tests complete"

.PHONY: libs
libs:
	@mkdir -p $(LIB_DIR)/include/stb
	@cp $(LIB_DIR)/stb/*.h $(LIB_DIR)/include/stb

	@mkdir -p $(LIB_DIR)/include/cgltf
	@cp $(LIB_DIR)/cgltf/*.h $(LIB_DIR)/include/cgltf

	@mkdir -p $(LIB_DIR)/include/jsmn
	@cp $(LIB_DIR)/jsmn/*.h $(LIB_DIR)/include/jsmn

	@mkdir -p $(LIB_DIR)/include/freetype
	@cmake -S $(LIB_DIR)/freetype -B $(LIB_DIR)/freetype/build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=true
	@cmake --build $(LIB_DIR)/freetype/build
	@cp $(LIB_DIR)/freetype/build/*.so $(LIB_DIR)/
	@cp -r $(LIB_DIR)/freetype/include/* $(LIB_DIR)/include

	@mkdir -p $(LIB_DIR)/include/SDL3
	@cmake -S $(LIB_DIR)/sdl -B $(LIB_DIR)/sdl/build -DCMAKE_BUILD_TYPE=Release \
	       -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF -DSDL_DISABLE_INSTALL=ON
	@cmake --build $(LIB_DIR)/sdl/build
	@cp $(LIB_DIR)/sdl/include/SDL3/* $(LIB_DIR)/include/SDL3
	@cp $(LIB_DIR)/sdl/build/*.so* $(LIB_DIR)/

	@echo "Finished building libs"

.PHONY: restore
restore:
	@mkdir -p $(SHADER_DIR)/spirv/

	@git submodule update --init --remote --merge --recursive -j 8
	@make libs

	@cmake -S $(TOOL_DIR)/bear -B $(TOOL_DIR)/bear -DENABLE_UNIT_TESTS=OFF -DENABLE_FUNC_TESTS=OFF
	@make -C $(TOOL_DIR)/bear -j8
	@$(TOOL_DIR)/bear/stage/bin/bear                             \
		--bear-path   $(TOOL_DIR)/bear/stage/bin/bear            \
		--library     $(TOOL_DIR)/bear/stage/lib/bear/libexec.so \
		--wrapper     $(TOOL_DIR)/bear/stage/lib/bear/wrapper    \
		--wrapper-dir $(TOOL_DIR)/bear/stage/lib/bear/wrapper.d  \
		-- make -j8

	@cmake -S $(TOOL_DIR)/spright -B $(TOOL_DIR)/spright
	@make -C $(TOOL_DIR)/spright -j8

	@echo "Restore complete"

.PHONY: clean
clean:
	@rm -f $(OBJ)
	@echo "Cleanup complete"
	@rm -f $(DEP)
	@echo "Dependency files removed"
	@rm -f $(SHADER_DIR)/spirv/*
	@echo "Shader bytecode removed"

.PHONY: remove
remove:	clean
	@rm -f ./$(BIN) ./$(STLIB) ./$(SHLIB)
	@echo "Executables removed"
	@rm -rf $(LIB_DIR)/*
	@echo "Libraries removed"
	@rm -rf $(TOOL_DIR)/*
	@echo "Tools removed"

.PHONY: cloc
cloc:
	cloc --vcs=git
