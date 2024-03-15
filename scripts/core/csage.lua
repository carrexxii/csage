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
		uint16 duration;
	};

	struct SpriteState {
		enum SpriteAnimation type;
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
