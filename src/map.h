#ifndef MAP_H
#define MAP_H

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
	int w, h;
	int layerc;
	struct MapLayer* layers;
};

struct Map map_new(const char* name);

#endif
