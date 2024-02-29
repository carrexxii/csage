#ifndef MAP_H
#define MAP_H

struct Tileset {
	char image[64];
	char name[32];
	int first_gid;
	int margin;
	int spacing;
	int columns;
	int tw, th;
};

struct MapChunk {
	uint16 x, y, w, h;
	byte data[256];
};

struct MapLayer {
	char name[32];
	int x, y, w, h;
	int chunkc;
	struct MapChunk* chunks;
};

struct Map {
	int w, h, tw, th;
	int layerc;
	int tilesetc;
	struct MapLayer* layers;
	struct Tileset* tilesets;
};

struct Map map_new(const char* name);

#endif
