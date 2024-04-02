function load_level()
	local group = ffi.C.entity_new_group("player")
	local poss = ffi.new("Vec2[1000000]")
	for y = 0, 999 do
		for x = 0, 999 do
			poss[y*1000 + x] = vec2(x / 1.5, y / 1.5)
		end
	end
	ffi.C.entity_new_batch(group, 1000000, poss)

	return ffi.new("struct Level", {
		name = to_c_str("The level's name"),
	})
end
