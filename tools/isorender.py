import bpy
import re
from math import *
from mathutils import *

original_path = bpy.data.scenes[0].render.filepath

matcap_dir = "/home/charles/Pictures/matcaps/"
directions = ["sw", "s", "se", "e", "ne", "n", "nw", "w"]
matcaps    = [
    (""       , matcap_dir + "toon.exr"          , 1.0),
    ("-normal", matcap_dir + "check_normal+y.exr", 0.0),
]

matcap_tex = bpy.data.materials["matcap"].node_tree.nodes["matcap"].image
matcap_fac = bpy.data.materials["matcap"].node_tree.nodes["factor"].inputs[0]

models   = [model for model in bpy.data.collections["models"].objects      if model.type == "MESH"]
attmts   = [attmt for attmt in bpy.data.collections["attachments"].objects if attmt.type == "MESH"]
tiles    = [tile  for tile  in bpy.data.collections["tiles"].objects       if tile.type  == "MESH"]
target   = bpy.data.objects["floor"]
armature = bpy.data.objects["armature"]
light    = bpy.data.objects["light"]

anim_pattern = re.compile("^(.+)-(.+)$")
def render_obj(obj, animations, attmt_name):
    if not animations:
        animations = [None]
        
    for matcap in matcaps:
        matcap_tex.filepath      = matcap[1]
        matcap_fac.default_value = matcap[2]
        for action in animations:
            if action:
                armature.animation_data.action = action
                frame_range = (int(action.frame_range[0]),
                               int(action.frame_range[1]))
                bpy.context.scene.frame_start = frame_range[0]
                bpy.context.scene.frame_end   = frame_range[1]
                bpy.context.scene.frame_step  = ceil((frame_range[1] - frame_range[0]) / 10)

            step = 1 if action else 8
            for i in range(0, 8, step):
                target.rotation_euler[2] = i*2.0*pi / 8
                
                if attmt_name and action:
                    name = attmt_name + '-' + anim_pattern.match(action.name)[2]
                elif not action:
                    name = obj.name
                else:
                    name = action.name
                fmt = "%s%s-%s-##%s" if action else "%s%s-%s%s"
                bpy.data.scenes[0].render.filepath = fmt % (
                    original_path,
                    name,
                    directions[i],
                    matcap[0])
                bpy.ops.render.render(animation=bool(action), write_still=not action)
            target.rotation_euler[2] = 0.0

bpy.data.scenes[0].render.engine        = "BLENDER_EEVEE"
bpy.data.scenes[0].display.shading.type = "RENDERED"
for obj in bpy.data.objects:
    if obj != light:
        obj.hide_render = True

try:
    for model in models:
        model.hide_render = False
        animations = [action for action in bpy.data.actions if action.name.startswith(model.name)]
        render_obj(model, animations, model.name)
        
        render_with = []
        for attmt in attmts:
            if attmt.parent == model:
                render_with.append(attmt)
            else:
                for constraint in attmt.constraints:
                    if constraint.target == model:
                        render_with.append(attmt)
        
        model.is_holdout = True
        for attmt in render_with:
            attmt.hide_render = False
            render_obj(attmt, animations, attmt.name)
            attmt.hide_render = True
        model.is_holdout = False
        
        model.hide_render = True
    
    for tile in tiles:
        tile.hide_render = False
        render_obj(tile, [], tile.name)
        tile.hide_render = True
finally:
    bpy.data.scenes[0].render.filepath = original_path
