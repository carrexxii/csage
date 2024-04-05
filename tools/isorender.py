import bpy
from math import *
from mathutils import *

matcap_dir = "/home/charles/Pictures/matcaps/"
directions = ["sw", "s", "se", "e", "ne", "n", "nw", "w"]
matcaps    = [
    (""       , matcap_dir + "toon.exr"          , 1.0),
    ("-normal", matcap_dir + "check_normal+y.exr", 0.0),
]

matcap_tex = bpy.data.materials["matcap"].node_tree.nodes["matcap"].image
matcap_fac = bpy.data.materials["matcap"].node_tree.nodes["factor"].inputs[0]

target   = bpy.data.objects["floor"]
armature = bpy.data.objects["armature"]
light    = bpy.data.objects["light"]

bpy.data.scenes[0].render.engine        = "BLENDER_EEVEE"
bpy.data.scenes[0].display.shading.type = "RENDERED"

for obj in bpy.data.objects:
    obj.hide_render = True

original_path = bpy.data.scenes[0].render.filepath
try:
    for obj in bpy.data.objects:
        if obj.type != "MESH":
            continue

        for attached in bpy.data.objects:
            if attached.name.startswith(obj.name) or attached == target or attached == light:
                attached.hide_render = False
            else:
                attached.hide_render = True
        for matcap in matcaps:
            matcap_tex.filepath      = matcap[1]
            matcap_fac.default_value = matcap[2]
            for action in bpy.data.actions:
                should_animate = not obj in list(bpy.data.collections["tiles"].all_objects)
                if should_animate:
                    if not action.name.startswith(obj.name):
                        continue
                    armature.animation_data.action = action

                    frame_range = (int(action.frame_range[0]),
                                   int(action.frame_range[1]))
                    bpy.context.scene.frame_start = frame_range[0]
                    bpy.context.scene.frame_end   = frame_range[1]
                    bpy.context.scene.frame_step  = ceil((frame_range[1] - frame_range[0]) / 10)

                step = 1 if should_animate else 8
                for i in range(0, 8, step):
                    target.rotation_euler[2] = i*2.0*pi / 8
                    fmt = "%s%s-%s-##%s" if should_animate else "%s%s-%s%s"
                    bpy.data.scenes[0].render.filepath = fmt % (
                        original_path,
                        action.name if should_animate else obj.name,
                        directions[i],
                        matcap[0])
                    bpy.ops.render.render(animation=should_animate, write_still=not should_animate)
                target.rotation_euler[2] = 0.0
finally:
    bpy.data.scenes[0].render.filepath = original_path
