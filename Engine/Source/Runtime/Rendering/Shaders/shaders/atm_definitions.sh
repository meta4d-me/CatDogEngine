#include "../UniformDefines/AtmosphericScatteringTextureSize.sh"

#define Length                  float
#define Wavelength              float
#define Angle                   float
#define SolidAngle              float
#define Power                   float
#define LuminousPower           float

#define Number                  float
#define InverseLength           float
#define Area                    float
#define Volume                  float
#define NumberDensity           float
#define Irradiance              float
#define Radiance                float
#define SpectralPower           float
#define SpectralIrradiance      float
#define SpectralRadiance        float
#define SpectralRadianceDensity float
#define ScatteringCoefficient   float
#define InverseSolidAngle       float
#define LuminousIntensity       float
#define Luminance               float
#define Illuminance             float

// A generic function from Wavelength to some other type.
#define AbstractSpectrum        vec3
// A function from Wavelength to Number.
#define DimensionlessSpectrum   vec3
// A function from Wavelength to SpectralPower.
#define PowerSpectrum           vec3
// A function from Wavelength to SpectralIrradiance.
#define IrradianceSpectrum      vec3
// A function from Wavelength to SpectralRadiance.
#define RadianceSpectrum        vec3
// A function from Wavelength to SpectralRadianceDensity.
#define RadianceDensitySpectrum vec3
// A function from Wavelength to ScaterringCoefficient.
#define ScatteringSpectrum      vec3

// A position in 3D (3 length values).
#define Position                vec3
// A unit direction vector in 3D (3 unitless values).
#define Direction               vec3
// A vector of 3 luminance values.
#define Luminance3              vec3
// A vector of 3 illuminance values.
#define Illuminance3            vec3

static const Length m = 1.0;
static const Wavelength nm = 1.0;
static const Angle rad = 1.0;
static const SolidAngle sr = 1.0;
static const Power watt = 1.0;
static const LuminousPower lm = 1.0;

static const float PI = 3.14159265358979323846;
static const Length km = 1000.0 * m;
static const Area m2 = m * m;
static const Volume m3 = m * m * m;
static const Angle pi = PI * rad;
static const Angle deg = pi / 180.0;
static const Irradiance watt_per_square_meter = watt / m2;
static const Radiance watt_per_square_meter_per_sr = watt / (m2 * sr);
static const SpectralIrradiance watt_per_square_meter_per_nm = watt / (m2 * nm);
static const SpectralRadiance watt_per_square_meter_per_sr_per_nm = watt / (m2 * sr * nm);
static const SpectralRadianceDensity watt_per_cubic_meter_per_sr_per_nm = watt / (m3 * sr * nm);
static const LuminousIntensity cd = lm / sr;
static const LuminousIntensity kcd = 1000.0 * cd;
static const Luminance cd_per_square_meter = cd / m2;
static const Luminance kcd_per_square_meter = kcd / m2;

// An atmosphere layer of width 'width', and whose density is defined as
//   'exp_term' * exp('exp_scale' * h) + 'linear_term' * h + 'constant_term',
// clamped to [0,1], and where h is the altitude.
struct DensityProfileLayer
{
	Number width;
	Number exp_term;
	InverseLength exp_scale;
	InverseLength linear_term;
	Number constant_term;
};

// An atmosphere density profile made of several layers on top of each other
// (from bottom to top). The width of the last layer is ignored, i.e. it always
// extend to the top atmosphere boundary. The profile values vary between 0
// (null density) to 1 (maximum density).
struct DensityProfile {
	DensityProfileLayer layers[2];
};

struct AtmosphereParameters {
	// The solar irradiance at the top of the atmosphere.
	IrradianceSpectrum solar_irradiance;
	// The sun's angular radius. Warning: the implementation uses approximations
	// that are valid only if this angle is smaller than 0.1 radians.
	Angle sun_angular_radius;
	// The distance between the planet center and the bottom of the atmosphere.
	Length bottom_radius;
	// The distance between the planet center and the top of the atmosphere.
	Length top_radius;
	// The density profile of air molecules, i.e. a function from altitude to
	// dimensionless values between 0 (null density) and 1 (maximum density).
	DensityProfile rayleigh_density;
	// The scattering coefficient of air molecules at the altitude where their
	// density is maximum (usually the bottom of the atmosphere), as a function of
	// wavelength. The scattering coefficient at altitude h is equal to
	// 'rayleigh_scattering' times 'rayleigh_density' at this altitude.
	ScatteringSpectrum rayleigh_scattering;
	// The density profile of aerosols, i.e. a function from altitude to
	// dimensionless values between 0 (null density) and 1 (maximum density).
	DensityProfile mie_density;
	// The scattering coefficient of aerosols at the altitude where their density
	// is maximum (usually the bottom of the atmosphere), as a function of
	// wavelength. The scattering coefficient at altitude h is equal to
	// 'mie_scattering' times 'mie_density' at this altitude.
	ScatteringSpectrum mie_scattering;
	// The extinction coefficient of aerosols at the altitude where their density
	// is maximum (usually the bottom of the atmosphere), as a function of
	// wavelength. The extinction coefficient at altitude h is equal to
	// 'mie_extinction' times 'mie_density' at this altitude.
	ScatteringSpectrum mie_extinction;
	// The asymetry parameter for the Cornette-Shanks phase function for the
	// aerosols.
	Number mie_phase_function_g;
	// The density profile of air molecules that absorb light (e.g. ozone), i.e.
	// a function from altitude to dimensionless values between 0 (null density)
	// and 1 (maximum density).
	DensityProfile absorption_density;
	// The extinction coefficient of molecules that absorb light (e.g. ozone) at
	// the altitude where their density is maximum, as a function of wavelength.
	// The extinction coefficient at altitude h is equal to
	// 'absorption_extinction' times 'absorption_density' at this altitude.
	ScatteringSpectrum absorption_extinction;
	// The average albedo of the ground.
	DimensionlessSpectrum ground_albedo;
	// The cosine of the maximum Sun zenith angle for which atmospheric scattering
	// must be precomputed (for maximum precision, use the smallest Sun zenith
	// angle yielding negligible sky light radiance values. For instance, for the
	// Earth case, 102 degrees is a good choice - yielding mu_s_min = -0.2).
	Number mu_s_min;
};
