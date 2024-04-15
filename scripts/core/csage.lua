ffi = require("ffi")
ffi.cdef [[
	typedef char Arena[32];
	typedef char String[16];
	typedef char VArray[32];
	typedef char HPair[32];
	typedef char HTable[48];

	typedef char VBO[24];
	typedef char UBO[24];
	typedef char SBO[24];
	typedef char Image[24];
	typedef char Pipeline[96];

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

	typedef enum AxisType {
		AXIS_NONE,
		AXIS_X = 1 << 0,
		AXIS_Y = 1 << 1,
		AXIS_Z = 1 << 2,
	} AxisType;
	typedef enum DirectionMask {
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
	} DirectionMask;

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

	typedef struct DirectionalLight {
		Vec3 dir;      byte pad1[4];
		Vec3 ambient;  byte pad2[4];
		Vec3 diffuse;  byte pad3[4];
		Vec3 specular; byte pad4[4];
	} DirectionalLight;

	typedef struct PointLight {
		Vec3 pos;     byte pad1[4];
		Vec3 ambient; byte pad2[4];
		Vec3 diffuse; byte pad3[4];
		Vec3 specular;
		float constant;
		float linear;
		float quadratic;
		byte pad4[8];
	} PointLight;

	typedef struct SpotLight {
		Vec3 pos;     byte pad1[4];
		Vec3 dir;     byte pad2[4];
		Vec3 ambient; byte pad3[4];
		Vec3 diffuse; byte pad4[4];
		Vec3 specular;
		float constant;
		float linear;
		float quadratic;
		float cutoff;
		float outer_cutoff;
	} SpotLight;

	/* ---------------------------------------------------------------- */

	typedef enum SpriteFrameType {
		SPRITE_FRAME_E  = 0,
		SPRITE_FRAME_NE = 1,
		SPRITE_FRAME_N  = 2,
		SPRITE_FRAME_NW = 3,
		SPRITE_FRAME_W  = 4,
		SPRITE_FRAME_SW = 5,
		SPRITE_FRAME_S  = 6,
		SPRITE_FRAME_SE = 7,
		SPRITE_FRAME_NONE,
	} SpriteFrameType;

	typedef struct SpriteFrame {
		uint16 x, y, w, h;
	} SpriteFrame;

	typedef struct SpriteGroup {
		int         duration;
		int         framec;
		SpriteFrame frames[];
	} SpriteGroup;

	typedef struct Sprite {
		Vec2  pos;
		uint16 i, framec;
		uint16 timer, duration;
	} Sprite;

	typedef struct SpriteSheet {
		int16 w, h, z;
		HTable groups;
		VArray sprites;

		Pipeline* pipeln;
		Image*    albedo;
		Image*    normal;
		SBO       sprite_sheet_data;
		SBO       sprite_data;
	} SpriteSheet;

	typedef struct SpriteSheetCreateInfo {
		int w, h;
		int spritec;
		char (* names)[32];
		int*         framecs;
		SpriteFrame* (* frames)[8];
		int16*       durations;
	} SpriteSheetCreateInfo;

	/* ---------------------------------------------------------------- */

	typedef uint16 MapTile;

	typedef struct MapLayer {
		char name[32];
		int x, y, w, h;
		Sprite** sprites;
		MapTile data[?];
	} MapLayer;

	typedef struct Map {
		int w, h;
		SpriteSheet* sprite_sheet;
		int layerc, spot_lightc, point_lightc;
		MapLayer*   layers[8];
		SpotLight*  spot_lights;
		PointLight* point_lights;

		SBO spot_lights_sbo;
		SBO point_lights_sbo;
		Pipeline pipeln;
	} Map;

	/* ---------------------------------------------------------------- */

	typedef struct Level {
		String name;
	} Level;

	/* ---------------------------------------------------------------- */

	typedef struct Body {
		Vec2  pos;
		float speed;
		Vec2  vel;
		uint  dir_mask;
		Vec2  dir;
	} Body;

	/* ---------------------------------------------------------------- */

	typedef enum AIType {
		AI_TYPE_NONE,
		AI_TYPE_CONTROLLABLE,
		AI_TYPE_FRIENDLY,
		AI_TYPE_NEUTRAL,
		AI_TYPE_ENEMY,
	} AIType;

	typedef enum AIStateType {
		AI_STATE_IDLE,
		AI_STATE_FOLLOW,
		AI_STATE_PATROL,
		AI_STATE_WANDER,
		AI_STATE_PATHING,
		AI_STATE_MAX,
	} AIStateType;
	typedef struct AIState {
		AIStateType type;
		union {
			struct {
				Vec2* target;
				float dist;
			} follow;
			struct {
				uint8  pointc, i;
				uint16 delay, timer;
				Vec2*  points;
			} patrol;
		};
	} AIState;

	/* ---------------------------------------------------------------- */

	typedef enum EntityGroupMask {
		ENTITY_GROUP_NONE = 0,
		ENTITY_GROUP_AI   = 1 << 0,
	} EntityGroupMask;

	typedef uint GroupID;
	typedef uint EntityID;

	typedef struct EntityCreateInfo {
		Vec2   pos;
		float  speed;
		int    sprite_group;
		AIType ai_type;
	} EntityCreateInfo;

	GroupID entity_new_group(const char* sprite_sheet, EntityGroupMask mask);

	EntityID entity_new(GroupID gid, EntityCreateInfo* ci);
	isize    entity_new_batch(GroupID gid, isize entityc, EntityCreateInfo* cis);

	void entity_set_dir(EntityID eid, GroupID gid, DirectionMask dir, bool set);
	void entity_set_ai_state(EntityID eid, GroupID gid, AIState state);

	Body* entity_get_body(GroupID gid, EntityID eid);

	/* ---------------------------------------------------------------- */

	GroupID  player_group;
	EntityID player_entity;
]]

