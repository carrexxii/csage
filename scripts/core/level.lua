function load_level()
	name = to_c_str("The level's name")

	local group = ffi.C.entity_new_group("player")
	for i = 1, 100 do
		ffi.C.entity_new(group, vec2(i, 0))
	end

	return ffi.new("struct Level", {
		name = name,
	})
end
