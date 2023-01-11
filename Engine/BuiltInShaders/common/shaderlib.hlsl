float4 encodeRE8(float _r)
{
    float exponent = ceil(log2(_r));
    return float4(_r / exp2(exponent), 0.0, 0.0, (exponent + 128.0) / 255.0);
}

float decodeRE8(float4 _re8)
{
    float exponent = _re8.w * 255.0 - 128.0;
    return _re8.x * exp2(exponent);
}

float4 encodeRGBE8(float3 _rgb)
{
    float4 rgbe8;
    float maxComponent = max(max(_rgb.x, _rgb.y), _rgb.z);
    float exponent = ceil(log2(maxComponent));
    rgbe8.xyz = _rgb / exp2(exponent);
    rgbe8.w = (exponent + 128.0) / 255.0;
    return rgbe8;
}

float3 decodeRGBE8(float4 _rgbe8)
{
    float exponent = _rgbe8.w * 255.0 - 128.0;
    float3 rgb = _rgbe8.xyz * exp2(exponent);
    return rgb;
}

float3 encodeNormalUint(float3 _normal)
{
    return _normal * 0.5 + 0.5;
}

float3 decodeNormalUint(float3 _encodedNormal)
{
    return _encodedNormal * 2.0 - 1.0;
}

float2 encodeNormalSphereMap(float3 _normal)
{
    return normalize(_normal.xy) * sqrt(_normal.z * 0.5 + 0.5);
}

float3 decodeNormalSphereMap(float2 _encodedNormal)
{
    float zz = dot(_encodedNormal, _encodedNormal) * 2.0 - 1.0;
    return float3(normalize(_encodedNormal.xy) * sqrt(1.0 - zz * zz), zz);
}

float2 octahedronWrap(float2 _val)
{
    return (1.0 - abs(_val.yx)) * mix(vec2_splat(-1.0), vec2_splat(1.0), float2(greaterThanEqual(_val.xy, vec2_splat(0.0))));
}

float2 encodeNormalOctahedron(float3 _normal)
{
    _normal /= abs(_normal.x) + abs(_normal.y) + abs(_normal.z);
    _normal.xy = _normal.z >= 0.0 ? _normal.xy : octahedronWrap(_normal.xy);
    _normal.xy = _normal.xy * 0.5 + 0.5;
    return _normal.xy;
}

float3 decodeNormalOctahedron(float2 _encodedNormal)
{
    _encodedNormal = _encodedNormal * 2.0 - 1.0;
    float3 normal;
    normal.z = 1.0 - abs(_encodedNormal.x) - abs(_encodedNormal.y);
    normal.xy = normal.z >= 0.0 ? _encodedNormal.xy : octahedronWrap(_encodedNormal.xy);
    return normalize(normal);
}

float3 convertRGB2XYZ(float3 _rgb)
{
    float3 xyz;
    xyz.x = dot(float3(0.4124564, 0.3575761, 0.1804375), _rgb);
    xyz.y = dot(float3(0.2126729, 0.7151522, 0.0721750), _rgb);
    xyz.z = dot(float3(0.0193339, 0.1191920, 0.9503041), _rgb);
    return xyz;
}

float3 convertXYZ2RGB(float3 _xyz)
{
    float3 rgb;
    rgb.x = dot(float3(3.2404542, -1.5371385, -0.4985314), _xyz);
    rgb.y = dot(float3(-0.9692660, 1.8760108, 0.0415560), _xyz);
    rgb.z = dot(float3(0.0556434, -0.2040259, 1.0572252), _xyz);
    return rgb;
}

float3 convertXYZ2Yxy(float3 _xyz)
{
    float inv = 1.0 / dot(_xyz, float3(1.0, 1.0, 1.0));
    return float3(_xyz.y, _xyz.x * inv, _xyz.y * inv);
}

float3 convertYxy2XYZ(float3 _Yxy)
{
    float3 xyz;
    xyz.x = _Yxy.x * _Yxy.y / _Yxy.z;
    xyz.y = _Yxy.x;
    xyz.z = _Yxy.x * (1.0 - _Yxy.y - _Yxy.z) / _Yxy.z;
    return xyz;
}

float3 convertRGB2Yxy(float3 _rgb)
{
    return convertXYZ2Yxy(convertRGB2XYZ(_rgb));
}

float3 convertYxy2RGB(float3 _Yxy)
{
    return convertXYZ2RGB(convertYxy2XYZ(_Yxy));
}

