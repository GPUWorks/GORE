#ifndef WORKOUT_MATH_H
#define WORKOUT_MATH_H

struct v2 {
	float x, y;
};

struct v3 {
	union {
		struct {
			float x, y, z;
		};

		struct {
			float r, g, b;
		};
	};
};

struct v4 {
	union {
		struct {
			float x, y, z, w;
		};

		struct {
			float r, g, b, a;
		};
	};
};

inline v2 V2(float x, float y) {
	v2 Result;

	Result.x = x;
	Result.y = y;

	return(Result);
}

inline v3 V3(float x, float y, float z) {
	v3 Result;

	Result.x = x;
	Result.y = y;
	Result.z = z;

	return(Result);
}

inline v4 V4(float x, float y, float z, float w) {
	v4 Result;

	Result.x = x;
	Result.y = y;
	Result.z = z;
	Result.w = w;

	return(Result);
}

inline float Clamp01(float Val) {
	if (Val < 0.0f) {
		Val = 0.0f;
	}

	if (Val > 1.0f) {
		Val = 1.0f;
	}

	return(Val);
}

inline u32 PackRGBA(v4 Color) {
	u32 Result;

	Result =
		((u32)(255.0f * Color.r) << 24) |
		((u32)(255.0f * Color.g) << 16) |
		((u32)(255.0f * Color.b) << 8) |
		((u32)(255.0f * Color.a));

	return(Result);
}

inline v4 UnpackRGBA(u32 Color) {
	v4 Result;

	float OneOver255 = 1.0f / 255.0f;

	Result.r = (Color >> 24) & 0xFF;
	Result.g = (Color >> 16) & 0xFF;
	Result.b = (Color >> 8) & 0xFF;
	Result.a = Color & 0xFF;

	Result.r *= OneOver255;
	Result.g *= OneOver255;
	Result.b *= OneOver255;
	Result.a *= OneOver255;

	return(Result);
}

#endif