csage = require("ffi")
csage.cdef [[
	typedef  int8_t   int8;
	typedef  int16_t  int16;
	typedef  int32_t  int32;
	typedef  int64_t  int64;
	typedef uint8_t  uint8;
	typedef uint16_t uint16;
	typedef uint32_t uint32;
	typedef uint64_t uint64;
	typedef uint64_t ID;

	typedef unsigned int uint;
	typedef  intptr_t  intptr;
	typedef uintptr_t uintptr;
	typedef unsigned char byte;
	typedef  size_t usize;
	typedef ssize_t isize;

	enum Axis {
		AXIS_NONE,
		AXIS_X = 1 << 0,
		AXIS_Y = 1 << 1,
		AXIS_Z = 1 << 2,
	};
	enum Direction {
		DIR_NONE         = 1 << 0,
		DIR_UP           = 1 << 1,
		DIR_DOWN         = 1 << 2,
		DIR_RIGHT        = 1 << 4,
		DIR_LEFT         = 1 << 5,
		DIR_FORWARDS     = 1 << 6,
		DIR_BACKWARDS    = 1 << 7,
		DIR_ROTATE_LEFT  = 1 << 8,
		DIR_ROTATE_RIGHT = 1 << 9,
		DIR_N            = 1 << 10,
		DIR_S            = 1 << 11,
		DIR_E            = 1 << 12,
		DIR_W            = 1 << 13,
		DIR_NW           = DIR_N | DIR_W,
		DIR_NE           = DIR_N | DIR_E,
		DIR_SW           = DIR_S | DIR_W,
		DIR_SE           = DIR_S | DIR_E,
	};

	/* ---------------------------------------------------------------- */

	struct VArray {
		byte* data;
		int len;
		int cap;
		int elem_sz;
	};

	/* ---------------------------------------------------------------- */

	typedef struct Rect {
		float x, y, w, h;
	} Rect;

	typedef struct Recti {
		int16 x, y, w, h;
	} Recti;

	/* ---------------------------------------------------------------- */

	enum SpriteAnimation {
		SPRITE_NONE,
		SPRITE_WALK,
		SPRITE_RUN,
		SPRITE_ATTACK1,
		SPRITE_ATTACK2,
	};

	struct SpriteFrame {
		uint16 x, y, w, h;
	};

	struct SpriteState {
		enum SpriteAnimation type;
		enum Direction       dir;
		int framec;
		struct SpriteFrame* frames;
	};

	struct SpriteGroup {
		char name[32];
		int statec;
		struct SpriteState* states;
	};

	struct SpriteSheet {
		char name[32];
		int w, h;
		int groupc;
		struct VArray       sprites;
		struct SpriteGroup* groups;
		struct Texture*     tex;
	};
]]

direction_enum = {
	[""]             = csage.C.DIR_NONE,
	["up"]           = csage.C.DIR_UP,
	["down"]         = csage.C.DIR_DOWN,
	["right"]        = csage.C.DIR_RIGHT,
	["left"]         = csage.C.DIR_LEFT,
	["forwards"]     = csage.C.DIR_FORWARDS,
	["backwards"]    = csage.C.DIR_BACKWARDS,
	["rotate_left"]  = csage.C.DIR_ROTATE_LEFT,
	["rotate_right"] = csage.C.DIR_ROTATE_RIGHT,
	["n"]            = csage.C.DIR_N,
	["s"]            = csage.C.DIR_S,
	["e"]            = csage.C.DIR_E,
	["w"]            = csage.C.DIR_W,
	["nw"]           = csage.C.DIR_NW,
	["ne"]           = csage.C.DIR_NE,
	["sw"]           = csage.C.DIR_SW,
	["se"]           = csage.C.DIR_SE,
}

sprite_animation = {
	[""]        = csage.C.SPRITE_NONE,
	["walk"]    = csage.C.SPRITE_WALK,
	["run"]     = csage.C.SPRITE_RUN,
	["attack1"] = csage.C.SPRITE_ATTACK1,
	["attack2"] = csage.C.SPRITE_ATTACK2,
}

function table_len(t)
    local count = 0
    for _, __ in pairs(t) do
        count = count + 1
    end
    return count
end
