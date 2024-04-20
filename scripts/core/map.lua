function load_tiled_map(fname)
	local map_data = dofile(fname)
	if not map_data then
		print("[LUA] Failed to load map file: ", fname)
		return nil
	end

	if map_data.orientation ~= "isometric" then
		print("[LUA] Map is not isometric, have: ", map_data.orientation)
		return nil
	end

	local map = {
		w = map_data.width,
		h = map_data.height,
		z = 0,
		layerc = 0,
		layers = {},
		spot_lightc  = 0,
		point_lightc = 0,
	}

	-- local sheet
	-- for _, tset in pairs(map_data.tilesets) do
	-- 	if map.sprite_sheet then
	-- 		print("[LUA] Multiple sprite sheets for: ", fname, "(Only the last will be used)")
	-- 	end

	-- 	sheet = load_sprite_sheet(sprite_path .. tset.name .. ".lua")
	-- 	if not sheet then
	-- 		print("[LUA] No spritesheet associated with map: ", fname)
	-- 		return nil
	-- 	end
	-- end

	for _, layer in pairs(map_data.layers) do
		if layer.type == "tilelayer" then
			-- local data = {}
			-- for i, d in pairs(layer.data) do
			-- 	if not d then
			-- 		data[i - 1] = ffi.C.SPRITE_NONE
			-- 	else
			-- 		local x = math.floor(d % (256 / 80))
			-- 		local y = math.floor(d / (256 / 64))
			-- 		for j = 0, sheet.groupc - 1 do
			-- 			local group = sheet.groups[j]
			-- 			for k = 0, group.statec - 1 do
			-- 				local state = group.states[k]
			-- 				local fx = math.floor(state.frames[0].x / 80)
			-- 				local fy = math.floor(state.frames[0].y / 64)
			-- 				if x == fx and y == fy then
			-- 					data[i - 1] = state.type
			-- 				end
			-- 			end
			-- 		end
			-- 	end
			-- end

			map.layers[map.layerc] = ffi.new("struct MapLayer", #layer.data, {
				name = layer.name,
				x    = tonumber(layer.x),
				y    = tonumber(layer.y),
				w    = tonumber(layer.width),
				h    = tonumber(layer.height),
				data = layer.data,
			})
			map.layerc = map.layerc + 1
		elseif layer.type == "objectgroup" then
			if layer.name == "lights" then
				local spot_lights  = {}
				local point_lights = {}
				for _, obj in pairs(layer.objects) do
					if obj.shape == "ellipse" then
						spot_lights[map.spot_lightc] = {
							pos = { 2.0 * obj.x / map_data.tilewidth + 1, obj.y / map_data.tileheight + 1, 5.0 },
							dir = { 0.0, 0.0, -1.0 },
							-- TODO: move this to a config file
							ambient      = vec3_of_colour(obj.properties["ambient"]  or "#FFFFFF"),
							diffuse      = vec3_of_colour(obj.properties["diffuse"]  or "#FFFFFF"),
							specular     = vec3_of_colour(obj.properties["specular"] or "#FFFFFF"),
							constant     = tonumber(obj.properties["constant"])    or 1.0,
							linear       = tonumber(obj.properties["linear"])      or 0.07,
							quadratic    = tonumber(obj.properties["constant"])    or 0.017,
							cutoff       = tonumber(obj.properties["cutoff"])      or 1.0,
							outer_cutoff = tonumber(obj.properties["outercutofa"]) or 0.8,
						}
						map.spot_lightc = map.spot_lightc + 1
					elseif obj.shape == "point" then
						point_lights[map.point_lightc] = {
							pos = { 2.0 * obj.x / map_data.tilewidth + 1, obj.y / map_data.tileheight + 1, 3.0 },
							-- TODO: move this to a config file
							ambient   = vec3_of_colour(obj.properties["ambient"]  or "#FFFFFF"),
							diffuse   = vec3_of_colour(obj.properties["diffuse"]  or "#FFFFFF"),
							specular  = vec3_of_colour(obj.properties["specular"] or "#FFFFFF"),
							constant  = tonumber(obj.properties["constant"]) or 1.0,
							linear    = tonumber(obj.properties["linear"])   or 0.22,
							quadratic = tonumber(obj.properties["constant"]) or 0.20,
						}
						map.point_lightc = map.point_lightc + 1
					end
				end
				map.spot_lights  = ffi.new("struct SpotLight[?]" , map.spot_lightc , spot_lights)
				map.point_lights = ffi.new("struct PointLight[?]", map.point_lightc, point_lights)
			else
				print("[LUA] Ignoring object group: ", layer.name)
			end
		else
			print("[LUA] Ignoring layer of type: ", layer.type)
		end
	end

	-- map.sprite_sheet = ffi.C.sprite_sheet_load(sheet)
	return ffi.new("struct Map", map);
end

