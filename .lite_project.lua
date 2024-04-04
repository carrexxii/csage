local config = require "core.config"

config.ignore_files = { "^%.",
	"tools",
	"lib",
	"build",
	"sfx",
	"shaders/spirv",
	"gfx/fonts",
	"gfx/models",
	"gfx/textures",
	"gfx/tilesets",
	"gfx/sprites/renders",
	"^csage$",
}

