import bpy
from math import *
from mathutils import *

directions = ["sw", "s", "se", "e", "ne", "n", "nw", "w"]

target = bpy.data.objects["floor"]
cam = bpy.data.objects["Camera"]
t_loc_x = target.location.x
t_loc_y = target.location.y
cam_loc_x = cam.location.x
cam_loc_y = cam.location.y

anglec = 8
original_path = bpy.data.scenes[0].render.filepath
for obj in target.children:
    if obj.type != "MESH":
        continue
    
    obj.hide_render = False
    for i in range(anglec):
        target.rotation_euler[2] = i*2.0*pi / anglec
        
        bpy.data.scenes[0].render.filepath = "%s%s-%s-%s##" % (
            original_path,
            obj.users_collection[0].name,
            obj.name,
            directions[i])
        bpy.ops.render.render(animation=True) 

    obj.hide_render = True
    target.rotation_euler[2] = 0.0

bpy.data.scenes[0].render.filepath = original_path
