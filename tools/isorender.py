import bpy
from math import *
from mathutils import *

directions = ["sw", "s", "se", "e", "ne", "n", "nw", "w"]
shadings   = [("albedo", "basic_1.exr", "TEXTURE"), ("normal", "check_normal+y.exr", "MATERIAL")]

target   = bpy.data.objects["floor"]
armature = bpy.data.objects["armature"]

bpy.data.scenes[0].render.engine = "BLENDER_WORKBENCH"
bpy.data.scenes[0].display.shading.type  = "SOLID"
bpy.data.scenes[0].display.shading.light = "MATCAP"

anglec = 8
original_path = bpy.data.scenes[0].render.filepath
for obj in target.children:
    if obj.type != "MESH":
        continue

    obj.hide_render = False
    for shading in shadings:
        bpy.data.scenes[0].display.shading.studio_light = shading[1]
        bpy.data.scenes[0].display.shading.color_type   = shading[2]
        for action in bpy.data.actions:
            if not action.name.startswith(obj.name):
                continue
            armature.animation_data.action = action

            frame_range = (int(action.frame_range[0]),
                           int(action.frame_range[1]))
            bpy.context.scene.frame_start = frame_range[0]
            bpy.context.scene.frame_end   = frame_range[1]
            bpy.context.scene.frame_step  = ceil((frame_range[1] - frame_range[0]) / 10)
            for i in range(anglec):
                target.rotation_euler[2] = i*2.0*pi / anglec
                
                bpy.data.scenes[0].render.filepath = "%s%s-%s-%s-##" % (
                    original_path,
                    action.name,
                    shading[0],
                    directions[i])
                bpy.ops.render.render(animation=True)
            target.rotation_euler[2] = 0.0

    obj.hide_render = True

bpy.data.scenes[0].render.filepath = original_path
