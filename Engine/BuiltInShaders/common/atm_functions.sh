/*
 * If you want to use this shader as a compute shader,
 * make sure:
 *   #include "bgfx_compute.sh"
 *   #define COMPUTE
 * before you include this file.
 */

#include "atm_definitions.sh"

#define IN(x) const in x
#define OUT(x) out x
#define TEMPLATE(x)
#define TEMPLATE_ARGUMENT(x)
#define assert(x)

// TODO : add function to generate these parameters.
static const AtmosphereParameters ATMOSPHERE = {
	vec3(10.000000, 10.000000, 10.000000),
	0.004675,
	6360.000000,
	6420.000000,
	{{0.000000, 0.000000, 0.000000, 0.000000, 0.000000},
		{0.000000, 1.000000, -0.125000, 0.000000, 0.000000}},
	vec3(0.005802, 0.013558, 0.033100),
	{{0.000000, 0.000000, 0.000000, 0.000000, 0.000000},
		{0.000000, 1.000000, -0.833333, 0.000000, 0.000000}},
	vec3(0.003996, 0.003996, 0.003996),
	vec3(0.004440, 0.004440, 0.004440),
	0.900000,
	{{25.000000, 0.000000, 0.000000, 0.066667, -0.666667},
		{0.000000, 0.000000, 0.000000, -0.066667, 2.666667}},
	vec3(0.000650, 0.001881, 0.000085),
	vec3(0.100000, 0.100000, 0.100000),
	-0.207912};

// ------------------------------ Textures ------------------------------ //

#if defined(COMPUTE)
	IMAGE2D_RO(s_textureTransmittance_LUT, rgba32f, ATM_TRANSMITTANCE_SLOT);
	IMAGE3D_RO(s_textureSingleRayleighScattering_LUT, rgba32f, ATM_SINGLE_RAYLEIGH_SCATTERING_SLOT);
	IMAGE3D_RO(s_textureSingleMieScattering_LUT, rgba32f, ATM_SINGLE_MIE_SCATTERING_SLOT);
	IMAGE3D_RO(s_textureMultipleScattering_LUT, rgba32f, ATM_MULTIPLE_SCATTERING_SLOT);
	IMAGE3D_RO(s_textureScatteringDensity_LUT, rgba32f, ATM_SCATTERING_DENSITY);
	IMAGE2D_RO(s_textureIrradiance_LUT, rgba32f, ATM_IRRADIANCE_SLOT);
	IMAGE3D_RO(s_textureScattering_LUT, rgba32f, ATM_SCATTERING_SLOT);

	ivec2 GetTransmittanceImageUV(vec2 uv) {
		return ivec2(uv * ivec2(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT));
	}
	ivec2 GetIrradianceImageUV(vec2 uv) {
		return ivec2(uv * ivec2(IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT));
	}
	ivec3 GetScatteringImageUVW(vec3 uvw) {
		return ivec3(uvw * ivec3(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH));
	}
#else
	SAMPLER2D(s_textureTransmittance_LUT, ATM_TRANSMITTANCE_SLOT);
	SAMPLER3D(s_textureSingleRayleighScattering_LUT, ATM_SINGLE_RAYLEIGH_SCATTERING_SLOT);
	SAMPLER3D(s_textureSingleMieScattering_LUT, ATM_SINGLE_MIE_SCATTERING_SLOT);
	SAMPLER3D(s_textureMultipleScattering_LUT, ATM_MULTIPLE_SCATTERING_SLOT);
	SAMPLER3D(s_textureScatteringDensity_LUT, ATM_SCATTERING_DENSITY);
	SAMPLER2D(s_textureIrradiance_LUT, ATM_IRRADIANCE_SLOT);
	SAMPLER3D(s_textureScattering_LUT, ATM_SCATTERING_SLOT);
#endif

// ------------------------------ Utils ------------------------------ //

Number ClampCosine(Number mu) {
	return clamp(mu, Number(-1.0), Number(1.0));
}
Length ClampDistance(Length d) {
	return max(d, 0.0 * m);
}
Length ClampRadius(IN(AtmosphereParameters) atmosphere, Length r) {
	return clamp(r, atmosphere.bottom_radius, atmosphere.top_radius);
}
Length SafeSqrt(Area a) {
	return sqrt(max(a, 0.0 * m2));
}

// ------------------------------ Transmittance ------------------------------ //

Length DistanceToTopAtmosphereBoundary(IN(AtmosphereParameters) atmosphere, Length r, Number mu) {
	Area discriminant = r * r * (mu * mu - 1.0) + atmosphere.top_radius * atmosphere.top_radius;
	return ClampDistance(-r * mu + SafeSqrt(discriminant));
}

Length DistanceToBottomAtmosphereBoundary(IN(AtmosphereParameters) atmosphere, Length r, Number mu) {
	Area discriminant = r * r * (mu * mu - 1.0) + atmosphere.bottom_radius * atmosphere.bottom_radius;
	return ClampDistance(-r * mu - SafeSqrt(discriminant));
}

bool RayIntersectsGround(IN(AtmosphereParameters) atmosphere, Length r, Number mu) {
	return (mu < 0.0) &&
		((r * r * (mu * mu - 1.0) + atmosphere.bottom_radius * atmosphere.bottom_radius) >= (0.0 * m2));
}

// When calculating non-ozone atmospheric particles, this equation degenerates to :
//     density = layer.linear_term * altitude + layer.constant_term;
// In other cases :
//     density = exp(layer.exp_scale * altitude);
Number GetLayerDensity(IN(DensityProfileLayer) layer, Length altitude) {
	Number density = layer.exp_term * exp(layer.exp_scale * altitude) +
		layer.linear_term * altitude + layer.constant_term;
	return clamp(density, Number(0.0), Number(1.0));
}

// When calculating non-ozone atmospheric particles, this function will never ues profile.layers[0]
Number GetProfileDensity(IN(DensityProfile) profile, Length altitude) {
	return (altitude < profile.layers[0].width) ?
		GetLayerDensity(profile.layers[0], altitude) :
		GetLayerDensity(profile.layers[1], altitude);
}

// Optical length to top
Length ComputeOpticalLengthToTopAtmosphereBoundary(
	IN(AtmosphereParameters) atmosphere, IN(DensityProfile) profile, Length r, Number mu) {
	// Number of intervals for the numerical integration.
	const int SAMPLE_COUNT = 500;
	// The integration step, i.e. the length of each integration interval.
	Length dx = DistanceToTopAtmosphereBoundary(atmosphere, r, mu) / Number(SAMPLE_COUNT);
	// Integration loop.
	Length result = 0.0 * m;
	for (int i = 0; i <= SAMPLE_COUNT; ++i) {
		Length d_i = Number(i) * dx;
		// Distance between the current sample point and the planet center.
		Length r_i = sqrt(d_i * d_i + 2.0 * r * mu * d_i + r * r);
		// Number density at the current sample point (divided by the number density
		// at the bottom of the atmosphere, yielding a dimensionless number).
		Number y_i = GetProfileDensity(profile, r_i - atmosphere.bottom_radius);
		// Sample weight (from the trapezoidal rule).
		Number weight_i = (i == 0 || i == SAMPLE_COUNT) ? 0.5 : 1.0;
		result += y_i * weight_i * dx;
	}
	return result;
}

