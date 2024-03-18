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
		local name, anim_name, dir = string.match(anim_name, "(.+)@(.+)-(.+)")
		if dir and anim_name and name then
			if groups[name] == nil then
				groups[name]      = {}
				groups[name].name = name

				local statec = 0
				for anim_name, _ in pairs(sheet_data) do
					if name == string.match(anim_name, "(.+)@.+") then
						statec = statec + 1
					end
				end
				groups[name].states = {}
				groups[name].statec = 0
			end

			local state = {
				duration = tonumber(sheet_data[name .. "@" .. anim_name]),
				framec   = #anim,
				frames   = csage.new("struct SpriteFrame[?]", #anim, anim),
				type     = animation_enum[anim_name] or csage.C.SPRITE_NONE,
				dir      = direction_enum[dir],
			}

			groups[name].states[groups[name].statec] = state
			groups[name].statec = groups[name].statec + 1
		end
	end

	sheet.groupc = table_len(groups)
	sheet.groups = csage.new("struct SpriteGroup[?]", sheet.groupc, groups)
	local i = 0
	for _, group in pairs(groups) do
		sheet.groups[i].name   = group.name
		sheet.groups[i].statec = group.statec
		sheet.groups[i].states = csage.new("struct SpriteState[?]", sheet.groups[i].statec, group.states)
		i = i + 1
	end

	return sheet
end
