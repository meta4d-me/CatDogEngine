#pragma once

struct EmitterShape
{
	enum Enum
	{
		Sphere,
		Hemisphere,
		Circle,
		Disc,
		Rect,

		Count
	};
};

struct EmitterDirection
{
	enum Enum
	{
		Up,
		Outward,

		Count
	};
};

static const char* s_shapeNames[] =
{
	"Sphere",
	"Hemisphere",
	"Circle",
	"Disc",
	"Rect",
};

static const char* s_directionName[] =
{
	"Up",
	"Outward",
};
