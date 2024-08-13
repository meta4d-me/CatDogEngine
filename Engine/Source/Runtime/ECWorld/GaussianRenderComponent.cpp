#include "GaussianRenderComponent.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Scene/Types.h"
#include "Scene/VertexFormat.h"
#include "Log\Log.h"
#include "GaussianUtil\DataView.h"

namespace engine
{

void GaussianRenderComponent::Build()
{
	cd::VertexFormat vertexFormat;
	vertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Position, cd::AttributeValueType::Float, 3);

	const uint32_t vertexCount = 4;
	std::vector<cd::Point> vertexArray;
	vertexArray.resize(vertexCount);
	// pos 
	for (uint32_t i = 0; i < vertexCount; i += 4)
	{
		vertexArray[i] = { cd::Point{-2, -2, 0} };
		vertexArray[i + 1] = { cd::Point{2, -2, 0} };
		vertexArray[i + 2] = { cd::Point{2, 2, 0} };
		vertexArray[i + 3] = { cd::Point{-2, 2, 0} };
	}

	m_vertexBuffer.resize(vertexCount * vertexFormat.GetStride());
	uint32_t currentDataSize = 0U;
	auto currentDataPtr = m_vertexBuffer.data();
	for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
	{
		//position
		const cd::Point& position = vertexArray[vertexIndex];
		constexpr uint32_t posDataSize = cd::Point::Size * sizeof(cd::Point::ValueType);
		std::memcpy(&currentDataPtr[currentDataSize], position.begin(), posDataSize);
		currentDataSize += posDataSize;
	}

	const bool useU16Index = false;
	const uint32_t indexTypeSize = useU16Index ? sizeof(uint16_t) : sizeof(uint32_t);
	const uint32_t indicesCount = 6;
	m_indexBuffer.resize(indicesCount * indexTypeSize);
	currentDataSize = 0U;
	currentDataPtr = m_indexBuffer.data();

	std::vector<uint16_t> indexes = { 0,1,2,0,3,2 };

	for (const auto& index : indexes)
	{
		std::memcpy(&currentDataPtr[currentDataSize], &index, indexTypeSize);
		currentDataSize += static_cast<uint32_t>(indexTypeSize);
	}

	bgfx::VertexLayout vertexLayout;
	VertexLayoutUtility::CreateVertexLayout(vertexLayout, vertexFormat.GetVertexAttributeLayouts());
	m_vertexBufferHandle = bgfx::createVertexBuffer(bgfx::makeRef(m_vertexBuffer.data(), static_cast<uint32_t>(m_vertexBuffer.size())), vertexLayout).idx;
	m_indexBufferHandle = bgfx::createIndexBuffer(bgfx::makeRef(m_indexBuffer.data(), static_cast<uint32_t>(m_indexBuffer.size())), 0U).idx;
}