sprite_path = "./gfx/sprites/"

direction_enum = {
	[""]             = ffi.C.DIR_NONE,
	["up"]           = ffi.C.DIR_UP,
	["down"]         = ffi.C.DIR_DOWN,
	["right"]        = ffi.C.DIR_RIGHT,
	["left"]         = ffi.C.DIR_LEFT,
	["forwards"]     = ffi.C.DIR_FORWARDS,
	["backwards"]    = ffi.C.DIR_BACKWARDS,
	["rotate_left"]  = ffi.C.DIR_ROTATE_LEFT,
	["rotate_right"] = ffi.C.DIR_ROTATE_RIGHT,
	["n"]            = ffi.C.DIR_N,
	["s"]            = ffi.C.DIR_S,
	["e"]            = ffi.C.DIR_E,
	["w"]            = ffi.C.DIR_W,
	["nw"]           = ffi.C.DIR_NW,
	["ne"]           = ffi.C.DIR_NE,
	["sw"]           = ffi.C.DIR_SW,
	["se"]           = ffi.C.DIR_SE,
}

function vec2(x, y)
	return ffi.new("Vec2", { x = x, y = y })
end

function to_c_str(str)
	local cstr = ffi.new("char[?]", #str + 1, str)
	return ffi.new("String", {
		cap  = #str + 1,
		len  = #str,
		data = cstr,
	})
end

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

function get_player_pos()
	local body = ffi.C.get_entity_body(player_group, player_entity)
	return body.pos
end

function ai_set_follow(group, entity, target, dist)
	ffi.C.entity_set_ai_state(group, entity, {
		type = "AI_STATE_FOLLOW",
		follow = {
			target = target,
			dist   = dist or 1.0,
		},
	});
end

function ai_set_patrol(group, entity, points, delay)
	local pointc = #points
	local points = ffi.new("Vec2[?]", pointc, points)
	ffi.C.entity_set_ai_state(group, entity, {
		type = "AI_STATE_PATROL",
		patrol = {
			pointc = pointc,
			points = points,
			delay  = delay or 500,
		},
	})
end

