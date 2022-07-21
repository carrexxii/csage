#include "util/file.h"
#include "buffers.h"
#include "model.h"

#define LINE_BUFFER_SIZE 256

struct Model create_model(char* const path)
{
	struct Model mdl = { 0 };

	FILE* file = file_open(path, "r");
	char line[LINE_BUFFER_SIZE];

	char   name[64];
	uint   vertc;
	float* vert;
	while (fgets(line, LINE_BUFFER_SIZE, file)) {
		/* Object header */
		if (sscanf(line, "%64[a-zA-Z_]: %u", name, &vertc)) {
			mdl.verts = smalloc(vertc*sizeof(float));
		} else { /* Mesh vertices */
			vert = mdl.verts + 3*mdl.vertc++;
			fscanf(file, "%f %f %f", vert, vert + 1, vert + 2);
		}
	}

	fclose(file);
	DEBUG(3, "[RES] Loaded model \"%s\"", path);
	print_model(mdl, name);
	exit(0);
	return mdl;
}

void print_model(struct Model mdl, char* name)
{
	DEBUG(1, "Model \"%s\"", name);
	for (uint i = 0; i < mdl.vertc; i++)
		DEBUG(1, "\t[%u] %f, %f, %f", i, mdl.verts[i], mdl.verts[i+1], mdl.verts[i+2]);
}
