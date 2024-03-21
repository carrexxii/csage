function load_sprite_sheet(fname)
	local sheet_data = dofile(fname)
	if not sheet_data then
		print("[LUA] Failed to load file: ", fname)
		return nil
	end

	local sheet = csage.new("struct SpriteSheet")
	sheet.name  = string.match(fname, ".*/(.+).lua")
	sheet.w     = sheet_data["w"]
	sheet.h     = sheet_data["h"]

	local gi = 0
	local groups = {}
	for state_name, anim in pairs(sheet_data) do
		local name, anim_name, dir = string.match(state_name, "(.+)@(.+)-(.+)")
		if dir and anim_name and name then
			if groups[name] == nil then
				groups[name] = {
					name   = name,
					statec = 0,
					states = {},
				}
			end

			groups[name].states[groups[name].statec] = {
				duration = tonumber(sheet_data[name .. "@" .. anim_name]),
				framec   = #anim,
				frames   = csage.new("struct SpriteFrame[?]", #anim, anim),
				type     = animation_enum[anim_name] or direction_enum[""],
				dir      = direction_enum[dir],
				gi       = gi,
			}
			groups[name].statec = groups[name].statec + 1
			gi = gi + #anim
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