// Transmittance to top
DimensionlessSpectrum ComputeTransmittanceToTopAtmosphereBoundary(
	IN(AtmosphereParameters) atmosphere, Length r, Number mu) {
	DimensionlessSpectrum rayleighTerm = atmosphere.rayleigh_scattering *
		ComputeOpticalLengthToTopAtmosphereBoundary(atmosphere, atmosphere.rayleigh_density, r, mu);
	DimensionlessSpectrum mieTerm = atmosphere.mie_extinction *
		ComputeOpticalLengthToTopAtmosphereBoundary(atmosphere, atmosphere.mie_density, r, mu);
	DimensionlessSpectrum ozoneTerm = atmosphere.absorption_extinction *
		ComputeOpticalLengthToTopAtmosphereBoundary(atmosphere, atmosphere.absorption_density, r, mu);
	return exp(-(rayleighTerm + mieTerm + ozoneTerm));
}

Number GetTextureCoordFromUnitRange(Number x, int texture_size) {
	return 0.5 / Number(texture_size) + x * (1.0 - 1.0 / Number(texture_size));
}

Number GetUnitRangeFromTextureCoord(Number u, int texture_size) {
	return (u - 0.5 / Number(texture_size)) / (1.0 - 1.0 / Number(texture_size));
}

// RMu -> UV
vec2 GetTransmittanceTextureUvFromRMu(IN(AtmosphereParameters) atmosphere, Length r, Number mu) {
	// Distance to top atmosphere boundary for a horizontal ray at ground level.
	Length H = sqrt(atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius);
	// Distance to the horizon.
	Length rho = SafeSqrt(r * r - atmosphere.bottom_radius * atmosphere.bottom_radius);
	// Distance to the top atmosphere boundary for the ray (r,mu), and its minimum
	// and maximum values over all mu - obtained for (r,1) and (r,mu_horizon).
	Length d = DistanceToTopAtmosphereBoundary(atmosphere, r, mu);
	Length d_min = atmosphere.top_radius - r;
	Length d_max = rho + H;
	Number x_mu = (d - d_min) / (d_max - d_min);
	Number x_r = rho / H;
	return vec2(GetTextureCoordFromUnitRange(x_mu, TRANSMITTANCE_TEXTURE_WIDTH),
		GetTextureCoordFromUnitRange(x_r, TRANSMITTANCE_TEXTURE_HEIGHT));
}

// UV -> RMu
void GetRMuFromTransmittanceTextureUv(IN(AtmosphereParameters) atmosphere, IN(vec2) uv,
	OUT(Length) r, OUT(Number) mu) {
	Number x_mu = GetUnitRangeFromTextureCoord(uv.x, TRANSMITTANCE_TEXTURE_WIDTH);
	Number x_r = GetUnitRangeFromTextureCoord(uv.y, TRANSMITTANCE_TEXTURE_HEIGHT);
	// Distance to top atmosphere boundary for a horizontal ray at ground level.
	Length H = sqrt(atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius);
	// Distance to the horizon, from which we can compute r:
	Length rho = H * x_r;
	r = sqrt(rho * rho + atmosphere.bottom_radius * atmosphere.bottom_radius);
	// Distance to the top atmosphere boundary for the ray (r,mu), and its minimum
	// and maximum values over all mu - obtained for (r,1) and (r,mu_horizon) -
	// from which we can recover mu:
	Length d_min = atmosphere.top_radius - r;
	Length d_max = rho + H;
	Length d = d_min + x_mu * (d_max - d_min);
	mu = d == 0.0 * m ? Number(1.0) : (H * H - rho * rho - d * d) / (2.0 * r * d);
	mu = ClampCosine(mu);
}