void GaussianRenderComponent::ProcessingPlyBuffer()
{
	std::string header(reinterpret_cast<const char*>(m_readBuffer.data()), 1024 * 10);
	const std::string header_end = "end_header\n";
	const size_t header_end_index = header.find(header_end);
	if (header_end_index == std::string::npos)
	{
		throw std::runtime_error("Unable to read .ply file header");
	}

	const std::string vertexCountStr = header.substr(header.find("element vertex ") + 15, header_end_index - header.find("element vertex ") - 15);
	const int vertexCount = std::stoi(vertexCountStr);
	m_vertextCount = vertexCount;
	CD_ERROR("Vertex Count: ");
	CD_ERROR(vertexCount);

	size_t  row_offset = 0;
	std::unordered_map<std::string, size_t> offsets;
	std::unordered_map<std::string, std::string> types;

	const std::unordered_map<std::string, std::string> TYPE_MAP = {
		{"double", "getFloat64"},
		{"int", "getInt32"},
		{"uint", "getUint32"},
		{"float", "getFloat32"},
		{"short", "getInt16"},
		{"ushort", "getUint16"},
		{"uchar", "getUint8"}
	};

	std::string delimiter = "\n";
	size_t pos = 0;
	while ((pos = header.find(delimiter)) != std::string::npos)
	{
		std::string line = header.substr(0, pos);
		header.erase(0, pos + delimiter.length());

		if (line.find("property ") == 0)
		{
			std::istringstream iss(line);
			std::string property, type, name;

			iss >> property >> type >> name;
			std::string arrayType = TYPE_MAP.count(type) ? TYPE_MAP.at(type) : "getInt8";

			types[name] = arrayType;
			offsets[name] = row_offset;
			CD_ERROR(name);
			CD_ERROR(offsets[name]);	
			std::string number_str;
			for (char c : arrayType)
			{
				if (isdigit(c))
				{
					number_str += c;
				}
			}

			if (!number_str.empty())
			{
				row_offset += std::stoi(number_str) / 8;
			}
		}
	}

	CD_ERROR("all row_offset: ");
	CD_ERROR(row_offset);

	std::vector<float> sizeList(vertexCount);
	std::vector<size_t> sizeIndex(vertexCount);
	DataView dataView(m_readBuffer, header_end_index + header_end.length());

	for (size_t row = 0; row < vertexCount; ++row)
	{
		sizeIndex[row] = row;
		if (types.find("scale_0") == types.end()) continue;

		float scale_0 = dataView.get<float>(row * row_offset + offsets["scale_0"]);
		float scale_1 = dataView.get<float>(row * row_offset + offsets["scale_1"]);
		float scale_2 = dataView.get<float>(row * row_offset + offsets["scale_2"]);
		float opacity = dataView.get<float>(row * row_offset + offsets["opacity"]);

		float size = std::exp(scale_0) * std::exp(scale_1) * std::exp(scale_2);
		float opacity_value = 1 / (1 + std::exp(-opacity));
		sizeList[row] = size * opacity_value;
	}

	std::sort(sizeIndex.begin(), sizeIndex.end(), [&sizeList](size_t a, size_t b){
		return sizeList[a] > sizeList[b];
		});


	// 6*4 + 4 + 4 = 8*4
// XYZ - Position (Float32)
// XYZ - Scale (Float32)
// RGBA - colors (uint8)
// IJKL - quaternion/rot (uint8)

	const size_t rowLength = 3 * 4 + 3 * 4 + 4 + 4;
	std::vector<std::byte> buffer(rowLength * vertexCount);

	for (size_t j = 0; j < vertexCount; ++j)
	{
		size_t row = sizeIndex[j];

		float* position = reinterpret_cast<float*>(&buffer[j * rowLength]);
		float* scales = reinterpret_cast<float*>(&buffer[j * rowLength + 4 * 3]);
		std::byte* rgba = &buffer[j * rowLength + 4 * 3 + 4 * 3];
		std::byte* rot = &buffer[j * rowLength + 4 * 3 + 4 * 3 + 4];

		float scale_0 = dataView.get<float>(row * rowLength + offsets.at("scale_0"));
		float scale_1 = dataView.get<float>(row * rowLength + offsets.at("scale_1"));
		float scale_2 = dataView.get<float>(row * rowLength + offsets.at("scale_2"));
		float opacity = dataView.get<float>(row * rowLength + offsets.at("opacity"));

		if (types.find("scale_0") != types.end())
		{
			float rot_0 = dataView.get<float>(row * rowLength + offsets.at("rot_0"));
			float rot_1 = dataView.get<float>(row * rowLength + offsets.at("rot_1"));
			float rot_2 = dataView.get<float>(row * rowLength + offsets.at("rot_2"));
			float rot_3 = dataView.get<float>(row * rowLength + offsets.at("rot_3"));

			float qlen = std::sqrt(
				rot_0 * rot_0 +
				rot_1 * rot_1 +
				rot_2 * rot_2 +
				rot_3 * rot_3
			);

			rot[0] = static_cast<std::byte>((rot_0 / qlen) * 128 + 128);
			rot[1] = static_cast<std::byte>((rot_1 / qlen) * 128 + 128);
			rot[2] = static_cast<std::byte>((rot_2 / qlen) * 128 + 128);
			rot[3] = static_cast<std::byte>((rot_3 / qlen) * 128 + 128);

			scales[0] = std::exp(scale_0);
			scales[1] = std::exp(scale_1);
			scales[2] = std::exp(scale_2);
		}
		else
		{
			scales[0] = 0.01f;
			scales[1] = 0.01f;
			scales[2] = 0.01f;

			rot[0] = std::byte{ 255 };
			rot[1] = std::byte{ 0 };
			rot[2] = std::byte{ 0 };
			rot[3] = std::byte{ 0 };
		}

		position[0] = dataView.get<float>(row * rowLength + offsets.at("x"));
		position[1] = dataView.get<float>(row * rowLength + offsets.at("y"));
		position[2] = dataView.get<float>(row * rowLength + offsets.at("z"));

		if (types.find("f_dc_0") != types.end())
		{
			const float SH_C0 = 0.28209479177387814f;
			rgba[0] = static_cast<std::byte>((0.5f + SH_C0 * dataView.get<float>(row * rowLength + offsets.at("f_dc_0"))) * 255);
			rgba[1] = static_cast<std::byte>((0.5f + SH_C0 * dataView.get<float>(row * rowLength + offsets.at("f_dc_1"))) * 255);
			rgba[2] = static_cast<std::byte>((0.5f + SH_C0 * dataView.get<float>(row * rowLength + offsets.at("f_dc_2"))) * 255);
		}
		else
		{
			rgba[0] = std::byte{ dataView.get<uint8_t>(row * rowLength + offsets.at("red")) };
			rgba[1] = std::byte{ dataView.get<uint8_t>(row * rowLength + offsets.at("green")) };
			rgba[2] = std::byte{ dataView.get<uint8_t>(row * rowLength + offsets.at("blue")) };
		}

		if (types.find("opacity") != types.end())
		{
			rgba[3] = static_cast<std::byte>((1 / (1 + std::exp(-opacity))) * 255);
		}
		else
		{
			rgba[3] = std::byte{ 255 };
		}
	}

	m_gausianAttributesBuffer = buffer;
	CD_ERROR("ReadBuffer Over");
}

