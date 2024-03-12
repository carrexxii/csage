function load_sprite_sheet(fname)
	local sheet_data = dofile(fname)
	if not sheet_data then
		return nil
	end

	local name = string.match(fname, ".*/(.+).lua")
	local sheet = csage.new("struct SpriteSheet")
	sheet.name = csage.new("char[?]", #name, name)
	sheet.w    = sheet_data["w"]
	sheet.h    = sheet_data["h"]

	local sprites = {}
	for anim_name, anim in pairs(sheet_data) do
		name, anim_name = string.match(anim_name, "(.+)@(.+)")
		if name and anim_name then
			if sprites[name] == nil then
				sheet.spritec = sheet.spritec + 1
				sprites[name]      = csage.new("struct Sprite")
				sprites[name].name = csage.new("char[?]", #name, name)

				local statec = 0
				for anim_name, _ in pairs(sheet_data) do
					if name == string.match(anim_name, "(.+)@.+") then
						statec = statec + 1
					end
				end
				sprites[name].states = csage.new("struct SpriteState[?]", statec)
			end

			sprites[name].states[sprites[name].statec].type   = sprite_animation[anim_name] or csage.C.SPRITE_NONE
			sprites[name].states[sprites[name].statec].framec = #anim
			sprites[name].states[sprites[name].statec].frames = csage.new("struct SpriteFrame[?]", #anim)
			for i, frame in ipairs(anim) do
				sprites[name].states[sprites[name].statec].frames[i - 1] = csage.new("struct SpriteFrame", frame)
			end
			sprites[name].statec = sprites[name].statec + 1
		end
	end
	sprites_arr = {}
	for _, spr in pairs(sprites) do
		sprites_arr[#sprites_arr + 1] = spr
	end

	sheet.sprites = csage.new("struct Sprite[?]", #sprites_arr, sprites_arr)
	return csage.new("struct SpriteSheet", sheet)
end
