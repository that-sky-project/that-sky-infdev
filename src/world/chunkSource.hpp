#ifndef __WORLD_CHUNKSOURCE_HPP__
#define __WORLD_CHUNKSOURCE_HPP__

class ChunkSource {
public:
  virtual void LoadChunk(ClientChunk &chunk);
};

#endif
