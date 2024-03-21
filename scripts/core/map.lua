function load_tiled_map(fname)
	local map_data = dofile(fname)
	if not map_data then
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

	for _, tset in pairs(map_data.tilesets) do
		if map.sprite_sheet then
			print("[LUA] Multiple sprite sheets for: ", fname, "(Only the last will be used)")
		end
		local sheet = load_sprite_sheet(sprite_path .. tset.name .. ".lua")
		map.sprite_sheet = csage.C.sprite_sheet_load(sheet)
	end

	for _, layer in pairs(map_data.layers) do
		if layer.type == "tilelayer" then
			map.layers[map.layerc] = csage.new("struct MapLayer", #layer.data, {
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
							pos = { obj.x, obj.y, -1.0 },
							dir = { 0.0, 0.0, 1.0 },
							-- TODO: move this to a config file
							ambient   = vec3_of_colour(obj.properties["ambient"]  or "#FFFFFF"),
							diffuse   = vec3_of_colour(obj.properties["diffuse"]  or "#FFFFFF"),
							specular  = vec3_of_colour(obj.properties["specular"] or "#FFFFFF"),
							constant  = tonumber(obj.properties["constant"]) or 1.0,
							linear    = tonumber(obj.properties["linear"])   or 0.5,
							quadratic = tonumber(obj.properties["constant"]) or 3.5,
						}
						map.spot_lightc = map.spot_lightc + 1
					elseif obj.shape == "point" then
						point_lights[map.point_lightc] = {
							-- pos = { obj.x, obj.y, 0.0 },
							-- pos = { -obj.x / map_data.tilewidth, -obj.y / map_data.tileheight, 0.0 },
							pos = { 0, 0, 0 },
							-- TODO: move this to a config file
							ambient   = vec3_of_colour(obj.properties["ambient"]  or "#FFFFFF"),
							diffuse   = vec3_of_colour(obj.properties["diffuse"]  or "#FFFFFF"),
							specular  = vec3_of_colour(obj.properties["specular"] or "#FFFFFF"),
							constant  = tonumber(obj.properties["constant"]) or 1.0,
							linear    = tonumber(obj.properties["linear"])   or 0.5,
							quadratic = tonumber(obj.properties["constant"]) or 3.5,
						}
						map.point_lightc = map.point_lightc + 1
					end
				end
				map.spot_lights  = csage.new("struct SpotLight[?]" , #spot_lights , spot_lights)
				map.point_lights = csage.new("struct PointLight[?]", #point_lights, point_lights)
			else
				print("[LUA] Ignoring object group: ", layer.name)
			end
		else
			print("[LUA] Ignoring layer of type: ", layer.type)
		end
	end

	return csage.new("struct Map", map);
end
