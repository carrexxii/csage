function load_sprite(fname)
	local sprite = dofile(fname)
	if not sprite then
		return nil
	end

	animcs = {}
	anims  = {}
	-- names  = {}
	for animName, anim in pairs(sprite) do
		for _, frame in ipairs(anim) do
			anims[#anims + 1] = csage.new("Recti", frame)
		end
		animcs[#animcs + 1] = #anim
	end
	name = string.match(fname, ".*/(.+).lua")

	return csage.C.sprite_load(csage.new("char[?]", #name, name),
	                           #animcs,
	                           csage.new("isize[?]", #animcs, animcs),
	                           csage.new("Recti[?]", #anims, anims))
end
