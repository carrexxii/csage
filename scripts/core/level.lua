function load_level()
	local group  = ffi.C.entity_new_group("player")
	local bodies = ffi.new("struct Body[100]")
	for y = 0, 2 do
		for x = 0, 5 do
			bodies[y*5 + x] = { pos = vec2(x / 1.5, y / 1.5) }
		end
	end
	ffi.C.entity_new_batch(group, 10, bodies)

	return ffi.new("struct Level", {
		name = to_c_str("The level's name"),
	})
end
