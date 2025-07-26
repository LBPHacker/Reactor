#version 450 core

in vec2 uv;
uniform ivec3 simSize;
uniform mat3 cfu;
uniform float fov;
uniform float aspectRatio;
layout (location = 0) out vec4 color;
layout (std430) readonly buffer Sim
{
	uint data[];
} sim;
layout (std430) readonly buffer Colors
{
	vec3 data[];
} colors;

ivec3 fromBehind(vec3 pos, vec3 dir)
{
	return ivec3(mix(floor(pos), ceil(pos), lessThan(dir, vec3(0))));
}

int linear(ivec3 index)
{
	return index.x + simSize.x * (index.y + simSize.y * index.z);
}

#define AllAxes(X) \
	X(x, yz, 0) \
	X(y, xz, 1) \
	X(z, yx, 2)

vec3 hitTest(inout vec3 pos, inout ivec3 posBehind, vec3 dir)
{
	for (int i = 0; i < simSize.x + simSize.y + simSize.z; ++i)
	{
		ivec3 target = clamp(mix(posBehind + ivec3(1), posBehind - ivec3(1), lessThan(dir, vec3(0))), ivec3(0), simSize);
		vec3 dist = (target - pos) / dir;
		float minDist = 0;
		int useComponent = -1;
		vec3 newPos = vec3(0);
#define SelectAxis(a, bc, uc) \
		if ((useComponent == -1 || (!isinf(dist.a) && dist.a >= 0 && minDist > dist.a))) \
		{ \
			vec3 candidate = pos + dir * dist.a; \
			if (all(greaterThanEqual(candidate.bc, vec2(0))) && all(lessThanEqual(candidate.bc, simSize.bc))) \
			{ \
				minDist = dist.a; \
				useComponent = uc; \
				newPos = candidate; \
			} \
		}
		AllAxes(SelectAxis)
#undef SelectAxis
		if (useComponent == -1)
		{
			return vec3(0, 0, 0);
		}
		pos = newPos;
		posBehind = fromBehind(pos, dir);
#define PickBehind(a, bc, uc) \
		if (useComponent == uc) \
		{ \
			posBehind.a = target.a; \
		}
		AllAxes(PickBehind)
#undef PickBehind
		ivec3 index = mix(posBehind, posBehind - ivec3(1), lessThan(dir, vec3(0)));
		if (all(greaterThanEqual(index, ivec3(0))) && all(lessThan(index, simSize)))
		{
			uint data = sim.data[linear(index)];
			if (data != 0)
			{
				return colors.data[data % colors.data.length()];
			}
		}
	}
	return vec3(0, 0, 0);
}

void main()
{
	vec3 pos = cfu[0];
	vec3 forward = cfu[1];
	vec3 upward = cfu[2];
	vec3 rightward = cross(forward, upward);
	vec3 dir = normalize(forward + rightward * tan(fov * 0.0087266462599716) * uv.x +
	                               upward    * tan(fov * 0.0087266462599716) * uv.y / aspectRatio);

	ivec3 posBehind = fromBehind(pos, dir);
	vec3 rgb = hitTest(pos, posBehind, dir);

	color = vec4(rgb, 1);
}