uint16_t floatToHalf(float value)
{
	uint32_t f = *reinterpret_cast<uint32_t*>(&value);
	uint32_t sign = (f >> 16) & 0x8000;
	uint32_t exponent = ((f >> 23) & 0xff) - (127 - 15);
	uint32_t mantissa = f & 0x007fffff;

	if (exponent <= 0)
	{
		if (exponent < -10)
		{
			return static_cast<uint16_t>(sign);
		}
		mantissa = (mantissa | 0x00800000) >> (1 - exponent);
		return static_cast<uint16_t>(sign | (mantissa >> 13));
	}
	else if (exponent == 0xff - (127 - 15))
	{
		if (mantissa == 0)
		{
			return static_cast<uint16_t>(sign | 0x7c00);
		}
		else
		{
			mantissa >>= 13;
			return static_cast<uint16_t>(sign | 0x7c00 | mantissa | (mantissa == 0));
		}
	}
	else
	{
		if (exponent > 30)
		{
			return static_cast<uint16_t>(sign | 0x7c00);
		}
		return static_cast<uint16_t>(sign | (exponent << 10) | (mantissa >> 13));
	}
}


uint32_t packHalf2x16(float v1, float v2)
{
	uint16_t half1 = floatToHalf(v1);
	uint16_t half2 = floatToHalf(v2);
	return (static_cast<uint32_t>(half1) << 16) | static_cast<uint32_t>(half2);
}

