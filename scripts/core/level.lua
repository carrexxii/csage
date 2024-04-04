function load_level()
	local group = ffi.C.entity_new_group("player", "ENTITY_GROUP_AI")
	local ecis  = ffi.new("struct EntityCreateInfo[100]")
	for y = 0, 2 do
		for x = 0, 5 do
			ecis[y*5 + x] = {
				pos   = vec2(x / 1.5, y / 1.5),
				speed = 0.2,
				type  = "AI_TYPE_FRIENDLY"
			}
		end
	end
	local fst_entity = ffi.C.entity_new_batch(group, 10, ecis)

	ai_set_follow(group, fst_entity, vec2(10, 10))
	ai_set_patrol(group, fst_entity + 1, {vec2(5, 5), vec2(10, 10)})

	return ffi.new("struct Level", {
		name = to_c_str("The level's name"),
	})
end
