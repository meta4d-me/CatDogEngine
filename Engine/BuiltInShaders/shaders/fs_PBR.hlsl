float3 instMul(float3 _vec, float3x3 _mtx) { return mul(_mtx, _vec); }
float3 instMul(float3x3 _mtx, float3 _vec) { return mul(_vec, _mtx); }
float4 instMul(float4 _vec, float4x4 _mtx) { return mul(_mtx, _vec); }
float4 instMul(float4x4 _mtx, float4 _vec) { return mul(_vec, _mtx); }
bool2 lessThan(float2 _a, float2 _b) { return _a < _b; }
bool3 lessThan(float3 _a, float3 _b) { return _a < _b; }
bool4 lessThan(float4 _a, float4 _b) { return _a < _b; }
bool2 lessThanEqual(float2 _a, float2 _b) { return _a <= _b; }
bool3 lessThanEqual(float3 _a, float3 _b) { return _a <= _b; }
bool4 lessThanEqual(float4 _a, float4 _b) { return _a <= _b; }
bool2 greaterThan(float2 _a, float2 _b) { return _a > _b; }
bool3 greaterThan(float3 _a, float3 _b) { return _a > _b; }
bool4 greaterThan(float4 _a, float4 _b) { return _a > _b; }
bool2 greaterThanEqual(float2 _a, float2 _b) { return _a >= _b; }
bool3 greaterThanEqual(float3 _a, float3 _b) { return _a >= _b; }
bool4 greaterThanEqual(float4 _a, float4 _b) { return _a >= _b; }
bool2 notEqual(float2 _a, float2 _b) { return _a != _b; }
bool3 notEqual(float3 _a, float3 _b) { return _a != _b; }
bool4 notEqual(float4 _a, float4 _b) { return _a != _b; }
bool2 equal(float2 _a, float2 _b) { return _a == _b; }
bool3 equal(float3 _a, float3 _b) { return _a == _b; }
bool4 equal(float4 _a, float4 _b) { return _a == _b; }
float mix(float _a, float _b, float _t) { return lerp(_a, _b, _t); }
float2 mix(float2 _a, float2 _b, float2 _t) { return lerp(_a, _b, _t); }
float3 mix(float3 _a, float3 _b, float3 _t) { return lerp(_a, _b, _t); }
float4 mix(float4 _a, float4 _b, float4 _t) { return lerp(_a, _b, _t); }
float mod(float _a, float _b) { return _a - _b * floor(_a / _b); }
float2 mod(float2 _a, float2 _b) { return _a - _b * floor(_a / _b); }
float3 mod(float3 _a, float3 _b) { return _a - _b * floor(_a / _b); }
float4 mod(float4 _a, float4 _b) { return _a - _b * floor(_a / _b); }
float2 vec2_splat(float _x) { return float2(_x, _x); }
float3 vec3_splat(float _x) { return float3(_x, _x, _x); }
float4 vec4_splat(float _x) { return float4(_x, _x, _x, _x); }
uint2 uvec2_splat(uint _x) { return uint2(_x, _x); }
uint3 uvec3_splat(uint _x) { return uint3(_x, _x, _x); }
uint4 uvec4_splat(uint _x) { return uint4(_x, _x, _x, _x); }

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
    return float3x3(
        _m[1][1] * _m[2][2] - _m[1][2] * _m[2][1],
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

uniform float4 u_params[8];
uniform float4 u_lightCount;
uniform float4 u_lightStride;

/*uniform float lightType;
uniform float3 lightPosition;
uniform float lightIntensity;
uniform float3 lightColor;
uniform float lightRange;
uniform float3 lightDirection;
uniform float lightRadius;
uniform float3 lightUp;
uniform float lightWidth;
uniform float lightHeight;
uniform float lightAngleScale;
uniform float lightAngleOffeset;*/
uniform float4 u_lightParams[80];

Texture2D _AlbedoTexture : register(t0);
SamplerState _AlbedoSampler : register(s0);

Texture2D _NormalTexture : register(t1);
SamplerState _NormalSampler : register(s1);

Texture2D _ORMTexture : register(t2);
SamplerState _ORMSampler : register(s2);

Texture2D _LUTTexture : register(t3);
SamplerState _LUTSampler : register(s3);

TextureCube _CubeTexture : register(t4);
SamplerState _CubeSampler : register(s4);

TextureCube _CubeIrrTexture : register(t5);
SamplerState _CubeIrrSampler : register(s5);

struct U_Light
{
    float type;
    float3 position;
    float intensity;
    float3 color;
    float range;
    float3 direction;
    float radius;
    float3 up;
    float width;
    float height;
    float lightAngleScale;
    float lightAngleOffeset;
};

/*U_Light GetLightParams(int pointer)
{
    U_Light light;
    light.type = lightType;
    light.position = lightPosition;
    light.intensity = lightIntensity;
    light.color = lightColor;
    light.range = lightRange;
    light.direction = lightDirection;
    light.radius = lightRadius;
    light.up = lightUp;
    light.width = lightWidth;
    light.height = lightHeight;
    light.lightAngleScale = lightAngleScale;
    light.lightAngleOffeset = lightAngleOffeset;
    return light;
}*/

U_Light GetLightParams(int pointer)
{
    U_Light light;
    light.type = u_lightParams[pointer + 0].x;
    light.position = u_lightParams[pointer + 0].yzw;
    light.intensity = u_lightParams[pointer + 1].x;
    light.color = u_lightParams[pointer + 1].yzw;
    light.range = u_lightParams[pointer + 2].x;
    light.direction = u_lightParams[pointer + 2].yzw;
    light.radius = u_lightParams[pointer + 3].x;
    light.up = u_lightParams[pointer + 3].yzw;
    light.width = u_lightParams[pointer + 4].x;
    light.height = u_lightParams[pointer + 4].y;
    light.lightAngleScale = u_lightParams[pointer + 4].z;
    light.lightAngleOffeset = u_lightParams[pointer + 4].w;
    return light;
}

struct Material
{
    float3 albedo;
    float3 normal;
    float occlusion;
    float roughness;
    float metallic;
    float3 F0;
    float opacity;
    float3 emissive;
};

Material CreateMaterial()
{
    Material material;
    material.albedo = float3(0.99, 0.98, 0.95);
    material.normal = float3(0.0, 1.0, 0.0);
    material.occlusion = 1.0;
    material.roughness = 0.9;
    material.metallic = 0.1;
    material.F0 = float3(0.04, 0.04, 0.04);
    material.opacity = 1.0;
    material.emissive = vec3_splat(0.0);
    return material;
}

float3 CalcuateNormal(float3 worldPos)
{
    float3 dx = ddx(worldPos);
    float3 dy = ddy(-(worldPos));
    return normalize(cross(dx, dy));
}

float3 CalcuateF0(Material material)
{
    return mix(vec3_splat(0.04), material.albedo, material.metallic);
}

float SmoothDistanceAtt(float squaredDistance, float invSqrAttRadius)
{
    float factor = squaredDistance * invSqrAttRadius;
    float smoothFactor = saturate(1.0 - factor * factor);
    return smoothFactor * smoothFactor;
}

float GetDistanceAtt(float sqrDist, float invSqrAttRadius)
{
    float attenuation = 1.0 / (max(sqrDist, 0.0001));
    attenuation *= SmoothDistanceAtt(sqrDist, invSqrAttRadius);
    return attenuation;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(float NdotH, float rough)
{
    float a = rough * rough;
    float a2 = a * a;
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    denom = 3.1415926536 * denom * denom;
    return a2 / denom;
}

float Visibility(float NdotV, float NdotL, float rough)
{
    float f = rough + 1.0;
    float k = f * f * 0.125;
    float ggxV = 1.0 / (NdotV * (1.0 - k) + k);
    float ggxL = 1.0 / (NdotL * (1.0 - k) + k);
    return ggxV * ggxL * 0.25;
}

float GetAngleAtt(float3 lightDir, float3 lightForward, float lightAngleScale, float lightAngleOffeset)
{
    float cd = dot(lightDir, lightForward);
    float attenuation = saturate(cd * lightAngleScale + lightAngleOffeset);
    attenuation *= attenuation;
    return attenuation;
}

float cot(float x)
{
    return cos(x) / sin(x);
}

float acot(float x)
{
    return atan(1.0 / x);
}

float RectangleSolidAngle(float3 worldPos, float3 p0, float3 p1, float3 p2, float3 p3)
{
    float3 v0 = p0 - worldPos;
    float3 v1 = p1 - worldPos;
    float3 v2 = p2 - worldPos;
    float3 v3 = p3 - worldPos;
    float3 n0 = normalize(cross(v0, v1));
    float3 n1 = normalize(cross(v1, v2));
    float3 n2 = normalize(cross(v2, v3));
    float3 n3 = normalize(cross(v3, v0));
    float g0 = acos(dot(-n0, n1));
    float g1 = acos(dot(-n1, n2));
    float g2 = acos(dot(-n2, n3));
    float g3 = acos(dot(-n3, n0));
    return g0 + g1 + g2 + g3 - 2.0 * 3.1415926536;
}

float3 closestPointOnLine(float3 a, float3 b, float3 c)
{
    float3 ab = b - a;
    float t = dot(c - a, ab) / dot(ab, ab);
    return a + t * ab;
}

float3 closestPointOnSegment(float3 a, float3 b, float3 c)
{
    float3 ab = b - a;
    float t = dot(c - a, ab) / dot(ab, ab);
    return a + saturate(t) * ab;
}

float3 CalculatePointLight(U_Light light, Material material, float3 worldPos, float3 viewDir, float3 diffuseBRDF)
{
    float3 lightDir = normalize(light.position - worldPos);
    float3 harfDir = normalize(lightDir + viewDir);
    float NdotV = max(dot(material.normal, viewDir), 0.0);
    float NdotL = max(dot(material.normal, lightDir), 0.0);
    float NdotH = max(dot(material.normal, harfDir), 0.0);
    float HdotV = max(dot(harfDir, viewDir), 0.0);
    float distance = length(light.position - worldPos);
    float attenuation = GetDistanceAtt(distance * distance, 1.0 / (light.range * light.range));
    float3 radiance = light.color * light.intensity * 0.25 * 0.3183098862 * attenuation;
    float3 Fre = FresnelSchlick(HdotV, material.F0);
    float NDF = DistributionGGX(NdotH, material.roughness);
    float Vis = Visibility(NdotV, NdotL, material.roughness);
    float3 specularBRDF = Fre * NDF * Vis;
    float3 KD = mix(1.0 - Fre, vec3_splat(0.0), material.metallic);
    return (KD * diffuseBRDF + specularBRDF) * radiance * NdotL;
}

float3 CalculateSpotLight(U_Light light, Material material, float3 worldPos, float3 viewDir, float3 diffuseBRDF)
{
    float3 lightDir = normalize(light.position - worldPos);
    float3 harfDir = normalize(lightDir + viewDir);
    float NdotV = max(dot(material.normal, viewDir), 0.0);
    float NdotL = max(dot(material.normal, lightDir), 0.0);
    float NdotH = max(dot(material.normal, harfDir), 0.0);
    float HdotV = max(dot(harfDir, viewDir), 0.0);
    float distance = length(light.position - worldPos);
    float attenuation = GetDistanceAtt(distance * distance, 1.0 / (light.range * light.range));
    attenuation *= GetAngleAtt(lightDir, -light.direction, light.lightAngleScale, light.lightAngleOffeset);
    float3 radiance = light.color * light.intensity * 0.3183098862 * attenuation;
    float3 Fre = FresnelSchlick(HdotV, material.F0);
    float NDF = DistributionGGX(NdotH, material.roughness);
    float Vis = Visibility(NdotV, NdotL, material.roughness);
    float3 specularBRDF = Fre * NDF * Vis;
    float3 KD = mix(1.0 - Fre, vec3_splat(0.0), material.metallic);
    return (KD * diffuseBRDF + specularBRDF) * radiance * NdotL;
}

float3 CalculateDirectionalLight(U_Light light, Material material, float3 worldPos, float3 viewDir, float3 diffuseBRDF)
{
    float3 lightDir = -light.direction;
    float3 harfDir = normalize(lightDir + viewDir);
    float NdotV = max(dot(material.normal, viewDir), 0.0);
    float NdotL = max(dot(material.normal, lightDir), 0.0);
    float NdotH = max(dot(material.normal, harfDir), 0.0);
    float HdotV = max(dot(harfDir, viewDir), 0.0);
    float3 Fre = FresnelSchlick(HdotV, material.F0);
    float NDF = DistributionGGX(NdotH, material.roughness);
    float Vis = Visibility(NdotV, NdotL, material.roughness);
    float3 specularBRDF = Fre * NDF * Vis;
    float3 KD = mix(1.0 - Fre, vec3_splat(0.0), material.metallic);
    float3 irradiance = light.color * light.intensity;
    return (KD * diffuseBRDF + specularBRDF) * irradiance * NdotL;
}

float3 CalculateSphereDiffuse(U_Light light, Material material, float3 worldPos, float3 diffuseBRDF)
{
    float3 fragToLight = light.position - worldPos;
    float3 lightDir = normalize(fragToLight);
    float sqrDist = dot(fragToLight, fragToLight);
    float beta = acos(dot(material.normal, lightDir));
    float H = sqrt(sqrDist);
    float h = H / light.radius;
    float x = sqrt(h * h - 1.0);
    float y = -x * (1.0 / tan(beta));
    float formFactor = 0.0;
    if (h * cos(beta) > 1.0)
    {
        formFactor = cos(beta) / (h * h);
    }
    else
    {
        formFactor = (1.0 / (3.1415926536 * h * h)) *
                         (cos(beta) * acos(y) - x * sin(beta) * sqrt(1.0 - y * y)) +
                     (1.0 / 3.1415926536) * atan(sin(beta) * sqrt(1.0 - y * y) / x);
    }
    formFactor = saturate(formFactor);
    float3 radiance = light.color * light.intensity / (4.0 * light.radius * light.radius * 9.8696044011);
    return diffuseBRDF * radiance * 3.1415926536 * formFactor;
}

float3 CalculateSphereSpecular(U_Light light, Material material, float3 worldPos, float3 viewDir, out float3 KD)
{
    float3 reflectDir = normalize(reflect(viewDir, material.normal));
    float3 fragToLight = light.position - worldPos;
    float3 centerToRay = dot(fragToLight, reflectDir) * reflectDir - fragToLight;
    float3 fragToClosest = fragToLight + centerToRay * saturate(light.radius / length(centerToRay));
    float3 lightDir = normalize(fragToClosest);
    float3 harfDir = normalize(lightDir + viewDir);
    float distance = length(fragToClosest);
    float NdotV = max(dot(material.normal, viewDir), 0.0);
    float NdotL = max(dot(material.normal, lightDir), 0.0);
    float NdotH = max(dot(material.normal, harfDir), 0.0);
    float HdotV = max(dot(harfDir, viewDir), 0.0);
    float3 Fre = FresnelSchlick(HdotV, material.F0);
    float NDF = DistributionGGX(NdotH, material.roughness);
    float Vis = Visibility(NdotV, NdotL, material.roughness);
    float3 specularBRDF = saturate(Fre * NDF * Vis);
    KD = mix(1.0 - Fre, vec3_splat(0.0), material.metallic);
    float attenuation = GetDistanceAtt(distance * distance, 1.0 / (light.range * light.range));
    float3 radiance = light.color * light.intensity / (4.0 * light.radius * light.radius * 9.8696044011) * attenuation;
    return specularBRDF * radiance * NdotL;
}

float3 CalculateSphereLight(U_Light light, Material material, float3 worldPos, float3 viewDir, float3 diffuseBRDF)
{
    float3 color = vec3_splat(0.0);
    float3 KD = vec3_splat(1.0);
    color += CalculateSphereSpecular(light, material, worldPos, viewDir, KD);
    color += KD * CalculateSphereDiffuse(light, material, worldPos, diffuseBRDF);
    return color;
}

float3 CalculateDiskDiffuse(U_Light light, Material material, float3 worldPos, float3 diffuseBRDF)
{
    float3 fragToLight = light.position - worldPos;
    float3 lightDir = normalize(fragToLight);
    float sqrDist = dot(fragToLight, fragToLight);
    float h = length(light.position - worldPos);
    float r = light.radius;
    float theta = acos(dot(material.normal, lightDir));
    float H = h / r;
    float H2 = H * H;
    float X = pow((1.0 - H2 * cot(theta) * cot(theta)), 0.5);
    float formFactor = 0.0;
    if (theta < acot(1.0 / H))
    {
        formFactor = (1.0 / (1.0 + H2)) * cos(theta);
    }
    else
    {
        formFactor = -H * X * sin(theta) / (3.1415926536 * (1.0 + H2)) +
                     (1.0 / 3.1415926536) * atan(X * sin(theta) / H) +
                     cos(theta) * (3.1415926536 - acos(H * cot(theta))) / (3.1415926536 * (1.0 + H2));
    }
    formFactor = saturate(formFactor);
    float3 radiance = light.color * light.intensity / (light.radius * light.radius * 9.8696044011);
    return diffuseBRDF * radiance * 3.1415926536 * formFactor * saturate(dot(light.direction, -lightDir));
}

float3 CalculateDiskSpecular(U_Light light, Material material, float3 worldPos, float3 viewDir, out float3 KD)
{
    float3 reflectDir = normalize(reflect(viewDir, material.normal));
    float t = dot((light.position - worldPos), light.direction) / dot(reflectDir, light.direction);
    float3 RIntersectP = worldPos + t * reflectDir;
    float3 d0 = -light.direction;
    float3 d1 = normalize(worldPos - light.position);
    float3 d2 = reflectDir;
    float3 offset = RIntersectP - light.position;
    float r2 = light.radius * light.radius;
    float curR2 = saturate(dot(offset, offset) / r2) * r2;
    float k = -dot(d1, d0) / dot(d2, d0);
    float3 d1kd2 = d1 + k * d2;
    float a2 = curR2 / dot(d1kd2, d1kd2);
    float a = sqrt(a2);
    float3 d3 = sign(dot(-reflectDir, d0)) * a * d1kd2;
    float3 closestPoint = light.position + d3;
    float3 fragToClosest = closestPoint - worldPos;
    float3 lightDir = normalize(fragToClosest);
    float3 harfDir = normalize(lightDir + viewDir);
    float distance = length(fragToClosest);
    float NdotV = max(dot(material.normal, viewDir), 0.0);
    float NdotL = max(dot(material.normal, lightDir), 0.0);
    float NdotH = max(dot(material.normal, harfDir), 0.0);
    float HdotV = max(dot(harfDir, viewDir), 0.0);
    float3 Fre = FresnelSchlick(HdotV, material.F0);
    float NDF = DistributionGGX(NdotH, material.roughness);
    float Vis = Visibility(NdotV, NdotL, material.roughness);
    float3 specularBRDF = saturate(Fre * NDF * Vis);
    KD = mix(1.0 - Fre, vec3_splat(0.0), material.metallic);
    float attenuation = GetDistanceAtt(distance * distance, 1.0 / (light.range * light.range));
    float3 radiance = light.color * light.intensity / (light.radius * light.radius * 9.8696044011) * attenuation;
    return specularBRDF * radiance * saturate(dot(light.direction, -lightDir));
}

float3 CalculateDiskLight(U_Light light, Material material, float3 worldPos, float3 viewDir, float3 diffuseBRDF)
{
    float3 color = vec3_splat(0.0);
    float3 KD = vec3_splat(1.0);
    color += CalculateDiskSpecular(light, material, worldPos, viewDir, KD);
    color += KD * CalculateDiskDiffuse(light, material, worldPos, diffuseBRDF);
    return color;
}

float3 CalculateRectangleDiffuse(U_Light light, Material material, float3 worldPos, float3 diffuseBRDF)
{
    float3 fragToLight = light.position - worldPos;
    float3 lightDir = normalize(fragToLight);
    float sqrDist = dot(fragToLight, fragToLight);
    float3 lightLeft = normalize(cross(light.direction, light.up));
    float halfWidth = light.width * 0.5;
    float halfHeight = light.height * 0.5;
    float3 p0 = light.position + lightLeft * -halfWidth + light.up * halfHeight;
    float3 p1 = light.position + lightLeft * -halfWidth + light.up * -halfHeight;
    float3 p2 = light.position + lightLeft * halfWidth + light.up * -halfHeight;
    float3 p3 = light.position + lightLeft * halfWidth + light.up * halfHeight;
    float solidAngle = RectangleSolidAngle(worldPos, p0, p1, p2, p3);
    float averageCosine = 0.2 * (saturate(dot(normalize(p0 - worldPos), material.normal)) +
                                 saturate(dot(normalize(p1 - worldPos), material.normal)) +
                                 saturate(dot(normalize(p2 - worldPos), material.normal)) +
                                 saturate(dot(normalize(p3 - worldPos), material.normal)) +
                                 saturate(dot(normalize(light.position - worldPos), material.normal)));
    float3 radiance = light.color * light.intensity / (light.width * light.height * 3.1415926536);
    return diffuseBRDF * solidAngle * radiance * averageCosine * saturate(dot(light.direction, -lightDir));
}

float3 CalculateRectangleSpecular(U_Light light, Material material, float3 worldPos, float3 viewDir, out float3 KD)
{
    float3 reflectDir = normalize(reflect(viewDir, material.normal));
    float t = dot((light.position - worldPos), light.direction) / dot(reflectDir, light.direction);
    float3 RIntersectP = worldPos + t * reflectDir;
    float3 offset = RIntersectP - light.position;
    float offsetLen2 = dot(offset, offset);
    float3 horizontal = normalize(cross(light.direction, light.up));
    float cosTheta = dot(horizontal, offset / sqrt(offsetLen2));
    float cos2Theta = cosTheta * cosTheta;
    float tanPhi = light.height / light.width;
    float cos2Phi = 1.0 / (1.0 + tanPhi * tanPhi);
    float distToBorder2;
    if (cos2Theta > cos2Phi)
    {
        float distToBorder = light.width / 2.0 / cosTheta;
        distToBorder2 = distToBorder * distToBorder;
    }
    else
    {
        float sin2Theta = 1.0 - cos2Theta;
        distToBorder2 = light.height * light.height / 4.0 / sin2Theta;
    }
    float3 d0 = -light.direction;
    float3 d1 = -normalize(light.position - worldPos);
    float3 d2 = reflectDir;
    float curR2 = saturate(offsetLen2 / distToBorder2) * distToBorder2;
    float k = -dot(d1, d0) / dot(d2, d0);
    float3 d1kd2 = d1 + k * d2;
    float a2 = curR2 / dot(d1kd2, d1kd2);
    float a = sqrt(a2);
    float3 d3 = sign(dot(-reflectDir, d0)) * a * d1kd2;
    float3 closestPoint = light.position + d3;
    float3 fragToClosest = closestPoint - worldPos;
    float3 lightDir = normalize(fragToClosest);
    float3 harfDir = normalize(lightDir + viewDir);
    float distance = length(fragToClosest);
    float NdotV = max(dot(material.normal, viewDir), 0.0);
    float NdotL = max(dot(material.normal, lightDir), 0.0);
    float NdotH = max(dot(material.normal, harfDir), 0.0);
    float HdotV = max(dot(harfDir, viewDir), 0.0);
    float3 Fre = FresnelSchlick(HdotV, material.F0);
    float NDF = DistributionGGX(NdotH, material.roughness);
    float Vis = Visibility(NdotV, NdotL, material.roughness);
    float3 specularBRDF = saturate(Fre * NDF * Vis);
    KD = mix(1.0 - Fre, vec3_splat(0.0), material.metallic);
    float attenuation = GetDistanceAtt(distance * distance, 1.0 / (light.range * light.range));
    float3 radiance = light.color * light.intensity / (light.width * light.height * 3.1415926536) * attenuation;
    return specularBRDF * radiance * saturate(dot(light.direction, -lightDir));
}

float3 CalculateRectangleLight(U_Light light, Material material, float3 worldPos, float3 viewDir, float3 diffuseBRDF)
{
    float3 color = vec3_splat(0.0);
    float3 KD = vec3_splat(1.0);
    color += CalculateRectangleSpecular(light, material, worldPos, viewDir, KD);
    color += KD * CalculateRectangleDiffuse(light, material, worldPos, diffuseBRDF);
    return color;
}

float3 CalculateTubeDiffuse(U_Light light, Material material, float3 worldPos, float3 diffuseBRDF)
{
    float3 fragToLight = light.position - worldPos;
    float sqrDist = dot(fragToLight, fragToLight);
    float halfWidth = light.width * 0.5;
    float3 left = light.direction;
    float3 P0 = light.position - left * halfWidth;
    float3 P1 = light.position + left * halfWidth;
    float3 forward = normalize(worldPos - closestPointOnLine(P0, P1, worldPos));
    float3 up = normalize(cross(left, forward));
    float3 p0 = light.position - left * halfWidth + light.radius * up;
    float3 p1 = light.position - left * halfWidth - light.radius * up;
    float3 p2 = light.position + left * halfWidth - light.radius * up;
    float3 p3 = light.position + left * halfWidth + light.radius * up;
    float solidAngle = RectangleSolidAngle(worldPos, p0, p1, p2, p3);
    float averageCosine = 0.2 * (saturate(dot(normalize(p0 - worldPos), material.normal)) +
                                 saturate(dot(normalize(p1 - worldPos), material.normal)) +
                                 saturate(dot(normalize(p2 - worldPos), material.normal)) +
                                 saturate(dot(normalize(p3 - worldPos), material.normal)) +
                                 saturate(dot(normalize(light.position - worldPos), material.normal)));
    float3 diffuseColor = vec3_splat(0.0);
    float3 radiance = light.color * light.intensity * 0.3183098862 /
                      (2.0 * 3.1415926536 * light.radius * light.width + 4.0 * 3.1415926536 * light.radius * light.radius);
    diffuseColor += diffuseBRDF * solidAngle * radiance * averageCosine;
    float3 spherePosition = closestPointOnSegment(P0, P1, worldPos);
    float3 sphereFragToLight = spherePosition - worldPos;
    float3 sphereLightDir = normalize(sphereFragToLight);
    float sphereSqrDist = dot(sphereFragToLight, sphereFragToLight);
    float sphereFormFactor = ((light.radius * light.radius) / sphereSqrDist) *
                             saturate(dot(sphereLightDir, material.normal));
    sphereFormFactor = saturate(sphereFormFactor);
    diffuseColor += diffuseBRDF * radiance * 3.1415926536 * sphereFormFactor;
    return diffuseColor;
}

float3 CalculateTubeSpecular(U_Light light, Material material, float3 worldPos, float3 viewDir, out float3 KD)
{
    float3 reflectDir = normalize(reflect(viewDir, material.normal));
    float halfWidth = light.width * 0.5;
    float3 left = light.direction;
    float3 P0 = light.position - left * halfWidth;
    float3 P1 = light.position + left * halfWidth;
    float3 L0 = P0 - worldPos;
    float3 L1 = P1 - worldPos;
    float3 Ld = L1 - L0;
    float t = (dot(reflectDir, L0) * dot(reflectDir, Ld) - dot(L0, Ld)) /
              (dot(Ld, Ld) - pow(dot(reflectDir, Ld), 2.0));
    float3 fragToTight = L0 + saturate(t) * Ld;
    float3 lightDir = normalize(fragToTight);
    float3 harfDir = normalize(lightDir + viewDir);
    float distance = length(fragToTight);
    float NdotV = max(dot(material.normal, viewDir), 0.0);
    float NdotL = max(dot(material.normal, lightDir), 0.0);
    float NdotH = max(dot(material.normal, harfDir), 0.0);
    float HdotV = max(dot(harfDir, viewDir), 0.0);
    float3 Fre = FresnelSchlick(HdotV, material.F0);
    float NDF = DistributionGGX(NdotH, material.roughness);
    float Vis = Visibility(NdotV, NdotL, material.roughness);
    float3 specularBRDF = saturate(Fre * NDF * Vis);
    KD = mix(1.0 - Fre, vec3_splat(0.0), material.metallic);
    float attenuation = GetDistanceAtt(distance * distance, 1.0 / (light.range * light.range));
    float3 radiance = light.color * light.intensity * 0.3183098862 /
                      (2.0 * 3.1415926536 * light.radius * light.width + 4.0 * 3.1415926536 * light.radius * light.radius) * attenuation;
    return specularBRDF * radiance * NdotL;
}

float3 CalculateTubeLight(U_Light light, Material material, float3 worldPos, float3 viewDir, float3 diffuseBRDF)
{
    float3 color = vec3_splat(0.0);
    float3 KD = vec3_splat(1.0);
    color += CalculateTubeSpecular(light, material, worldPos, viewDir, KD);
    color += KD * CalculateTubeDiffuse(light, material, worldPos, diffuseBRDF);
    return max(color, vec3_splat(0.0));
}

float3 CalculateLight(U_Light light, Material material, float3 worldPos, float3 viewDir, float3 diffuseBRDF)
{
    float3 color = vec3_splat(0.0);
    switch (light.type)
    {
    case 0:
        color = CalculatePointLight(light, material, worldPos, viewDir, diffuseBRDF);
        break;
    case 1:
        color = CalculateSpotLight(light, material, worldPos, viewDir, diffuseBRDF);
        break;
    case 2:
        color = CalculateDirectionalLight(light, material, worldPos, viewDir, diffuseBRDF);
        break;
    case 3:
        color = CalculateSphereLight(light, material, worldPos, viewDir, diffuseBRDF);
        break;
    case 4:
        color = CalculateDiskLight(light, material, worldPos, viewDir, diffuseBRDF);
        break;
    case 5:
        color = CalculateRectangleLight(light, material, worldPos, viewDir, diffuseBRDF);
        break;
    case 6:
        color = CalculateTubeLight(light, material, worldPos, viewDir, diffuseBRDF);
        break;
    default:
        color = vec3_splat(0.0);
        break;
    }
    return color;
}

float3 CalculateLights(Material material, float3 worldPos, float3 viewDir, float3 diffuseBRDF)
{
    float3 color = vec3_splat(0.0);
    for (int lightIndex = 0; lightIndex < int(u_lightCount.x); ++lightIndex)
    {
        int pointer = lightIndex * u_lightStride.x;
        U_Light light = GetLightParams(pointer);
        color += CalculateLight(light, material, worldPos, viewDir, diffuseBRDF);
    }
    return color;
}

struct Varying
{
    float4 gl_FragCoord : SV_POSITION;
    float3x3 v_TBN : TEXCOORD4;
    float3 v_normal : NORMAL;
    float2 v_texcoord0 : TEXCOORD0;
    float3 v_worldPos : TEXCOORD1;
};

float4 main(Varying input) : SV_TARGET
{
    float4 bgfx_VoidFrag = 0;
    Material material = CreateMaterial();
    
    material.albedo = _AlbedoTexture.Sample(_AlbedoSampler, input.v_texcoord0);
    material.normal = normalize(_NormalTexture.Sample(_NormalSampler, input.v_texcoord0).xyz * 2.0 - 1.0);
    material.normal = normalize(mul(input.v_TBN, material.normal));

    float3 orm = _ORMTexture.Sample(_ORMSampler, input.v_texcoord0).rgb;
    orm.y = clamp(orm.y, 0.04, 1.0);

    material.roughness = orm.y;
    material.metallic = orm.z;
    material.F0 = CalcuateF0(material);

    float3 viewDir = normalize(u_params[4].xyz - input.v_worldPos);
    float3 diffuseBRDF = material.albedo * 0.3183098862;
    float NoV = saturate(dot(material.normal, viewDir));
    float mip = clamp(6.0 * material.roughness, 0.1, 6.0);
    float3 dirColor = CalculateLights(material, input.v_worldPos, viewDir, diffuseBRDF);
    float3 cubeNormalDir = normalize(fixCubeLookup(material.normal, mip, 256.0));
    float3 envIrradiance = toLinear(_CubeIrrTexture.Sample(_CubeIrrSampler, cubeNormalDir).xyz);
    float2 lut = _LUTTexture.Sample(_LUTSampler, float2(NoV, 1.0 - material.roughness)).xy;
    float3 envSpecularBRDF = (material.F0 * lut.x + lut.y);
    float3 reflectDir = normalize(reflect(-viewDir, material.normal));
    float3 cubeReflectDir = normalize(fixCubeLookup(reflectDir, mip, 256.0));
    float3 envRadiance = toLinear(_CubeTexture.SampleLevel(_CubeSampler, cubeReflectDir, mip).xyz);
    float specularOcclusion = lerp(pow(material.occlusion, 4.0), 1.0, saturate(-0.3 + NoV * NoV));
    float horizonOcclusion = saturate(1.0 + 1.2 * dot(reflectDir, input.v_normal));
    horizonOcclusion *= horizonOcclusion;
    float3 F = FresnelSchlick(NoV, material.F0);
    float3 KD = mix(1.0 - F, vec3_splat(0.0), material.metallic);
    float3 envColor = KD * material.albedo * envIrradiance * material.occlusion + envSpecularBRDF * envRadiance * min(specularOcclusion, horizonOcclusion);
    return float4(dirColor + envColor, 1.0) * float4(1, 0.15, 0.1, 1);
}
