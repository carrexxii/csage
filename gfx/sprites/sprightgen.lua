os = require("os")
io = require("io")

output = "spright.conf"

local cwd  = debug.getinfo(1).source:match("@?(.*/)")
local file = io.open(cwd .. output, "w+")
assert(file)

local directions = { "sw", "s", "se", "e", "ne", "n", "nw", "w" }

local sprites = {
	player = { idle = 250, run = 100, torch = 1000 },
}

local sheet_tmpl = [[
debug
description "%s.lua"
	template "lua.inja"

sheet "%s"
	output "sheets/%s.png"
		maps "" "-normal"
	pack         compact
	padding      1
	power-of-two true
	square       true
	allow-rotate true
]]

local sprite_tmpl = [[
glob "renders/%s-%s-%s-*.png"
	id "%s@%s-%s"
	maps "" "-normal"
	crop-pivot true
]]

for sprite, states in pairs(sprites) do
	file:write(sheet_tmpl:format(sprite, sprite, sprite))
	for state, time in pairs(states) do
		file:write(("\ntag \"%s@%s\" %d"):format(sprite, state, time))
	end
	file:write("\n")
	for state, _ in pairs(states) do
		file:write("\n")
		for _, dir in ipairs(directions) do
			file:write(sprite_tmpl:format(sprite, state, dir, sprite, state, dir))
		end
	end
end

io.close(file)