float3 convertRGB2Yuv(float3 _rgb)
{
    float3 yuv;
    yuv.x = dot(_rgb, float3(0.299, 0.587, 0.114));
    yuv.y = (_rgb.x - yuv.x) * 0.713 + 0.5;
    yuv.z = (_rgb.z - yuv.x) * 0.564 + 0.5;
    return yuv;
}

float3 convertYuv2RGB(float3 _yuv)
{
    float3 rgb;
    rgb.x = _yuv.x + 1.403 * (_yuv.y - 0.5);
    rgb.y = _yuv.x - 0.344 * (_yuv.y - 0.5) - 0.714 * (_yuv.z - 0.5);
    rgb.z = _yuv.x + 1.773 * (_yuv.z - 0.5);
    return rgb;
}

float3 convertRGB2YIQ(float3 _rgb)
{
    float3 yiq;
    yiq.x = dot(float3(0.299, 0.587, 0.114), _rgb);
    yiq.y = dot(float3(0.595716, -0.274453, -0.321263), _rgb);
    yiq.z = dot(float3(0.211456, -0.522591, 0.311135), _rgb);
    return yiq;
}

float3 convertYIQ2RGB(float3 _yiq)
{
    float3 rgb;
    rgb.x = dot(float3(1.0, 0.9563, 0.6210), _yiq);
    rgb.y = dot(float3(1.0, -0.2721, -0.6474), _yiq);
    rgb.z = dot(float3(1.0, -1.1070, 1.7046), _yiq);
    return rgb;
}

float3 toLinear(float3 _rgb)
{
    return pow(abs(_rgb), vec3_splat(2.2));
}

float4 toLinear(float4 _rgba)
{
    return float4(toLinear(_rgba.xyz), _rgba.w);
}

float3 toLinearAccurate(float3 _rgb)
{
    float3 lo = _rgb / 12.92;
    float3 hi = pow((_rgb + 0.055) / 1.055, vec3_splat(2.4));
    float3 rgb = mix(hi, lo, float3(lessThanEqual(_rgb, vec3_splat(0.04045))));
    return rgb;
}

float4 toLinearAccurate(float4 _rgba)
{
    return float4(toLinearAccurate(_rgba.xyz), _rgba.w);
}

float toGamma(float _r)
{
    return pow(abs(_r), 1.0 / 2.2);
}

float3 toGamma(float3 _rgb)
{
    return pow(abs(_rgb), vec3_splat(1.0 / 2.2));
}

float4 toGamma(float4 _rgba)
{
    return float4(toGamma(_rgba.xyz), _rgba.w);
}

float3 toGammaAccurate(float3 _rgb)
{
    float3 lo = _rgb * 12.92;
    float3 hi = pow(abs(_rgb), vec3_splat(1.0 / 2.4)) * 1.055 - 0.055;
    float3 rgb = mix(hi, lo, float3(lessThanEqual(_rgb, vec3_splat(0.0031308))));
    return rgb;
}

float4 toGammaAccurate(float4 _rgba)
{
    return float4(toGammaAccurate(_rgba.xyz), _rgba.w);
}

float3 toReinhard(float3 _rgb)
{
    return toGamma(_rgb / (_rgb + vec3_splat(1.0)));
}

float4 toReinhard(float4 _rgba)
{
    return float4(toReinhard(_rgba.xyz), _rgba.w);
}

float3 toFilmic(float3 _rgb)
{
    _rgb = max(vec3_splat(0.0), _rgb - 0.004);
    _rgb = (_rgb * (6.2 * _rgb + 0.5)) / (_rgb * (6.2 * _rgb + 1.7) + 0.06);
    return _rgb;
}

float4 toFilmic(float4 _rgba)
{
    return float4(toFilmic(_rgba.xyz), _rgba.w);
}

float3 toAcesFilmic(float3 _rgb)
{
    float aa = 2.51f;
    float bb = 0.03f;
    float cc = 2.43f;
    float dd = 0.59f;
    float ee = 0.14f;
    return saturate((_rgb * (aa * _rgb + bb)) / (_rgb * (cc * _rgb + dd) + ee));
}

float4 toAcesFilmic(float4 _rgba)
{
    return float4(toAcesFilmic(_rgba.xyz), _rgba.w);
}

float3 luma(float3 _rgb)
{
    float yy = dot(float3(0.2126729, 0.7151522, 0.0721750), _rgb);
    return vec3_splat(yy);
}

float4 luma(float4 _rgba)
{
    return float4(luma(_rgba.xyz), _rgba.w);
}

