function load_sprite_sheet(fname)
	local sheet_data = dofile(fname)
	if not sheet_data then
		return nil
	end

	local name  = string.match(fname, ".*/(.+).lua")
	local sheet = csage.new("struct SpriteSheet")
	sheet.name  = name
	sheet.w     = sheet_data["w"]
	sheet.h     = sheet_data["h"]

	local groups = {}
	for anim_name, anim in pairs(sheet_data) do
		name, anim_name = string.match(anim_name, "(.+)@(.+)")
		if name and anim_name then
			if groups[name] == nil then
				sheet.groupc      = sheet.groupc + 1
				groups[name]      = csage.new("struct SpriteGroup")
				groups[name].name = name

				local statec = 0
				for anim_name, _ in pairs(sheet_data) do
					if name == string.match(anim_name, "(.+)@.*") then
						statec = statec + 1
					end
				end
				groups[name].states = csage.new("struct SpriteState[?]", statec)
			end

			groups[name].states[groups[name].statec].type   = sprite_animation[anim_name] or csage.C.SPRITE_NONE
			groups[name].states[groups[name].statec].framec = #anim
			groups[name].states[groups[name].statec].frames = csage.new("struct SpriteFrame[?]", #anim)
			for i, frame in ipairs(anim) do
				groups[name].states[groups[name].statec].frames[i - 1] = csage.new("struct SpriteFrame", frame)
			end
			groups[name].statec = groups[name].statec + 1
		end
	end
	groups_arr = {}
	for _, spr in pairs(groups) do
		groups_arr[#groups_arr + 1] = spr
	end

	sheet.groups = csage.new("struct SpriteGroup[?]", #groups_arr, groups_arr)
	return sheet
end
