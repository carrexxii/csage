import os

output = os.path.join(os.path.dirname(__file__), "spright.conf")

directions = ["sw", "s", "se", "e", "ne", "n", "nw", "w"]
# shadings = ["albedo", "normal"]

sheet_tmpl = \
r"""
description "{0}.lua"
	template "lua.inja"

sheet "{0}"
	output "sheets/{0}.png"
		maps "" "-normal"
	pack compact
	power-of-two true
	square       true
	allow-rotate true
"""

sprite_tmpl = \
r"""
glob "renders/{0}-{1}-{2}-*.png"
	id "{0}@{1}-{2}"
	maps "" "-normal" """

sprites = {
	"player": [ "idle", "run", "torch" ],
}

with open(output, "w+") as file:
	for sprite in sprites:
		file.write(sheet_tmpl.format(sprite))
		for state in sprites[sprite]:
			for dir in directions:
				file.write(sprite_tmpl.format(sprite, state, dir))
			file.write("\n")
