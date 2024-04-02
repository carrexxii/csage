function load_level()
	name = to_c_str("The level's name")

	local sheet = load_sprite_sheet("gfx/sprites/player.lua")
	for i = 1, 100 do
		ffi.C.entity_new(sheet, vec2(i, 0))
	end

	return ffi.new("struct Level", {
		name = name,
	})
end
