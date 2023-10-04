import bpy
import os

class Material:
    index  = None
    colour = None
    def __init__(self, index, colour):
        self.index  = index
        self.colour = colour

def write_file(path):
    context = bpy.context
    objs    = bpy.data.objects
    meshes  = bpy.data.meshes
    
    materials  = {}
    vert_count = 0
    for obj in objs:
        for mat in obj.material_slots:
            if not mat.name in materials:
                materials[mat.name] = Material(len(materials), mat.material.diffuse_color[:-1])
        vert_count += len(objs['Ship'].to_mesh().vertices)
    
    with open(path, "w", encoding="utf-8") as file:
        file.write("Vertices: %d; Materials: %d\n" % (vert_count, len(materials)))
        file.write("Dimensions: %.2f %.2f\n" % context.object.dimensions[:-1])
        for mat in materials:
            file.write("M %d \"%s\" " % (materials[mat].index, mat))
            file.write("[%.2f %.2f %.2f]" % materials[mat].colour)
            file.write("\n")

        for obj in objs:
            mesh = obj.data
            pos = obj.matrix_world
            mat_names = [m.name for m in mesh.materials]
            for tri in mesh.polygons:
                for i in tri.vertices:
                    v = pos @ mesh.vertices[i].undeformed_co.copy()
                    n = tri.normal.copy()
                    n[2] = -n[2]
                    file.write("%.4f %.4f " % v[:-1])
                    file.write("%d\n" % (materials[mat_names[tri.material_index]].index))

if __name__ == "__main__":
    print(os.path.basename(bpy.data.filepath))
    write_file("/home/charles/Projects/csage/data/models/" + os.path.basename(bpy.data.filepath).replace(".blend", ""))
