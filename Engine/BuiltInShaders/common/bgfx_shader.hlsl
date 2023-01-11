float intBitsToFloat(int _x)
{
    return asfloat(_x);
}
float2 intBitsToFloat(uint2 _x)
{
    return asfloat(_x);
}
float3 intBitsToFloat(uint3 _x)
{
    return asfloat(_x);
}
float4 intBitsToFloat(uint4 _x)
{
    return asfloat(_x);
}
float uintBitsToFloat(uint _x)
{
    return asfloat(_x);
}
float2 uintBitsToFloat(uint2 _x)
{
    return asfloat(_x);
}
float3 uintBitsToFloat(uint3 _x)
{
    return asfloat(_x);
}
float4 uintBitsToFloat(uint4 _x)
{
    return asfloat(_x);
}
uint floatBitsToUint(float _x)
{
    return asuint(_x);
}
uint2 floatBitsToUint(float2 _x)
{
    return asuint(_x);
}
uint3 floatBitsToUint(float3 _x)
{
    return asuint(_x);
}
uint4 floatBitsToUint(float4 _x)
{
    return asuint(_x);
}
int floatBitsToInt(float _x)
{
    return asint(_x);
}
int2 floatBitsToInt(float2 _x)
{
    return asint(_x);
}
int3 floatBitsToInt(float3 _x)
{
    return asint(_x);
}
int4 floatBitsToInt(float4 _x)
{
    return asint(_x);
}
uint bitfieldReverse(uint _x)
{
    return reversebits(_x);
}
uint2 bitfieldReverse(uint2 _x)
{
    return reversebits(_x);
}
uint3 bitfieldReverse(uint3 _x)
{
    return reversebits(_x);
}
uint4 bitfieldReverse(uint4 _x)
{
    return reversebits(_x);
}
uint packHalf2x16(float2 _x)
{
    return (f32tof16(_x.y) << 16) | f32tof16(_x.x);
}
float2 unpackHalf2x16(uint _x)
{
    return float2(f16tof32(_x & 0xffff), f16tof32(_x >> 16));
}
struct BgfxSampler2D
{
    SamplerState m_sampler;
    Texture2D m_texture;
};
struct BgfxISampler2D
{
    Texture2D<int4> m_texture;
};
struct BgfxUSampler2D
{
    Texture2D<uint4> m_texture;
};
struct BgfxSampler2DArray
{
    SamplerState m_sampler;
    Texture2DArray m_texture;
};
struct BgfxSampler2DShadow
{
    SamplerComparisonState m_sampler;
    Texture2D m_texture;
};
struct BgfxSampler2DArrayShadow
{
    SamplerComparisonState m_sampler;
    Texture2DArray m_texture;
};
struct BgfxSampler3D
{
    SamplerState m_sampler;
    Texture3D m_texture;
};
struct BgfxISampler3D
{
    Texture3D<int4> m_texture;
};
struct BgfxUSampler3D
{
    Texture3D<uint4> m_texture;
};
struct BgfxSamplerCube
{
    SamplerState m_sampler;
    TextureCube m_texture;
};
struct BgfxSamplerCubeShadow
{
    SamplerComparisonState m_sampler;
    TextureCube m_texture;
};
struct BgfxSampler2DMS
{
    Texture2DMS<float4> m_texture;
};
float4 bgfxTexture2D(BgfxSampler2D _sampler, float2 _coord)
{
    return _sampler.m_texture.Sample(_sampler.m_sampler, _coord);
}
float4 bgfxTexture2DBias(BgfxSampler2D _sampler, float2 _coord, float _bias)
{
    return _sampler.m_texture.SampleBias(_sampler.m_sampler, _coord, _bias);
}
float4 bgfxTexture2DLod(BgfxSampler2D _sampler, float2 _coord, float _level)
{
    return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level);
}
float4 bgfxTexture2DLodOffset(BgfxSampler2D _sampler, float2 _coord, float _level, int2 _offset)
{
    return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level, _offset);
}
float4 bgfxTexture2DProj(BgfxSampler2D _sampler, float3 _coord)
{
    float2 coord = _coord.xy * rcp(_coord.z);
    return _sampler.m_texture.Sample(_sampler.m_sampler, coord);
}
float4 bgfxTexture2DProj(BgfxSampler2D _sampler, float4 _coord)
{
    float2 coord = _coord.xy * rcp(_coord.w);
    return _sampler.m_texture.Sample(_sampler.m_sampler, coord);
}
float4 bgfxTexture2DGrad(BgfxSampler2D _sampler, float2 _coord, float2 _dPdx, float2 _dPdy)
{
    return _sampler.m_texture.SampleGrad(_sampler.m_sampler, _coord, _dPdx, _dPdy);
}
float4 bgfxTexture2DArray(BgfxSampler2DArray _sampler, float3 _coord)
{
    return _sampler.m_texture.Sample(_sampler.m_sampler, _coord);
}
float4 bgfxTexture2DArrayLod(BgfxSampler2DArray _sampler, float3 _coord, float _lod)
{
    return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _lod);
}
float4 bgfxTexture2DArrayLodOffset(BgfxSampler2DArray _sampler, float3 _coord, float _level, int2 _offset)
{
    return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level, _offset);
}
float bgfxShadow2D(BgfxSampler2DShadow _sampler, float3 _coord)
{
    return _sampler.m_texture.SampleCmpLevelZero(_sampler.m_sampler, _coord.xy, _coord.z);
}
float bgfxShadow2DProj(BgfxSampler2DShadow _sampler, float4 _coord)
{
    float3 coord = _coord.xyz * rcp(_coord.w);
    return _sampler.m_texture.SampleCmpLevelZero(_sampler.m_sampler, coord.xy, coord.z);
}
float4 bgfxShadow2DArray(BgfxSampler2DArrayShadow _sampler, float4 _coord)
{
    return _sampler.m_texture.SampleCmpLevelZero(_sampler.m_sampler, _coord.xyz, _coord.w);
}
float4 bgfxTexture3D(BgfxSampler3D _sampler, float3 _coord)
{
    return _sampler.m_texture.Sample(_sampler.m_sampler, _coord);
}
float4 bgfxTexture3DLod(BgfxSampler3D _sampler, float3 _coord, float _level)
{
    return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level);
}
int4 bgfxTexture3D(BgfxISampler3D _sampler, float3 _coord)
{
    uint3 size;
    _sampler.m_texture.GetDimensions(size.x, size.y, size.z);
    return _sampler.m_texture.Load(int4(_coord * size, 0));
}
uint4 bgfxTexture3D(BgfxUSampler3D _sampler, float3 _coord)
{
    uint3 size;
    _sampler.m_texture.GetDimensions(size.x, size.y, size.z);
    return _sampler.m_texture.Load(int4(_coord * size, 0));
}
float4 bgfxTextureCube(BgfxSamplerCube _sampler, float3 _coord)
{
    return _sampler.m_texture.Sample(_sampler.m_sampler, _coord);
}
float4 bgfxTextureCubeBias(BgfxSamplerCube _sampler, float3 _coord, float _bias)
{
    return _sampler.m_texture.SampleBias(_sampler.m_sampler, _coord, _bias);
}
float4 bgfxTextureCubeLod(BgfxSamplerCube _sampler, float3 _coord, float _level)
{
    return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level);
}
float bgfxShadowCube(BgfxSamplerCubeShadow _sampler, float4 _coord)
{
    return _sampler.m_texture.SampleCmpLevelZero(_sampler.m_sampler, _coord.xyz, _coord.w);
}
float4 bgfxTexelFetch(BgfxSampler2D _sampler, int2 _coord, int _lod)
{
    return _sampler.m_texture.Load(int3(_coord, _lod));
}
float4 bgfxTexelFetchOffset(BgfxSampler2D _sampler, int2 _coord, int _lod, int2 _offset)
{
    return _sampler.m_texture.Load(int3(_coord, _lod), _offset);
}
float2 bgfxTextureSize(BgfxSampler2D _sampler, int _lod)
{
    float2 result;
    _sampler.m_texture.GetDimensions(result.x, result.y);
    return result;
}
float2 bgfxTextureSize(BgfxISampler2D _sampler, int _lod)
{
    float2 result;
    _sampler.m_texture.GetDimensions(result.x, result.y);
    return result;
}
float2 bgfxTextureSize(BgfxUSampler2D _sampler, int _lod)
{
    float2 result;
    _sampler.m_texture.GetDimensions(result.x, result.y);
    return result;
}
float4 bgfxTextureGather0(BgfxSampler2D _sampler, float2 _coord)
{
    return _sampler.m_texture.GatherRed(_sampler.m_sampler, _coord);
}
float4 bgfxTextureGather1(BgfxSampler2D _sampler, float2 _coord)
{
    return _sampler.m_texture.GatherGreen(_sampler.m_sampler, _coord);
}
float4 bgfxTextureGather2(BgfxSampler2D _sampler, float2 _coord)
{
    return _sampler.m_texture.GatherBlue(_sampler.m_sampler, _coord);
}
float4 bgfxTextureGather3(BgfxSampler2D _sampler, float2 _coord)
{
    return _sampler.m_texture.GatherAlpha(_sampler.m_sampler, _coord);
}
float4 bgfxTextureGatherOffset0(BgfxSampler2D _sampler, float2 _coord, int2 _offset)
{
    return _sampler.m_texture.GatherRed(_sampler.m_sampler, _coord, _offset);
}
float4 bgfxTextureGatherOffset1(BgfxSampler2D _sampler, float2 _coord, int2 _offset)
{
    return _sampler.m_texture.GatherGreen(_sampler.m_sampler, _coord, _offset);
}
float4 bgfxTextureGatherOffset2(BgfxSampler2D _sampler, float2 _coord, int2 _offset)
{
    return _sampler.m_texture.GatherBlue(_sampler.m_sampler, _coord, _offset);
}
float4 bgfxTextureGatherOffset3(BgfxSampler2D _sampler, float2 _coord, int2 _offset)
{
    return _sampler.m_texture.GatherAlpha(_sampler.m_sampler, _coord, _offset);
}
float4 bgfxTextureGather0(BgfxSampler2DArray _sampler, float3 _coord)
{
    return _sampler.m_texture.GatherRed(_sampler.m_sampler, _coord);
}
float4 bgfxTextureGather1(BgfxSampler2DArray _sampler, float3 _coord)
{
    return _sampler.m_texture.GatherGreen(_sampler.m_sampler, _coord);
}
float4 bgfxTextureGather2(BgfxSampler2DArray _sampler, float3 _coord)
{
    return _sampler.m_texture.GatherBlue(_sampler.m_sampler, _coord);
}
float4 bgfxTextureGather3(BgfxSampler2DArray _sampler, float3 _coord)
{
    return _sampler.m_texture.GatherAlpha(_sampler.m_sampler, _coord);
}
int4 bgfxTexelFetch(BgfxISampler2D _sampler, int2 _coord, int _lod)
{
    return _sampler.m_texture.Load(int3(_coord, _lod));
}
uint4 bgfxTexelFetch(BgfxUSampler2D _sampler, int2 _coord, int _lod)
{
    return _sampler.m_texture.Load(int3(_coord, _lod));
}
float4 bgfxTexelFetch(BgfxSampler2DMS _sampler, int2 _coord, int _sampleIdx)
{
    return _sampler.m_texture.Load(_coord, _sampleIdx);
}
float4 bgfxTexelFetch(BgfxSampler2DArray _sampler, int3 _coord, int _lod)
{
    return _sampler.m_texture.Load(int4(_coord, _lod));
}
float4 bgfxTexelFetch(BgfxSampler3D _sampler, int3 _coord, int _lod)
{
    return _sampler.m_texture.Load(int4(_coord, _lod));
}
float3 bgfxTextureSize(BgfxSampler3D _sampler, int _lod)
{
    float3 result;
    _sampler.m_texture.GetDimensions(result.x, result.y, result.z);
    return result;
}
float3 instMul(float3 _vec, float3x3 _mtx)
{
    return mul(_mtx, _vec);
}
float3 instMul(float3x3 _mtx, float3 _vec)
{
    return mul(_vec, _mtx);
}
float4 instMul(float4 _vec, float4x4 _mtx)
{
    return mul(_mtx, _vec);
}
float4 instMul(float4x4 _mtx, float4 _vec)
{
    return mul(_vec, _mtx);
}
bool2 lessThan(float2 _a, float2 _b)
{
    return _a < _b;
}
bool3 lessThan(float3 _a, float3 _b)
{
    return _a < _b;
}
bool4 lessThan(float4 _a, float4 _b)
{
    return _a < _b;
}
bool2 lessThanEqual(float2 _a, float2 _b)
{
    return _a <= _b;
}
bool3 lessThanEqual(float3 _a, float3 _b)
{
    return _a <= _b;
}
bool4 lessThanEqual(float4 _a, float4 _b)
{
    return _a <= _b;
}
bool2 greaterThan(float2 _a, float2 _b)
{
    return _a > _b;
}
bool3 greaterThan(float3 _a, float3 _b)
{
    return _a > _b;
}
bool4 greaterThan(float4 _a, float4 _b)
{
    return _a > _b;
}
bool2 greaterThanEqual(float2 _a, float2 _b)
{
    return _a >= _b;
}
bool3 greaterThanEqual(float3 _a, float3 _b)
{
    return _a >= _b;
}
bool4 greaterThanEqual(float4 _a, float4 _b)
{
    return _a >= _b;
}
bool2 notEqual(float2 _a, float2 _b)
{
    return _a != _b;
}
bool3 notEqual(float3 _a, float3 _b)
{
    return _a != _b;
}
bool4 notEqual(float4 _a, float4 _b)
{
    return _a != _b;
}
bool2 equal(float2 _a, float2 _b)
{
    return _a == _b;
}
bool3 equal(float3 _a, float3 _b)
{
    return _a == _b;
}
bool4 equal(float4 _a, float4 _b)
{
    return _a == _b;
}
float mix(float _a, float _b, float _t)
{
    return lerp(_a, _b, _t);
}
float2 mix(float2 _a, float2 _b, float2 _t)
{
    return lerp(_a, _b, _t);
}
float3 mix(float3 _a, float3 _b, float3 _t)
{
    return lerp(_a, _b, _t);
}
float4 mix(float4 _a, float4 _b, float4 _t)
{
    return lerp(_a, _b, _t);
}
float mod(float _a, float _b)
{
    return _a - _b * floor(_a / _b);
}
float2 mod(float2 _a, float2 _b)
{
    return _a - _b * floor(_a / _b);
}
float3 mod(float3 _a, float3 _b)
{
    return _a - _b * floor(_a / _b);
}
float4 mod(float4 _a, float4 _b)
{
    return _a - _b * floor(_a / _b);
}
float2 vec2_splat(float _x)
{
    return float2(_x, _x);
}
float3 vec3_splat(float _x)
{
    return float3(_x, _x, _x);
}
float4 vec4_splat(float _x)
{
    return float4(_x, _x, _x, _x);
}
uint2 uvec2_splat(uint _x)
{
    return uint2(_x, _x);
}
uint3 uvec3_splat(uint _x)
{
    return uint3(_x, _x, _x);
}
uint4 uvec4_splat(uint _x)
{
    return uint4(_x, _x, _x, _x);
}
float4x4 mtxFromRows(float4 _0, float4 _1, float4 _2, float4 _3)
{
    return float4x4(_0, _1, _2, _3);
}
float4x4 mtxFromCols(float4 _0, float4 _1, float4 _2, float4 _3)
{
    return transpose(float4x4(_0, _1, _2, _3));
}
float3x3 mtxFromRows(float3 _0, float3 _1, float3 _2)
{
    return float3x3(_0, _1, _2);
}
float3x3 mtxFromCols(float3 _0, float3 _1, float3 _2)
{
    return transpose(float3x3(_0, _1, _2));
}
uniform float4 u_viewRect;
uniform float4 u_viewTexel;
uniform float4x4 u_view;
uniform float4x4 u_invView;
uniform float4x4 u_proj;
uniform float4x4 u_invProj;
uniform float4x4 u_viewProj;
uniform float4x4 u_invViewProj;
uniform float4x4 u_model[32];
uniform float4x4 u_modelView;
uniform float4x4 u_modelViewProj;
uniform float4x4 u_modelInvTrans;
uniform float4 u_alphaRef4;
void main(float4 gl_FragCoord: SV_POSITION, float4 v_color0: COLOR0, out float4 bgfx_FragData0: SV_TARGET0)
{
    float4 bgfx_VoidFrag = vec4_splat(0.0);
    bgfx_FragData0 = v_color0;
}