float3 conSatBri(float3 _rgb, float3 _csb)
{
    float3 rgb = _rgb * _csb.z;
    rgb = mix(luma(rgb), rgb, _csb.y);
    rgb = mix(vec3_splat(0.5), rgb, _csb.x);
    return rgb;
}

float4 conSatBri(float4 _rgba, float3 _csb)
{
    return float4(conSatBri(_rgba.xyz, _csb), _rgba.w);
}

float3 posterize(float3 _rgb, float _numColors)
{
    return floor(_rgb * _numColors) / _numColors;
}

float4 posterize(float4 _rgba, float _numColors)
{
    return float4(posterize(_rgba.xyz, _numColors), _rgba.w);
}

float3 sepia(float3 _rgb)
{
    float3 color;
    color.x = dot(_rgb, float3(0.393, 0.769, 0.189));
    color.y = dot(_rgb, float3(0.349, 0.686, 0.168));
    color.z = dot(_rgb, float3(0.272, 0.534, 0.131));
    return color;
}

float4 sepia(float4 _rgba)
{
    return float4(sepia(_rgba.xyz), _rgba.w);
}

float3 blendOverlay(float3 _base, float3 _blend)
{
    float3 lt = 2.0 * _base * _blend;
    float3 gte = 1.0 - 2.0 * (1.0 - _base) * (1.0 - _blend);
    return mix(lt, gte, step(vec3_splat(0.5), _base));
}

float4 blendOverlay(float4 _base, float4 _blend)
{
    return float4(blendOverlay(_base.xyz, _blend.xyz), _base.w);
}

float3 adjustHue(float3 _rgb, float _hue)
{
    float3 yiq = convertRGB2YIQ(_rgb);
    float angle = _hue + atan2(yiq.z, yiq.y);
    float len = length(yiq.yz);
    return convertYIQ2RGB(float3(yiq.x, len * cos(angle), len * sin(angle)));
}

float4 packFloatToRgba(float _value)
{
    const float4 shift = float4(256 * 256 * 256, 256 * 256, 256, 1.0);
    const float4 mask = float4(0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);
    float4 comp = frac(_value * shift);
    comp -= comp.xxyz * mask;
    return comp;
}

float unpackRgbaToFloat(float4 _rgba)
{
    const float4 shift = float4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
    return dot(_rgba, shift);
}

float2 packHalfFloat(float _value)
{
    const float2 shift = float2(256, 1.0);
    const float2 mask = float2(0, 1.0 / 256.0);
    float2 comp = frac(_value * shift);
    comp -= comp.xx * mask;
    return comp;
}

float unpackHalfFloat(float2 _rg)
{
    const float2 shift = float2(1.0 / 256.0, 1.0);
    return dot(_rg, shift);
}

float random(float2 _uv)
{
    return frac(sin(dot(_uv.xy, float2(12.9898, 78.233))) * 43758.5453);
}

float3 fixCubeLookup(float3 _v, float _lod, float _topLevelCubeSize)
{
    float ax = abs(_v.x);
    float ay = abs(_v.y);
    float az = abs(_v.z);
    float vmax = max(max(ax, ay), az);
    float scale = 1.0 - exp2(_lod) / _topLevelCubeSize;
    if (ax != vmax)
    {
        _v.x *= scale;
    }
    if (ay != vmax)
    {
        _v.y *= scale;
    }
    if (az != vmax)
    {
        _v.z *= scale;
    }
    return _v;
}

float3x3 cofactor(float4x4 _m)
{
    return float3x3(_m[1][1] * _m[2][2] - _m[1][2] * _m[2][1],
                    _m[1][2] * _m[2][0] - _m[1][0] * _m[2][2],
                    _m[1][0] * _m[2][1] - _m[1][1] * _m[2][0],
                    _m[0][2] * _m[2][1] - _m[0][1] * _m[2][2],
                    _m[0][0] * _m[2][2] - _m[0][2] * _m[2][0],
                    _m[0][1] * _m[2][0] - _m[0][0] * _m[2][1],
                    _m[0][1] * _m[1][2] - _m[0][2] * _m[1][1],
                    _m[0][2] * _m[1][0] - _m[0][0] * _m[1][2],
                    _m[0][0] * _m[1][1] - _m[0][1] * _m[1][0]);
}

float toClipSpaceDepth(float _depthTextureZ)
{
    return _depthTextureZ;
}

float3 clipToWorld(float4x4 _invViewProj, float3 _clipPos)
{
    float4 wpos = mul(_invViewProj, float4(_clipPos, 1.0));
    return wpos.xyz / wpos.w;
}
