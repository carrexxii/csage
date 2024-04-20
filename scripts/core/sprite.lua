function load_sprite_sheet(fname)
	local sheet_data = dofile(fname)
	if not sheet_data then
		print("[LUA] Failed to load file: ", fname)
		return nil
	end

	local sheet = ffi.new("struct SpriteSheetCreateInfo")
	sheet.w     = sheet_data["w"]
	sheet.h     = sheet_data["h"]

	local framec
	local sprites = {}
	for i, state in pairs(sheet_data) do
		if tonumber(i) then
			local name, dir = string.match(state.id, "(.+[-].+)-(.+)")
			if sprites[name] == nil then
				framec = 0
				print("->", name)
				sprites[name] = {
					framec   = table_len(state.frames),
					duration = tonumber(sheet_data[name]),
					frames   = {},
				}
			end

			sprites[name].frames[sprite_dir_enum[dir]] = state.frames
			print(framec)
			framec = framec + 1
		end
	end

	local names     = {}
	local framecs   = {}
	local frames    = {}
	local durations = {}
	local spritec   = 0
	for name, sprite in pairs(sprites) do
		names[spritec]     = ffi.new("char[32]", name)
		framecs[spritec]   = sprite.framec
		durations[spritec] = sprite.duration or 0
		frames[spritec]    = ffi.new("SpriteFrame*[8]")
		for i, dir in pairs(sprite.frames) do
			frames[spritec][i] = ffi.new("SpriteFrame[?]", sprite.framec)
			for j, frame in pairs(dir) do
				frames[spritec][i][j] = frame
			end
		end
		spritec = spritec + 1
	end

	sheet.spritec   = spritec
	sheet.framecs   = ffi.new("int[?]"            , spritec, framecs)
	sheet.names     = ffi.new("char[?][32]"       , spritec, names)
	sheet.frames    = ffi.new("SpriteFrame*[?][8]", spritec, frames)
	sheet.durations = ffi.new("uint16[?]"         , spritec, durations)
	return sheet
end

