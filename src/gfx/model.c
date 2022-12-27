#include "util/file.h"
#include "gfx/buffers.h"
#include "gfx/vertices.h"
#include "model.h"

#define LINE_BUFFER_SIZE 256

static void print_model(struct Model mdl);

struct Model create_model(char* const path)
{
	struct Model mdl = { 0 };

	FILE* file = file_open(path, "rb");
	char line[LINE_BUFFER_SIZE];

	char   name[65];
	uint8  index = 0;
	float* vert  = NULL;
	vec3   rgb;
	struct Material* materials = NULL;
	while (fgets(line, LINE_BUFFER_SIZE, file)) {
		/* File header */
		if (sscanf(line, "Vertices: %hu; Materials: %hhu", &mdl.vertc, &mdl.materialc) >= 2) {
			mdl.verts     = scalloc(mdl.vertc, SIZEOF_MDL_VERT);
			mdl.materials = scalloc(mdl.materialc, sizeof(struct Material));
			vert      = mdl.verts;
			materials = mdl.materials;
		/* Mesh header */
		} else if (sscanf(line, "Mesh: %64[a-zA-Z_] %hu", name, &mdl.vertc) >= 2) {
			if (mdl.verts)
				ERROR("Not ready for multiple meshes");
		/* Material header */
		} else if (sscanf(line, "Material %hhu \"%64[a-zA-Z_]\" [%f %f %f]",
		                  &index, name, &rgb.r, &rgb.g, &rgb.b) >= 5) {
			materials[index].rgb.r = rgb.r;
			materials[index].rgb.g = rgb.g;
			materials[index].rgb.b = rgb.b;
		/* Mesh vertices */
		} else {
			if (sscanf(line, "%f %f %f %f %f %f %hhu", vert    , vert + 1, vert + 2,
			                                           vert + 6, vert + 7, vert + 8, &index) >= 7) {
				vert[3] = (float)materials[index].rgb.r;
				vert[4] = (float)materials[index].rgb.g;
				vert[5] = (float)materials[index].rgb.b;
				vert += 9;
			}
		}
	}
	fclose(file);
	// print_model(mdl);

	mdl.vbo = vbo_new(mdl.vertc*SIZEOF_MDL_VERT, mdl.verts);

	DEBUG(3, "[RES] Loaded model \"%s\" (%u triangles, %hhu materials)", path, mdl.vertc/3, mdl.materialc);
	return mdl;
}

static void print_model(struct Model mdl)
{
	DEBUG(1, "Model (%u vertices/%u triangles)", mdl.vertc, mdl.vertc/3);
	for (uint i = 0; i < mdl.materialc; i++)
		DEBUG(1, "\tMaterial %u: [%3.2f, %3.2f, %3.2f]", i, mdl.materials[i].rgb.r,
		      mdl.materials[i].rgb.g, mdl.materials[i].rgb.b);
	for (uint i = 0; i < 9*mdl.vertc; i += 9)
		DEBUG(1, "\t[%3u] %7.3f, %7.3f, %7.3f [%3.2f, %3.2f, %3.2f] [%5.2f %5.2f %5.2f]", i/9,
		      mdl.verts[i], mdl.verts[i+1], mdl.verts[i+2], mdl.verts[i+3], mdl.verts[i+4],
		      mdl.verts[i+5], mdl.verts[i+6], mdl.verts[i+7], mdl.verts[i+8]);
}

void free_model(struct Model* mdl)
{
	free(mdl->verts);
	buffer_free(&mdl->vbo);
}
