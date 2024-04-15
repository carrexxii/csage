os = require("os")
io = require("io")

local cwd = debug.getinfo(1).source:match("@?(.*/)")

local directions = { "sw", "se", "ne", "nw", "s", "e", "n", "w" }

local sprites = {
	player = {
		type = "animated",
		directions = 8,
		states = {
			idle = 250,
			run  = 100,
		},
	},
	tiles = {
		type = "static",
		directions = 1,
		states = {
			grass = 0,
			dirt  = 1,
		},
	}
}

local sheet_tmpl = [[
debug
description "%s.lua"
	template "lua.inja"

sheet "%s"
	output "sheets/%s.png"
		maps "" "-normal"
	power-of-two true
	square       true
	allow-rotate true
]]

local tmpl_animated = [[
glob "renders/%s-%s-%s-*.png"
	id "%s-%s-%s"
	maps "" "-normal"
	crop           true
	crop-pivot     true
	trim           rect
	trim-threshold 50
]]

local tmpl_static = [[
glob "renders/%s-%s.png"
	id "%s-%s-%s"
	maps "" "-normal"
	crop true
	min-bounds 64 64
]]

for sprite_name, sprite in pairs(sprites) do
	local file = io.open(cwd .. sprite_name .. ".conf", "w+")
	assert(file)

	file:write(sheet_tmpl:format(sprite_name, sprite_name, sprite_name))
	if sprite.type == "animated" then
		for state, time in pairs(sprite.states) do
			file:write(("\ntag \"%s-%s\" %d"):format(sprite_name, state, time))
		end
	end
	file:write("\n")
	for state, _ in pairs(sprite.states) do
		file:write("\n")
		for i = 1, sprite.directions do
			if sprite.type == "static" then
				file:write(tmpl_static:format(state, directions[i], sprite_name, state, directions[i]))
			elseif sprite.type == "animated" then
				file:write(tmpl_animated:format(sprite_name, state, directions[i], sprite_name, state, directions[i]))
			else
				print("Unhandled sprite type: ", sprite.type)
			end
		end
	end

	io.close(file)
end