// Transmittance to Texture
DimensionlessSpectrum ComputeTransmittanceToTopAtmosphereBoundaryTexture(
	IN(AtmosphereParameters) atmosphere, IN(vec2) frag_coord) {
	const vec2 TRANSMITTANCE_TEXTURE_SIZE = vec2(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
	Length r;
	Number mu;
	GetRMuFromTransmittanceTextureUv(atmosphere, frag_coord / TRANSMITTANCE_TEXTURE_SIZE, r, mu);
	return ComputeTransmittanceToTopAtmosphereBoundary(atmosphere, r, mu);
}

// Texture to Transmittance
DimensionlessSpectrum GetTransmittanceToTopAtmosphereBoundary(
	IN(AtmosphereParameters) atmosphere, Length r, Number mu) {
	vec2 uv = GetTransmittanceTextureUvFromRMu(atmosphere, r, mu);
#if defined(COMPUTE)
	return DimensionlessSpectrum(imageLoad(s_textureTransmittance_LUT, GetTransmittanceImageUV(uv)).xyz);
#else
	return DimensionlessSpectrum(texture2D(s_textureTransmittance_LUT, uv).xyz);
#endif
}

// T(camera, point)
DimensionlessSpectrum GetTransmittance(
	IN(AtmosphereParameters) atmosphere, Length r, Number mu, Length d, bool ray_r_mu_intersects_ground) {
	Length r_d = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
	Number mu_d = ClampCosine((r * mu + d) / r_d);
	if (ray_r_mu_intersects_ground) {
		return min(
			GetTransmittanceToTopAtmosphereBoundary(atmosphere, r_d, -mu_d) /
			GetTransmittanceToTopAtmosphereBoundary(atmosphere, r, -mu),
			DimensionlessSpectrum(1.0, 1.0, 1.0));
	}
	else {
		return min(
			GetTransmittanceToTopAtmosphereBoundary(atmosphere, r, mu) /
			GetTransmittanceToTopAtmosphereBoundary(atmosphere, r_d, mu_d),
			DimensionlessSpectrum(1.0, 1.0, 1.0));
	}
}

// T(point, sun)
DimensionlessSpectrum GetTransmittanceToSun(
	IN(AtmosphereParameters) atmosphere, Length r, Number mu_s) {
	Number sin_theta_h = atmosphere.bottom_radius / r;
	Number cos_theta_h = -sqrt(max(1.0 - sin_theta_h * sin_theta_h, 0.0));
	return GetTransmittanceToTopAtmosphereBoundary(atmosphere, r, mu_s) *
		smoothstep(-sin_theta_h * atmosphere.sun_angular_radius / rad,
			sin_theta_h * atmosphere.sun_angular_radius / rad,
			mu_s - cos_theta_h);
}

// ------------------------------ Single scattering ------------------------------ //

// Omit the solar irradiance and the phase function, as well as the scattering coefficients
void ComputeSingleScatteringIntegrand(IN(AtmosphereParameters) atmosphere,
	Length r, Number mu, Number mu_s, Number nu, Length d, bool ray_r_mu_intersects_ground,
	OUT(DimensionlessSpectrum) rayleigh, OUT(DimensionlessSpectrum) mie) {
	Length r_d = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
	Number mu_s_d = ClampCosine((r * mu_s + d * nu) / r_d);
	DimensionlessSpectrum transmittance = GetTransmittanceToSun(atmosphere, r_d, mu_s_d) *
		GetTransmittance(atmosphere, r, mu, d, ray_r_mu_intersects_ground);
	rayleigh = transmittance * GetProfileDensity(atmosphere.rayleigh_density, r_d - atmosphere.bottom_radius);
	mie = transmittance * GetProfileDensity(atmosphere.mie_density, r_d - atmosphere.bottom_radius);
}

Length DistanceToNearestAtmosphereBoundary(
	IN(AtmosphereParameters) atmosphere, Length r, Number mu, bool ray_r_mu_intersects_ground) {
	if (ray_r_mu_intersects_ground) {
		return DistanceToBottomAtmosphereBoundary(atmosphere, r, mu);
	}
	else {
		return DistanceToTopAtmosphereBoundary(atmosphere, r, mu);
	}
}

// Omit the phase function
void ComputeSingleScattering(IN(AtmosphereParameters) atmosphere,
	Length r, Number mu, Number mu_s, Number nu, bool ray_r_mu_intersects_ground,
	OUT(IrradianceSpectrum) rayleigh, OUT(IrradianceSpectrum) mie) {
	// Number of intervals for the numerical integration.
	const int SAMPLE_COUNT = 50;
	// The integration step, i.e. the length of each integration interval.
	Length dx = DistanceToNearestAtmosphereBoundary(
		atmosphere, r, mu, ray_r_mu_intersects_ground) / Number(SAMPLE_COUNT);
	// Integration loop.
	DimensionlessSpectrum rayleigh_sum = DimensionlessSpectrum(0.0, 0.0, 0.0);
	DimensionlessSpectrum mie_sum = DimensionlessSpectrum(0.0, 0.0, 0.0);
	for (int i = 0; i <= SAMPLE_COUNT; ++i) {
		Length d_i = Number(i) * dx;
		// The Rayleigh and Mie single scattering at the current sample point.
		DimensionlessSpectrum rayleigh_i;
		DimensionlessSpectrum mie_i;
		ComputeSingleScatteringIntegrand(atmosphere,
			r, mu, mu_s, nu, d_i, ray_r_mu_intersects_ground,
			rayleigh_i, mie_i);
		// Sample weight (from the trapezoidal rule).
		Number weight_i = (i == 0 || i == SAMPLE_COUNT) ? 0.5 : 1.0;
		rayleigh_sum += rayleigh_i * weight_i;
		mie_sum += mie_i * weight_i;
	}
	rayleigh = rayleigh_sum * dx * atmosphere.solar_irradiance * atmosphere.rayleigh_scattering;
	mie = mie_sum * dx * atmosphere.solar_irradiance * atmosphere.mie_scattering;
}

InverseSolidAngle RayleighPhaseFunction(Number nu) {
	return 3.0 / (16.0 * PI * sr) * (1.0 + nu * nu);
}

InverseSolidAngle MiePhaseFunction(Number nu, Number g) {
	Number g2 = g * g;
	return 3.0 / (8.0 * PI * sr) * (1.0 - g2) / (2.0 + g2) * (1.0 + nu * nu) / pow(1.0 + g2 - 2.0 * g * nu, 1.5);
}

// RMuMuSNu -> UVWZ
vec4 GetScatteringTextureUvwzFromRMuMuSNu(IN(AtmosphereParameters) atmosphere,
	Length r, Number mu, Number mu_s, Number nu, bool ray_r_mu_intersects_ground) {
	// Distance to top atmosphere boundary for a horizontal ray at ground level.
	Length H = sqrt(atmosphere.top_radius * atmosphere.top_radius -
		atmosphere.bottom_radius * atmosphere.bottom_radius);
	// Distance to the horizon.
	Length rho = SafeSqrt(r * r - atmosphere.bottom_radius * atmosphere.bottom_radius);
	Number u_r = GetTextureCoordFromUnitRange(rho / H, SCATTERING_TEXTURE_R_SIZE);
	
	// Discriminant of the quadratic equation for the intersections of the ray
	// (r,mu) with the ground (see RayIntersectsGround).
	Length r_mu = r * mu;
	Area discriminant = r_mu * r_mu - r * r + atmosphere.bottom_radius * atmosphere.bottom_radius;
	Number u_mu;
	if (ray_r_mu_intersects_ground) {
		// Distance to the ground for the ray (r,mu), and its minimum and maximum
		// values over all mu - obtained for (r,-1) and (r,mu_horizon).
		Length d = -r_mu - SafeSqrt(discriminant);
		Length d_min = r - atmosphere.bottom_radius;
		Length d_max = rho;
		u_mu = 0.5 - 0.5 * GetTextureCoordFromUnitRange(
			(d_max == d_min) ? 0.0 : (d - d_min) / (d_max - d_min),
			SCATTERING_TEXTURE_MU_SIZE / 2);
	}
	else {
		// Distance to the top atmosphere boundary for the ray (r,mu), and its
		// minimum and maximum values over all mu - obtained for (r,1) and
		// (r,mu_horizon).
		Length d = -r_mu + SafeSqrt(discriminant + H * H);
		Length d_min = atmosphere.top_radius - r;
		Length d_max = rho + H;
		u_mu = 0.5 + 0.5 *
			GetTextureCoordFromUnitRange((d - d_min) / (d_max - d_min), SCATTERING_TEXTURE_MU_SIZE / 2);
	}
	
	Length d = DistanceToTopAtmosphereBoundary(atmosphere, atmosphere.bottom_radius, mu_s);
	Length d_min = atmosphere.top_radius - atmosphere.bottom_radius;
	Length d_max = H;
	Number a = (d - d_min) / (d_max - d_min);
	Length D = DistanceToTopAtmosphereBoundary(atmosphere, atmosphere.bottom_radius, atmosphere.mu_s_min);
	Number A = (D - d_min) / (d_max - d_min);
	// An ad-hoc function equal to 0 for mu_s = mu_s_min (because then d = D and
	// thus a = A), equal to 1 for mu_s = 1 (because then d = d_min and thus
	// a = 0), and with a large slope around mu_s = 0, to get more texture 
	// samples near the horizon.
	Number u_mu_s = GetTextureCoordFromUnitRange(max(1.0 - a / A, 0.0) / (1.0 + a), SCATTERING_TEXTURE_MU_S_SIZE);
	
	Number u_nu = (nu + 1.0) / 2.0;
	return vec4(u_nu, u_mu_s, u_mu, u_r);
}

// UVWZ -> RMuMuSNu
void GetRMuMuSNuFromScatteringTextureUvwz(IN(AtmosphereParameters) atmosphere, IN(vec4) uvwz,
	OUT(Length) r, OUT(Number) mu, OUT(Number) mu_s, OUT(Number) nu,
	OUT(bool) ray_r_mu_intersects_ground) {
	// Distance to top atmosphere boundary for a horizontal ray at ground level.
	Length H = sqrt(atmosphere.top_radius * atmosphere.top_radius -
		atmosphere.bottom_radius * atmosphere.bottom_radius);
	// Distance to the horizon.
	Length rho = H * GetUnitRangeFromTextureCoord(uvwz.w, SCATTERING_TEXTURE_R_SIZE);
	r = sqrt(rho * rho + atmosphere.bottom_radius * atmosphere.bottom_radius);
	
	if (uvwz.z < 0.5) {
		// Distance to the ground for the ray (r,mu), and its minimum and maximum
		// values over all mu - obtained for (r,-1) and (r,mu_horizon) - from which
		// we can recover mu:
		Length d_min = r - atmosphere.bottom_radius;
		Length d_max = rho;
		Length d = d_min + (d_max - d_min) *
			GetUnitRangeFromTextureCoord(1.0 - 2.0 * uvwz.z, SCATTERING_TEXTURE_MU_SIZE / 2);
		mu = (d == 0.0 * m) ? Number(-1.0) :
			ClampCosine(-(rho * rho + d * d) / (2.0 * r * d));
		ray_r_mu_intersects_ground = true;
	}
	else {
		// Distance to the top atmosphere boundary for the ray (r,mu), and its
		// minimum and maximum values over all mu - obtained for (r,1) and
		// (r,mu_horizon) - from which we can recover mu:
		Length d_min = atmosphere.top_radius - r;
		Length d_max = rho + H;
		Length d = d_min + (d_max - d_min) *
			GetUnitRangeFromTextureCoord(2.0 * uvwz.z - 1.0, SCATTERING_TEXTURE_MU_SIZE / 2);
		mu = (d == 0.0 * m) ? Number(1.0) :
			ClampCosine((H * H - rho * rho - d * d) / (2.0 * r * d));
		ray_r_mu_intersects_ground = false;
	}
	
	Number x_mu_s = GetUnitRangeFromTextureCoord(uvwz.y, SCATTERING_TEXTURE_MU_S_SIZE);
	Length d_min = atmosphere.top_radius - atmosphere.bottom_radius;
	Length d_max = H;
	Length D = DistanceToTopAtmosphereBoundary(atmosphere, atmosphere.bottom_radius, atmosphere.mu_s_min);
	Number A = (D - d_min) / (d_max - d_min);
	Number a = (A - x_mu_s * A) / (1.0 + x_mu_s * A);
	Length d = d_min + min(a, A) * (d_max - d_min);
	mu_s = (d == 0.0 * m) ? Number(1.0) :
		ClampCosine((H * H - d * d) / (2.0 * atmosphere.bottom_radius * d));
	
	nu = ClampCosine(uvwz.x * 2.0 - 1.0);
}

// FragCoord -> UVWZ -> RMuMuSNu
void GetRMuMuSNuFromScatteringTextureFragCoord(
	IN(AtmosphereParameters) atmosphere, IN(vec3) frag_coord,
	OUT(Length) r, OUT(Number) mu, OUT(Number) mu_s, OUT(Number) nu,
	OUT(bool) ray_r_mu_intersects_ground) {
	const vec4 SCATTERING_TEXTURE_SIZE = vec4(
		SCATTERING_TEXTURE_NU_SIZE - 1,
		SCATTERING_TEXTURE_MU_S_SIZE,
		SCATTERING_TEXTURE_MU_SIZE,
		SCATTERING_TEXTURE_R_SIZE);
	Number frag_coord_nu = floor(frag_coord.x / Number(SCATTERING_TEXTURE_MU_S_SIZE));
	Number frag_coord_mu_s = mod(frag_coord.x, Number(SCATTERING_TEXTURE_MU_S_SIZE));
	vec4 uvwz = vec4(frag_coord_nu, frag_coord_mu_s, frag_coord.y, frag_coord.z) / SCATTERING_TEXTURE_SIZE;
	GetRMuMuSNuFromScatteringTextureUvwz(atmosphere, uvwz, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
	// Clamp nu to its valid range of values, given mu and mu_s.
	nu = clamp(nu, mu * mu_s - sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)),
		mu * mu_s + sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)));
}

