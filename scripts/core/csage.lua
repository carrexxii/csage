csage = require("ffi")
csage.cdef [[
	typedef char VBO[24];
	typedef char UBO[24];
	typedef char SBO[24];
	typedef char Texture[24];
	typedef char Pipeline[176];

	/* ---------------------------------------------------------------- */

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
		DIR_NONE         = 0,
		DIR_UP           = 1 << 0,
		DIR_DOWN         = 1 << 1,
		DIR_RIGHT        = 1 << 2,
		DIR_LEFT         = 1 << 3,
		DIR_FORWARDS     = 1 << 4,
		DIR_BACKWARDS    = 1 << 5,
		DIR_ROTATE_LEFT  = 1 << 6,
		DIR_ROTATE_RIGHT = 1 << 7,
		DIR_N            = 1 << 8,
		DIR_S            = 1 << 9,
		DIR_E            = 1 << 10,
		DIR_W            = 1 << 11,
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

	typedef union Vec2 { struct { float x, y;       }; float arr[2]; } Vec2;
	typedef union Vec3 { struct { float x, y, z;    }; float arr[3]; } Vec3;
	typedef union Vec4 { struct { float x, y, z, w; }; float arr[4]; } Vec4;
	typedef union Vec2i { struct { int x, y;       }; int arr[2]; } Vec2i;
	typedef union Vec3i { struct { int x, y, z;    }; int arr[3]; } Vec3i;
	typedef union Vec4i { struct { int x, y, z, w; }; int arr[4]; } Vec4i;
	typedef union Mat4x4 {
		struct { Vec4 r1, r2, r3, r4; };
		struct { float m11, m12, m13, m14,
		               m21, m22, m23, m24,
		               m31, m32, m33, m34,
		               m41, m42, m43, m44; };
		float arr[16];
	} Mat4x4;

	/* ---------------------------------------------------------------- */

	struct DirectionalLight {
		Vec3 dir;      float pad1;
		Vec3 ambient;  float pad2;
		Vec3 diffuse;  float pad3;
		Vec3 specular; float pad4;
	};

	struct PointLight {
		Vec3 pos;     float pad1;
		Vec3 ambient; float pad2;
		Vec3 diffuse; float pad3;
		Vec3 specular;
		float constant;
		float linear;
		float quadratic;
		float pad4[2];
	};

	struct SpotLight {
		Vec3 pos;     float pad1;
		Vec3 dir;     float pad2;
		Vec3 ambient; float pad3;
		Vec3 diffuse; float pad4;
		Vec3 specular;
		float constant;
		float linear;
		float quadratic;
		float cutoff;
		float outer_cutoff;
	};

	/* ---------------------------------------------------------------- */

	enum SpriteStateType {
		SPRITE_NONE,
		SPRITE_IDLE,
		SPRITE_WALK,
		SPRITE_RUN,
		SPRITE_ATTACK1,
		SPRITE_ATTACK2,

		SPRITE_GRASS,
		SPRITE_DIRT,

		SPRITE_LAST,
	};

	struct SpriteFrame {
		uint16 x, y, w, h;
	};

	struct SpriteState {
		enum SpriteStateType type;
		enum Direction       dir;
		int duration;
		int gi;
		int framec;
		struct SpriteFrame* frames;
	};

	struct SpriteGroup {
		char name[32];
		int statec;
		struct SpriteState* states;
	};

	struct Sprite {
		Vec3   pos;
		int16  gi;
		int8   state, frame;
		int8   sheet, group;
		uint16 time;
		byte pad[12];
	};

	struct SpriteSheet {
		char name[32];
		int w, h, z;
		int groupc;
		bool needs_update;
		struct VArray       sprites;
		struct SpriteGroup* groups;

		Texture albedo;
		Texture normal;
		Pipeline pipeln;
		SBO sprite_sheet_data;
		SBO sprite_data;
	};

	int sprite_sheet_new(char* name);
	int sprite_sheet_load(struct SpriteSheet* sheet_data);

	/* ---------------------------------------------------------------- */

	typedef uint16 MapTile;

	struct MapLayer {
		char name[32];
		int x, y, w, h;
		struct Sprite** sprites;
		MapTile data[?];
	};

	struct Map {
		int w, h;
		int sprite_sheet;
		int layerc, spot_lightc, point_lightc;
		struct MapLayer*   layers[8];
		struct SpotLight*  spot_lights;
		struct PointLight* point_lights;

		UBO ubo;
		SBO spot_lights_sbo;
		SBO point_lights_sbo;
		Pipeline pipeln;
	};
]]

sprite_path = "./gfx/sprites/"

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

animation_enum = {
	[""]    = csage.C.SPRITE_NONE,
	idle    = csage.C.SPRITE_IDLE,
	walk    = csage.C.SPRITE_WALK,
	run     = csage.C.SPRITE_RUN,
	attack1 = csage.C.SPRITE_ATTACK1,
	attack2 = csage.C.SPRITE_ATTACK2,

	grass = csage.C.SPRITE_GRASS,
	dirt  = csage.C.SPRITE_DIRT,
}

function table_len(t)
    local count = 0
    for _, __ in pairs(t) do
        count = count + 1
    end
    return count
end

function vec3_of_colour(string)
	return {
		tonumber(string:sub(2, 3), 16) / 255.0,
		tonumber(string:sub(4, 5), 16) / 255.0,
		tonumber(string:sub(6, 7), 16) / 255.0,
	}
end
