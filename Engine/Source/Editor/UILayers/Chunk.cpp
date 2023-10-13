#include "Chunk.h"

void editor::Chunk::AddChunk(const std::string& chunk, const std::vector<uint8_t>& buffer)
{
	ChunkBlock block;
	block.Chunk = chunk;
	block.Buffer = buffer;
	Blocks.push_back(block);
}

std::vector<uint8_t> editor::Chunk::Save()
{
	std::vector<uint8_t> data;

	for (const auto& binary : Blocks)
	{
		std::array<uint8_t, 4> chunk = { 0, 0, 0, 0 };
		std::memcpy(chunk.data(), binary.Chunk.data(), std::min(chunk.size(), binary.Chunk.size()));

		data.insert(data.end(), chunk.begin(), chunk.end());

		uint32_t bufferSize = static_cast<uint32_t>(binary.Buffer.size());
		data.insert(data.end(), reinterpret_cast<const uint8_t*>(&bufferSize), reinterpret_cast<const uint8_t*>(&bufferSize) + sizeof(uint32_t));

		data.insert(data.end(), binary.Buffer.begin(), binary.Buffer.end());
	}

	return data;
}

bool editor::Chunk::Load(const std::vector<uint8_t>& data)
{
	Blocks.clear();

	size_t pos = 0;

	while (pos < data.size())
	{
		std::array<uint8_t, 4> chunkData;
		std::memcpy(chunkData.data(), data.data() + pos, chunkData.size());
		pos += chunkData.size();

		std::string chunk(chunkData.begin(), chunkData.end());

		uint32_t size;
		std::memcpy(&size, data.data() + pos, sizeof(uint32_t));
		pos += sizeof(uint32_t);

		std::vector<uint8_t> buffer(data.begin() + pos, data.begin() + pos + size);
		pos += size;

		ChunkBlock binary;
		binary.Chunk = chunk;
		binary.Buffer = buffer;
		Blocks.push_back(binary);
	}

	return pos == data.size();
}