void GaussianRenderComponent::GenerateTexture()
{
	if (m_gausianAttributesBuffer.empty()) return;

	const float* f_buffer = reinterpret_cast<const float*>(m_gausianAttributesBuffer.data());
	const uint8_t* u_buffer = reinterpret_cast<const uint8_t*>(m_gausianAttributesBuffer.data());

	size_t  texwidth = 1024 * 2;
	size_t  texheight = static_cast<int>(std::ceil((2 * m_vertextCount) / static_cast<float>(texwidth)));
	m_textureBuffer.resize(texwidth * texheight * 4 * sizeof(uint32_t));
	const bgfx::Caps* caps = bgfx::getCaps();
	uint16_t maxTextureSize = caps->limits.maxTextureSize;

	uint8_t* texdata_c = reinterpret_cast<uint8_t*>(m_textureBuffer.data());
	float* texdata_f = reinterpret_cast<float*>(m_textureBuffer.data());

	for (size_t i = 0; i < m_vertextCount; ++i)
	{
		// x, y, z
		texdata_f[8 * i + 0] = f_buffer[8 * i + 0];
		texdata_f[8 * i + 1] = f_buffer[8 * i + 1];
		texdata_f[8 * i + 2] = f_buffer[8 * i + 2];

		// r, g, b, a
		texdata_c[4 * (8 * i + 7) + 0] = u_buffer[32 * i + 24 + 0];
		texdata_c[4 * (8 * i + 7) + 1] = u_buffer[32 * i + 24 + 1];
		texdata_c[4 * (8 * i + 7) + 2] = u_buffer[32 * i + 24 + 2];
		texdata_c[4 * (8 * i + 7) + 3] = u_buffer[32 * i + 24 + 3];

		// quaternions
		float scale[3] = {
			f_buffer[8 * i + 3 + 0],
			f_buffer[8 * i + 3 + 1],
			f_buffer[8 * i + 3 + 2]
		};
		float rot[4] = {
			(u_buffer[32 * i + 28 + 0] - 128) / 128.0f,
			(u_buffer[32 * i + 28 + 1] - 128) / 128.0f,
			(u_buffer[32 * i + 28 + 2] - 128) / 128.0f,
			(u_buffer[32 * i + 28 + 3] - 128) / 128.0f
		};

		// Compute the matrix product of S and R (M = Scale * Rotation)
		float M[9] = {
			1.0f - 2.0f * (rot[2] * rot[2] + rot[3] * rot[3]),
			2.0f * (rot[1] * rot[2] + rot[0] * rot[3]),
			2.0f * (rot[1] * rot[3] - rot[0] * rot[2]),

			2.0f * (rot[1] * rot[2] - rot[0] * rot[3]),
			1.0f - 2.0f * (rot[1] * rot[1] + rot[3] * rot[3]),
			2.0f * (rot[2] * rot[3] + rot[0] * rot[1]),

			2.0f * (rot[1] * rot[3] + rot[0] * rot[2]),
			2.0f * (rot[2] * rot[3] - rot[0] * rot[1]),
			1.0f - 2.0f * (rot[1] * rot[1] + rot[2] * rot[2])
		};

		for (int j = 0; j < 9; ++j)
		{
			M[j] *= scale[j / 3];
		}

		float sigma[6] = {
			M[0] * M[0] + M[3] * M[3] + M[6] * M[6],
			M[0] * M[1] + M[3] * M[4] + M[6] * M[7],
			M[0] * M[2] + M[3] * M[5] + M[6] * M[8],
			M[1] * M[1] + M[4] * M[4] + M[7] * M[7],
			M[1] * M[2] + M[4] * M[5] + M[7] * M[8],
			M[2] * M[2] + M[5] * M[5] + M[8] * M[8]
		};

		uint32_t packed1 = packHalf2x16(4 * sigma[0], 4 * sigma[1]);
		uint32_t packed2 = packHalf2x16(4 * sigma[2], 4 * sigma[3]);
		uint32_t packed3 = packHalf2x16(4 * sigma[4], 4 * sigma[5]);

		std::memcpy(&m_textureBuffer[8 * i + 4 * sizeof(uint32_t)], &packed1, sizeof(uint32_t));
		std::memcpy(&m_textureBuffer[8 * i + 5 * sizeof(uint32_t)], &packed2, sizeof(uint32_t));
		std::memcpy(&m_textureBuffer[8 * i + 6 * sizeof(uint32_t)], &packed3, sizeof(uint32_t));
	}

	bool hasMips = false;  //if-mip-map
	uint16_t numLayers = 1;
	bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA32U; // fomat

	const bgfx::Memory* mem = bgfx::copy(m_textureBuffer.data(), static_cast<uint32_t>(m_textureBuffer.size()));
	m_textureHandle = bgfx::createTexture2D(static_cast<uint16_t>(texwidth), static_cast<uint16_t>(texheight), hasMips, numLayers, format, BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, mem);
	CD_ERROR("Generate TextureHandle Over");
}


}