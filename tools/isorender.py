import bpy
from math import *
from mathutils import *

directions = ["sw", "s", "se", "e", "ne", "n", "nw", "w"]

target = bpy.data.objects["floor"]

anglec = 8
original_path = bpy.data.scenes[0].render.filepath
for obj in target.children:
    if obj.type != "MESH":
        continue

    obj.hide_render = False    
    for action in bpy.data.actions:
        if not action.name.startswith(obj.name):
            continue

        frame_range = (int(action.frame_range[0]),
                       int(action.frame_range[1]))
        bpy.context.scene.frame_start = frame_range[0]
        bpy.context.scene.frame_end   = frame_range[1]
        bpy.context.scene.frame_step  = ceil((frame_range[1] - frame_range[0]) / 10)
        for i in range(anglec):
            target.rotation_euler[2] = i*2.0*pi / anglec
            
            bpy.data.scenes[0].render.filepath = "%s%s-%s-##" % (
                original_path,
                action.name,
                directions[i])
            bpy.ops.render.render(animation=True)
        target.rotation_euler[2] = 0.0
    obj.hide_render = True

bpy.data.scenes[0].render.filepath = original_path
