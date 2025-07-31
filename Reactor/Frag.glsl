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

int linear(ivec3 index)
{
	return index.x + simSize.x * (index.y + simSize.y * index.z);
}

struct WithinSim
{
	ivec3 ipos;
	float dist;
	ivec3 dirOffset;
	vec3 posDir;
};
WithinSim getWithinSim(vec3 pos, vec3 dir)
{
	ivec3 dirOffset = ivec3(lessThan(dir, ivec3(0)));
	vec3 posDir = pos / dir;
	ivec3 ipos = ivec3(floor(pos)) + dirOffset;
	float dist = 0.0;
	ivec3 inBoundsTarget = dirOffset * simSize;
	vec3 addDist = vec3(inBoundsTarget) / dir - posDir;
	vec2 v2x = (pos + addDist.x * dir).yz;
	vec2 v2y = (pos + addDist.y * dir).zx;
	vec2 v2z = (pos + addDist.z * dir).xy;
	bvec3 ok = bvec3(v2x == clamp(v2x, vec2(0), vec2(simSize.yz)),
	                 v2y == clamp(v2y, vec2(0), vec2(simSize.zx)),
	                 v2z == clamp(v2z, vec2(0), vec2(simSize.xy)));
	ivec3 indexV = ivec3(1, 2, 4) * (ivec3(not(isinf(addDist))) & ivec3(ok));
	int component = findMSB(indexV.x + indexV.y + indexV.z);
	if (component != -1)
	{
		dist = addDist[component];
		ipos = ivec3(floor(pos + dir * dist)) + dirOffset;
		ipos[component] = inBoundsTarget[component];
	}
	return WithinSim(ipos, dist, dirOffset, posDir);
}

vec3 hitTest(vec3 pos, vec3 dir)
{
	WithinSim ws = getWithinSim(pos, dir);
	ivec3 ipos      = ws.ipos;
	float dist      = ws.dist;
	ivec3 dirOffset = ws.dirOffset;
	vec3 posDir     = ws.posDir;
	ivec3 dirStep = ivec3(1) - 2 * dirOffset;
	for (int i = 0; i < simSize.x + simSize.y + simSize.z; ++i)
	{
		ivec3 index = ipos - dirOffset;
		if (!(all(greaterThanEqual(index, ivec3(0))) && all(lessThan(index, simSize))))
		{
			break;
		}
		uint data = sim.data[linear(index)];
		if (data != 0)
		{
			return colors.data[data % colors.data.length()];
		}
		ivec3 nextIpos = ipos + dirStep;
		vec3 addDist = vec3(nextIpos) / dir - posDir - vec3(dist);
		addDist *= vec3(1.0) - 2.0 * vec3(isinf(addDist));
		float minAddDist = min(min(addDist.x, addDist.y), addDist.z);
		ivec3 indexV = ivec3(1, 2, 4) * ivec3(equal(addDist, vec3(minAddDist)));
		int component = findMSB(indexV.x + indexV.y + indexV.z);
		if (component == -1)
		{
			break;
		}
		dist += addDist[component];
		ipos[component] = nextIpos[component];
	}
	return vec3(0.0, 0.0, 0.0);
}

void main()
{
	vec3 pos = cfu[0];
	vec3 forward = cfu[1];
	vec3 upward = cfu[2];
	vec3 rightward = cross(forward, upward);
	vec3 dir = normalize(forward + rightward * tan(fov * 0.0087266462599716) * uv.x +
	                               upward    * tan(fov * 0.0087266462599716) * uv.y / aspectRatio);
	color = vec4(hitTest(pos, dir), 1.0);
}