// Single Scattering to Texture
void ComputeSingleScatteringTexture(IN(AtmosphereParameters) atmosphere, IN(vec3) frag_coord,
	OUT(IrradianceSpectrum) rayleigh, OUT(IrradianceSpectrum) mie) {
	Length r;
	Number mu;
	Number mu_s;
	Number nu;
	bool ray_r_mu_intersects_ground;
	GetRMuMuSNuFromScatteringTextureFragCoord(atmosphere, frag_coord, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
	ComputeSingleScattering(atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground, rayleigh, mie);
}

// We need this interpolation to sample a actually 4DTexture in a 3DTexture data structure.
void Get4DTextureSampleFunction(IN(AtmosphereParameters) atmosphere,
	Length r, Number mu, Number mu_s, Number nu, bool ray_r_mu_intersects_ground,
	OUT(Number) lerp, OUT(vec3) uvw0, OUT(vec3) uvw1) {
	vec4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu(atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
	Number tex_coord_x = uvwz.x * Number(SCATTERING_TEXTURE_NU_SIZE - 1);
	Number tex_x = floor(tex_coord_x);
	lerp = tex_coord_x - tex_x;
	uvw0 = vec3((tex_x + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE), uvwz.z, uvwz.w);
	uvw1 = vec3((tex_x + 1.0 + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE), uvwz.z, uvwz.w);
}

// Texture to Single Rayleigh Scattering
AbstractSpectrum GetSingleRayleighScattering(IN(AtmosphereParameters) atmosphere,
	Length r, Number mu, Number mu_s, Number nu, bool ray_r_mu_intersects_ground) {
	Number lerp;
	vec3 uvw0;
	vec3 uvw1;
	Get4DTextureSampleFunction(atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground, lerp, uvw0, uvw1);
	
#if defined(COMPUTE)
	return AbstractSpectrum(imageLoad(s_textureSingleRayleighScattering_LUT, GetScatteringImageUVW(uvw0)).xyz * (1.0 - lerp) +
		imageLoad(s_textureSingleRayleighScattering_LUT, GetScatteringImageUVW(uvw1)).xyz * lerp);
#else
	return AbstractSpectrum(texture3D(s_textureSingleRayleighScattering_LUT, uvw0).xyz * (1.0 - lerp) +
		texture3D(s_textureSingleRayleighScattering_LUT, uvw1).xyz * lerp);
#endif
}

// Texture to Single Mie Scattering
AbstractSpectrum GetSingleMieScattering(IN(AtmosphereParameters) atmosphere,
	Length r, Number mu, Number mu_s, Number nu, bool ray_r_mu_intersects_ground) {
	Number lerp;
	vec3 uvw0;
	vec3 uvw1;
	Get4DTextureSampleFunction(atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground, lerp, uvw0, uvw1);
	
#if defined(COMPUTE)
	return AbstractSpectrum(imageLoad(s_textureSingleMieScattering_LUT, GetScatteringImageUVW(uvw0)).xyz * (1.0 - lerp) +
		imageLoad(s_textureSingleMieScattering_LUT, GetScatteringImageUVW(uvw1)).xyz * lerp);
#else
	return AbstractSpectrum(texture3D(s_textureSingleMieScattering_LUT, uvw0).xyz * (1.0 - lerp) +
		texture3D(s_textureSingleMieScattering_LUT, uvw1).xyz * lerp);
#endif
}

// Texture to Multiple Scattering
AbstractSpectrum GetMultipleScattering(IN(AtmosphereParameters) atmosphere,
	Length r, Number mu, Number mu_s, Number nu, bool ray_r_mu_intersects_ground) {
	Number lerp;
	vec3 uvw0;
	vec3 uvw1;
	Get4DTextureSampleFunction(atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground, lerp, uvw0, uvw1);
	
#if defined(COMPUTE)
	return AbstractSpectrum(imageLoad(s_textureMultipleScattering_LUT, GetScatteringImageUVW(uvw0)).xyz * (1.0 - lerp) +
		imageLoad(s_textureMultipleScattering_LUT, GetScatteringImageUVW(uvw1)).xyz * lerp);
#else
	return AbstractSpectrum(texture3D(s_textureMultipleScattering_LUT, uvw0).xyz * (1.0 - lerp) +
		texture3D(s_textureMultipleScattering_LUT, uvw1).xyz * lerp);
#endif
}

// Get n-th order Scattering
RadianceSpectrum GetScattering(IN(AtmosphereParameters) atmosphere,
	Length r, Number mu, Number mu_s, Number nu,
	bool ray_r_mu_intersects_ground, int scattering_order) {
	if (scattering_order == 1) {
		IrradianceSpectrum rayleigh = GetSingleRayleighScattering(atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
		IrradianceSpectrum mie = GetSingleMieScattering(atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
		return rayleigh * RayleighPhaseFunction(nu) + mie * MiePhaseFunction(nu, atmosphere.mie_phase_function_g);
	}
	else {
		return GetMultipleScattering(atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
	}
}

// ------------------------------ Ground irradiance ------------------------------ //

// Direct Irradiance is the integral over the Sun disc of that
// Sun radiance at the top of the atmosphere, times the Transmittance through the atmosphere
// Since the Sun solid angle is small, we can approximate the Transmittance with a constant
// i.e. move it outside the irradiance integral
IrradianceSpectrum ComputeDirectIrradiance(IN(AtmosphereParameters) atmosphere, Length r, Number mu_s) {
	Number alpha_s = atmosphere.sun_angular_radius / rad;
	// Approximate average of the cosine factor mu_s over the visible fraction of the Sun disc.
	Number average_cosine_factor = (mu_s < -alpha_s) ? 0.0 :
		((mu_s > alpha_s) ? mu_s :
			(mu_s + alpha_s) * (mu_s + alpha_s) / (4.0 * alpha_s));
	return average_cosine_factor * atmosphere.solar_irradiance *
		GetTransmittanceToTopAtmosphereBoundary(atmosphere, r, mu_s);
}

// Compute the integral over all the directions omega of the hemisphere, of the product of:
//   Radiance arriving from direction omega after n bounces,
//   Cosine factor, i.e. omega.z
IrradianceSpectrum ComputeIndirectIrradiance(IN(AtmosphereParameters) atmosphere, Length r, Number mu_s, int scattering_order) {
	const int SAMPLE_COUNT = 32;
	const Angle dphi = pi / Number(SAMPLE_COUNT);
	const Angle dtheta = pi / Number(SAMPLE_COUNT);
	IrradianceSpectrum result = IrradianceSpectrum(0.0, 0.0, 0.0) * watt_per_square_meter_per_nm;
	vec3 omega_s = vec3(sqrt(1.0 - mu_s * mu_s), 0.0, mu_s);
	for (int j = 0; j < SAMPLE_COUNT / 2; ++j) {
		Angle theta = (Number(j) + 0.5) * dtheta;
		for (int i = 0; i < 2 * SAMPLE_COUNT; ++i) {
			Angle phi = (Number(i) + 0.5) * dphi;
			vec3 omega = vec3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
			SolidAngle domega = (dtheta / rad) * (dphi / rad) * sin(theta) * sr;
			Number nu = dot(omega, omega_s);
			result += GetScattering(atmosphere, r, omega.z, mu_s, nu, false, scattering_order) * omega.z * domega;
		}
	}
	return result;
}

// RMuS -> UV
vec2 GetIrradianceTextureUvFromRMuS(IN(AtmosphereParameters) atmosphere, Length r, Number mu_s) {
	Number x_r = (r - atmosphere.bottom_radius) / (atmosphere.top_radius - atmosphere.bottom_radius);
	Number x_mu_s = mu_s * 0.5 + 0.5;
	return vec2(GetTextureCoordFromUnitRange(x_mu_s, IRRADIANCE_TEXTURE_WIDTH),
		GetTextureCoordFromUnitRange(x_r, IRRADIANCE_TEXTURE_HEIGHT));
}

// UV -> RMuS
void GetRMuSFromIrradianceTextureUv(IN(AtmosphereParameters) atmosphere, IN(vec2) uv, OUT(Length) r, OUT(Number) mu_s) {
	Number x_mu_s = GetUnitRangeFromTextureCoord(uv.x, IRRADIANCE_TEXTURE_WIDTH);
	Number x_r = GetUnitRangeFromTextureCoord(uv.y, IRRADIANCE_TEXTURE_HEIGHT);
	r = atmosphere.bottom_radius + x_r * (atmosphere.top_radius - atmosphere.bottom_radius);
	mu_s = ClampCosine(2.0 * x_mu_s - 1.0);
}

static const vec2 IRRADIANCE_TEXTURE_SIZE = vec2(IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);

// Direct Irradiance -> Texture
IrradianceSpectrum ComputeDirectIrradianceTexture(IN(AtmosphereParameters) atmosphere, IN(vec2) frag_coord) {
	Length r;
	Number mu_s;
	GetRMuSFromIrradianceTextureUv(atmosphere, frag_coord / IRRADIANCE_TEXTURE_SIZE, r, mu_s);
	return ComputeDirectIrradiance(atmosphere, r, mu_s);
}

// Indirect Irradiance -> Texture
IrradianceSpectrum ComputeIndirectIrradianceTexture(IN(AtmosphereParameters) atmosphere, IN(vec2) frag_coord, int scattering_order) {
	Length r;
	Number mu_s;
	GetRMuSFromIrradianceTextureUv(atmosphere, frag_coord / IRRADIANCE_TEXTURE_SIZE, r, mu_s);
	return ComputeIndirectIrradiance(atmosphere, r, mu_s, scattering_order);
}

// Texture -> Ground Irradiance
IrradianceSpectrum GetIrradiance(IN(AtmosphereParameters) atmosphere, Length r, Number mu_s) {
	vec2 uv = GetIrradianceTextureUvFromRMuS(atmosphere, r, mu_s);
#if defined(COMPUTE)
	return IrradianceSpectrum(imageLoad(s_textureIrradiance_LUT, GetIrradianceImageUV(uv)).xyz);
#else
	return IrradianceSpectrum(texture2D(s_textureIrradiance_LUT, uv).xyz);
#endif
}

// ------------------------------ Multiple scattering ------------------------------ //

// Computes the Radiance which is scattered at some point q inside the atmosphere, towards some direction −omega.
// This Radiance is the integral over all the possible incident directions omega_i of the product of:
//   the incident Radiance arriving at q from direction omega_i after n−1 bounces, which is the sum of:
//     a term given by the precomputed scattering texture for the (n−1)-th order,
//     If the ray (q, omega_i) intersects the ground at r, this contribution is the product of:
//       Transmittance(q, r),
//       ground Albedo,
//       Lambertian BRDF,
//       Irradiance received on the ground after n−2 bounces.
//   the Scattering Coefficient at q,
//   the Scattering Phase function for the directions omega and omega_i.
RadianceDensitySpectrum ComputeScatteringDensity(IN(AtmosphereParameters) atmosphere,
	Length r, Number mu, Number mu_s, Number nu, int scattering_order) {
	
	// Compute unit direction vectors for the zenith, the view direction omega and
	// and the sun direction omega_s, such that the cosine of the view-zenith
	// angle is mu, the cosine of the sun-zenith angle is mu_s, and the cosine of
	// the view-sun angle is nu. The goal is to simplify computations below.
	vec3 zenith_direction = vec3(0.0, 0.0, 1.0);
	vec3 omega = vec3(sqrt(1.0 - mu * mu), 0.0, mu);
	Number sun_dir_x = (omega.x == 0.0) ? 0.0 : (nu - mu * mu_s) / omega.x;
	Number sun_dir_y = sqrt(max(1.0 - sun_dir_x * sun_dir_x - mu_s * mu_s, 0.0));
	vec3 omega_s = vec3(sun_dir_x, sun_dir_y, mu_s);
	
	const int SAMPLE_COUNT = 16;
	const Angle dphi = pi / Number(SAMPLE_COUNT);
	const Angle dtheta = pi / Number(SAMPLE_COUNT);
	RadianceDensitySpectrum rayleigh_mie = RadianceDensitySpectrum(0.0, 0.0, 0.0) * watt_per_cubic_meter_per_sr_per_nm;
	
	// Nested loops for the integral over all the incident directions omega_i.
	for (int l = 0; l < SAMPLE_COUNT; ++l) {
		Angle theta = (Number(l) + 0.5) * dtheta;
		Number cos_theta = cos(theta);
		Number sin_theta = sin(theta);
		bool ray_r_theta_intersects_ground = RayIntersectsGround(atmosphere, r, cos_theta);
		
		// The distance and transmittance to the ground only depend on theta, so we
		// can compute them in the outer loop for efficiency.
		Length distance_to_ground = 0.0 * m;
		DimensionlessSpectrum transmittance_to_ground = DimensionlessSpectrum(0.0, 0.0, 0.0);
		DimensionlessSpectrum ground_albedo = DimensionlessSpectrum(0.0, 0.0, 0.0);
		if (ray_r_theta_intersects_ground) {
			distance_to_ground = DistanceToBottomAtmosphereBoundary(atmosphere, r, cos_theta);
			transmittance_to_ground = GetTransmittance(atmosphere, r, cos_theta, distance_to_ground, true);
			ground_albedo = atmosphere.ground_albedo;
		}
		
		for (int m = 0; m < 2 * SAMPLE_COUNT; ++m) {
			Angle phi = (Number(m) + 0.5) * dphi;
			vec3 omega_i = vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
			SolidAngle domega_i = (dtheta / rad) * (dphi / rad) * sin(theta) * sr;
			
			// The radiance L_i arriving from direction omega_i after n-1 bounces is
			// the sum of a term given by the precomputed scattering texture for the
			// (n-1)-th order:
			Number nu1 = dot(omega_s, omega_i);
			RadianceSpectrum incident_radiance = GetScattering(atmosphere,
				r, omega_i.z, mu_s, nu1,
				ray_r_theta_intersects_ground, scattering_order - 1);
		
			// and of the contribution from the light paths with n-1 bounces and whose
			// last bounce is on the ground. This contribution is the product of the
			// transmittance to the ground, the ground albedo, the ground BRDF, and
			// the irradiance received on the ground after n-2 bounces.
			vec3 ground_normal = normalize(zenith_direction * r + omega_i * distance_to_ground);
			IrradianceSpectrum ground_irradiance = GetIrradiance(atmosphere, atmosphere.bottom_radius, dot(ground_normal, omega_s));
			incident_radiance += transmittance_to_ground * ground_albedo * (1.0 / (PI * sr)) * ground_irradiance;
		
			// The radiance finally scattered from direction omega_i towards direction
			// -omega is the product of the incident radiance, the scattering
			// coefficient, and the phase function for directions omega and omega_i
			// (all this summed over all particle types, i.e. Rayleigh and Mie).
			Number nu2 = dot(omega, omega_i);
			Number rayleigh_density = GetProfileDensity(atmosphere.rayleigh_density, r - atmosphere.bottom_radius);
			Number mie_density = GetProfileDensity(atmosphere.mie_density, r - atmosphere.bottom_radius);
			rayleigh_mie +=
				(atmosphere.rayleigh_scattering * rayleigh_density * RayleighPhaseFunction(nu2) +
					atmosphere.mie_scattering * mie_density * MiePhaseFunction(nu2, atmosphere.mie_phase_function_g)) *
					incident_radiance * domega_i;
		}
	}
	return rayleigh_mie;
}

// Texture to Scattering Density
AbstractSpectrum GetScatteringDensity(IN(AtmosphereParameters) atmosphere,
	Length r, Number mu, Number mu_s, Number nu, bool ray_r_mu_intersects_ground) {
	Number lerp;
	vec3 uvw0;
	vec3 uvw1;
	Get4DTextureSampleFunction(atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground, lerp, uvw0, uvw1);
#if defined(COMPUTE)
	return AbstractSpectrum(imageLoad(s_textureScatteringDensity_LUT, GetScatteringImageUVW(uvw0)).xyz * (1.0 - lerp) +
		imageLoad(s_textureScatteringDensity_LUT, GetScatteringImageUVW(uvw1)).xyz * lerp);
#else
	return AbstractSpectrum(texture3D(s_textureScatteringDensity_LUT, uvw0).xyz * (1.0 - lerp) +
		texture3D(s_textureScatteringDensity_LUT, uvw1).xyz * lerp);
#endif
}

// Compute the Radiance coming from direction omega after n bounces for each point p.
// This Radiance is the integral over all points q between p and the nearest atmosphere boundary in direction omega of the product of:
//   Radiance scattered at q towards p, coming from any direction after n−1 bounces(given by a texture precomputed with the previous function),
//   Transmittance(p, q).
RadianceSpectrum ComputeMultipleScattering(IN(AtmosphereParameters) atmosphere,
	Length r, Number mu, Number mu_s, Number nu, bool ray_r_mu_intersects_ground) {
	// Number of intervals for the numerical integration.
	const int SAMPLE_COUNT = 50;
	// The integration step, i.e. the length of each integration interval.
	Length dx = DistanceToNearestAtmosphereBoundary(atmosphere, r, mu, ray_r_mu_intersects_ground) / Number(SAMPLE_COUNT);
	// Integration loop.
	RadianceSpectrum rayleigh_mie_sum = RadianceSpectrum(0.0, 0.0, 0.0) * watt_per_square_meter_per_sr_per_nm;
	for (int i = 0; i <= SAMPLE_COUNT; ++i) {
		Length d_i = Number(i) * dx;
		
		// The r, mu and mu_s parameters at the current integration point (see the
		// single scattering section for a detailed explanation).
		Length r_i = ClampRadius(atmosphere, sqrt(d_i * d_i + 2.0 * r * mu * d_i + r * r));
		Number mu_i = ClampCosine((r * mu + d_i) / r_i);
		Number mu_s_i = ClampCosine((r * mu_s + d_i * nu) / r_i);
		
		// The Rayleigh and Mie multiple scattering at the current sample point.
		RadianceSpectrum rayleigh_mie_i = dx *
			GetScatteringDensity(atmosphere, r_i, mu_i, mu_s_i, nu, ray_r_mu_intersects_ground) *
			GetTransmittance(atmosphere, r, mu, d_i, ray_r_mu_intersects_ground);
		// Sample weight (from the trapezoidal rule).
		Number weight_i = (i == 0 || i == SAMPLE_COUNT) ? 0.5 : 1.0;
		rayleigh_mie_sum += rayleigh_mie_i * weight_i;
	}
	return rayleigh_mie_sum;
}

// Scattering Density -> Texture
RadianceDensitySpectrum ComputeScatteringDensityTexture(
	IN(AtmosphereParameters) atmosphere, IN(vec3) frag_coord, int scattering_order) {
	Length r;
	Number mu;
	Number mu_s;
	Number nu;
	bool ray_r_mu_intersects_ground;
	GetRMuMuSNuFromScatteringTextureFragCoord(atmosphere, frag_coord, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
	return ComputeScatteringDensity(atmosphere, r, mu, mu_s, nu, scattering_order);
}

// Multiple Scattering -> Texture
RadianceSpectrum ComputeMultipleScatteringTexture(
	IN(AtmosphereParameters) atmosphere, IN(vec3) frag_coord, OUT(Number) nu) {
	Length r;
	Number mu;
	Number mu_s;
	bool ray_r_mu_intersects_ground;
	GetRMuMuSNuFromScatteringTextureFragCoord(atmosphere, frag_coord, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
	return ComputeMultipleScattering(atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
}

// ------------------------------ Rendering ------------------------------ //

// Here we assume that the transmittance, scattering and irradiance textures have been precomputed.
// More precisely, we assume that the single Rayleigh scattering, without its phase function term,
// plus the multiple scattering terms (divided by the Rayleigh phase function for dimensional homogeneity)
// are stored in a scattering_texture. 
// We also assume that the single Mie scattering is stored, without its phase function term, in the scattering_texture.w. 

// Rayleigh and multiple scattering are stored in the RGB channels,
// and the red component of the single Mie scattering is stored in the alpha channel.
IrradianceSpectrum GetExtrapolatedSingleMieScattering(IN(AtmosphereParameters) atmosphere, IN(vec4) scattering) {
	// Algebraically this can never be negative, but rounding errors can produce that effect for sufficiently short view rays.
	if (scattering.x <= 0.0) {
		return vec3(0.0, 0.0, 0.0);
	}
	return scattering.xyz * scattering.w / scattering.x *
		(atmosphere.rayleigh_scattering.x / atmosphere.mie_scattering.x) *
		(atmosphere.mie_scattering / atmosphere.rayleigh_scattering);
}

// Texture -> (Rayleigh plus multiple Scattering) and (single Mie Scattering).
IrradianceSpectrum GetCombinedScattering(IN(AtmosphereParameters) atmosphere,
	Length r, Number mu, Number mu_s, Number nu, bool ray_r_mu_intersects_ground,
	OUT(IrradianceSpectrum) single_mie_scattering) {
	
	// Texture coordinates computation is shared between 2 lookups.
	vec4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu(atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
	Number tex_coord_x = uvwz.x * Number(SCATTERING_TEXTURE_NU_SIZE - 1);
	Number tex_x = floor(tex_coord_x);
	Number lerp = tex_coord_x - tex_x;
	vec3 uvw0 = vec3((tex_x + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE), uvwz.z, uvwz.w);
	vec3 uvw1 = vec3((tex_x + 1.0 + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE), uvwz.z, uvwz.w);
	
#if defined(COMPUTE)
		vec4 combined_scattering =
			imageLoad(s_textureScattering_LUT, GetScatteringImageUVW(uvw0)).xyzw * (1.0 - lerp) +
			imageLoad(s_textureScattering_LUT, GetScatteringImageUVW(uvw1)).xyzw * lerp;
#else
		vec4 combined_scattering =
			texture3D(s_textureScattering_LUT, uvw0).xyzw * (1.0 - lerp) +
			texture3D(s_textureScattering_LUT, uvw1).xyzw * lerp;
#endif

	IrradianceSpectrum scattering = IrradianceSpectrum(combined_scattering.xyz);
	single_mie_scattering = GetExtrapolatedSingleMieScattering(atmosphere, combined_scattering);
	
	return scattering;
}

// The sky Radiance is that precomputed Scattering multiplied by the Phase function.
// Also return Transmittance to render objects in spase.
RadianceSpectrum GetSkyRadiance(IN(AtmosphereParameters) atmosphere,
	Position camera, IN(Direction) view_ray, Length shadow_length,
	IN(Direction) sun_direction, OUT(DimensionlessSpectrum) transmittance) {
	// Compute the distance to the top atmosphere boundary along the view ray,
	// assuming the viewer is in space (or NaN if the view ray does not intersect the atmosphere).
	Length r = length(camera);
	Length rmu = dot(camera, view_ray);
	Length distance_to_top_atmosphere_boundary = -rmu - sqrt(rmu * rmu - r * r + atmosphere.top_radius * atmosphere.top_radius);
	// If the viewer is in space and the view ray intersects the atmosphere,
	// move the viewer to the top atmosphere boundary (along the view ray):
	if (distance_to_top_atmosphere_boundary > 0.0 * m) {
		camera = camera + view_ray * distance_to_top_atmosphere_boundary;
		r = atmosphere.top_radius;
		rmu += distance_to_top_atmosphere_boundary;
	}
	else if (r > atmosphere.top_radius) {
		// If the view ray does not intersect the atmosphere, simply return 0.
		transmittance = DimensionlessSpectrum(1.0, 1.0, 1.0);
		return RadianceSpectrum(0.0, 0.0, 0.0) * watt_per_square_meter_per_sr_per_nm;
	}
	// Compute the r, mu, mu_s and nu parameters needed for the texture lookups.
	Number mu = rmu / r;
	Number mu_s = dot(camera, sun_direction) / r;
	Number nu = dot(view_ray, sun_direction);
	bool ray_r_mu_intersects_ground = RayIntersectsGround(atmosphere, r, mu);
	
	transmittance = ray_r_mu_intersects_ground ? DimensionlessSpectrum(0.0, 0.0, 0.0) :
		GetTransmittanceToTopAtmosphereBoundary(atmosphere, r, mu);
	IrradianceSpectrum single_mie_scattering;
	IrradianceSpectrum scattering;
	if (shadow_length == 0.0 * m) {
		scattering = GetCombinedScattering(atmosphere, r, mu, mu_s, nu,
			ray_r_mu_intersects_ground, single_mie_scattering);
	}
	else {
		// Case of light shafts (shadow_length is the total length noted l in our
		// paper): we omit the scattering between the camera and the point at
		// distance l, by implementing Eq. (18) of the paper (shadow_transmittance
		// is the T(x,x_s) term, scattering is the S|x_s=x+lv term).
		Length d = shadow_length;
		Length r_p = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
		Number mu_p = (r * mu + d) / r_p;
		Number mu_s_p = (r * mu_s + d * nu) / r_p;
		
		scattering = GetCombinedScattering(atmosphere, r_p, mu_p, mu_s_p, nu,
			ray_r_mu_intersects_ground, single_mie_scattering);
		DimensionlessSpectrum shadow_transmittance = GetTransmittance(atmosphere,
				r, mu, shadow_length, ray_r_mu_intersects_ground);
		scattering = scattering * shadow_transmittance;
		single_mie_scattering = single_mie_scattering * shadow_transmittance;
	}
	
	return scattering * RayleighPhaseFunction(nu) +
		single_mie_scattering * MiePhaseFunction(nu, atmosphere.mie_phase_function_g);
}

// Aerial perspective
// To get Aerial perspective, we need both Transmittance and Scattering betweeen 2 points.
// 2 lookpus then subtracted the results to get the Scattering between 2 points.
RadianceSpectrum GetSkyRadianceToPoint(IN(AtmosphereParameters) atmosphere,
	Position camera, IN(Position) position, Length shadow_length,
	IN(Direction) sun_direction, OUT(DimensionlessSpectrum) transmittance) {
	// Compute the distance to the top atmosphere boundary along the view ray,
	// assuming the viewer is in space (or NaN if the view ray does not intersect the atmosphere).
	Direction view_ray = normalize(position - camera);
	Length r = length(camera);
	Length rmu = dot(camera, view_ray);
	Length distance_to_top_atmosphere_boundary = -rmu - sqrt(rmu * rmu - r * r + atmosphere.top_radius * atmosphere.top_radius);
	// If the viewer is in space and the view ray intersects the atmosphere,
	// move the viewer to the top atmosphere boundary (along the view ray):
	if (distance_to_top_atmosphere_boundary > 0.0 * m) {
		camera = camera + view_ray * distance_to_top_atmosphere_boundary;
		r = atmosphere.top_radius;
		rmu += distance_to_top_atmosphere_boundary;
	}
	
	// Compute the r, mu, mu_s and nu parameters for the first texture lookup.
	Number mu = rmu / r;
	Number mu_s = dot(camera, sun_direction) / r;
	Number nu = dot(view_ray, sun_direction);
	Length d = length(position - camera);
	bool ray_r_mu_intersects_ground = RayIntersectsGround(atmosphere, r, mu);
	
	transmittance = GetTransmittance(atmosphere, r, mu, d, ray_r_mu_intersects_ground);
	
	IrradianceSpectrum single_mie_scattering;
	IrradianceSpectrum scattering = GetCombinedScattering(atmosphere, r, mu, mu_s, nu,
		ray_r_mu_intersects_ground, single_mie_scattering);
	
	// Compute the r, mu, mu_s and nu parameters for the second texture lookup.
	// If shadow_length is not 0 (case of light shafts), we want to ignore the
	// scattering along the last shadow_length meters of the view ray, which we
	// do by subtracting shadow_length from d (this way scattering_p is equal to
	// the S|x_s=x_0-lv term in Eq. (17) of our paper).
	d = max(d - shadow_length, 0.0 * m);
	Length r_p = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
	Number mu_p = (r * mu + d) / r_p;
	Number mu_s_p = (r * mu_s + d * nu) / r_p;
	
	IrradianceSpectrum single_mie_scattering_p;
	IrradianceSpectrum scattering_p = GetCombinedScattering(atmosphere, r_p, mu_p, mu_s_p, nu,
		ray_r_mu_intersects_ground, single_mie_scattering_p);
	
	// Combine the lookup results to get the scattering between camera and point.
	DimensionlessSpectrum shadow_transmittance = transmittance;
	if (shadow_length > 0.0 * m) {
		// This is the T(x,x_s) term in Eq. (17) of our paper, for light shafts.
		shadow_transmittance = GetTransmittance(atmosphere, r, mu, d, ray_r_mu_intersects_ground);
	}
	// Subtracted here.
	scattering = scattering - shadow_transmittance * scattering_p;
	single_mie_scattering = single_mie_scattering - shadow_transmittance * single_mie_scattering_p;
	single_mie_scattering = GetExtrapolatedSingleMieScattering(atmosphere, vec4(scattering, single_mie_scattering.r));
	// Hack to avoid rendering artifacts when the sun is below the horizon.
	single_mie_scattering = single_mie_scattering * smoothstep(Number(0.0), Number(0.01), mu_s);
	return scattering * RayleighPhaseFunction(nu) +
		single_mie_scattering * MiePhaseFunction(nu, atmosphere.mie_phase_function_g);
	//return scattering;
}

// Ground Irradiance
// Direct Irradiance can be compute by solar Irradiance * T(point to sun)
// Indirect Irradiance is given by precomputed texture.
IrradianceSpectrum GetSunAndSkyIrradiance(IN(AtmosphereParameters) atmosphere,
	IN(Position) position, IN(Direction) normal, IN(Direction) sun_direction,
	OUT(IrradianceSpectrum) sky_irradiance) {
	Length r = length(position);
	Number mu_s = dot(position, sun_direction) / r;
	// Indirect irradiance (approximated if the surface is not horizontal).
	sky_irradiance = GetIrradiance(atmosphere, r, mu_s) * (1.0 + dot(normal, position) / r) * 0.5;
	// Direct irradiance.
	return atmosphere.solar_irradiance * GetTransmittanceToSun(atmosphere, r, mu_s) * max(dot(normal, sun_direction), 0.0);
}
