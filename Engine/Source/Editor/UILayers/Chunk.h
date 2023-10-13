#pragma once
#include <filesystem>
#include <array>

namespace editor
{
struct ChunkBlock
{
public:
	std::string Chunk;
	std::vector<uint8_t> Buffer;
};

class Chunk
{
public:
	std::vector<ChunkBlock> Blocks;

	void AddChunk(const std::string& chunk, const std::vector<uint8_t>& buffer);

	std::vector<uint8_t> Save();

	bool Load(const std::vector<uint8_t>& data);
};

}