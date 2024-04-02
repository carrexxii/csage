function load_level()
	local group = ffi.C.entity_new_group("player")
	local poss = ffi.new("Vec2[10000]")
	for y = 0, 99 do
		for x = 0, 99 do
			poss[y*100 + x] = vec2(x / 1.05, y / 1.5)
		end
	end
	ffi.C.entity_new_batch(group, 10000, poss)

	return ffi.new("struct Level", {
		name = to_c_str("The level's name"),
	})
end
