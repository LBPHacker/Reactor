#version 450 core

layout (local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

struct ReactionHeader
{
	uint longestChain;
	uint padding1;
	uint padding2;
	uint padding3;
};
struct Reaction
{
	uint inTypes[2]; // 0 is an invalid type so it means no reaction
	uint outTypes[2];
};

uniform ivec3 simSize;
uniform uint globalLcg;
layout (std430) readonly buffer Sim
{
	uint data[];
} sim;
layout (std430) writeonly buffer Rintent
{
	uint data[];
} rintent;
layout (std430) readonly buffer Reactions
{
	ReactionHeader header;
	Reaction data[];
} reactions;

int linear(ivec3 index)
{
	return index.x + simSize.x * (index.y + simSize.y * index.z);
}

uint linear(uvec3 index)
{
	return index.x + simSize.x * (index.y + simSize.y * index.z);
}

struct LcgParameters
{
	uint multiply;
	uint add;
} lcgParameters = { 2891336453U, 1U };

uint lcgStep(uint lcg)
{
	return lcg * lcgParameters.multiply + lcgParameters.add;
}

uint lcgSkip(uint lcg, uint steps)
{
	LcgParameters lcgSkipParameters = lcgParameters;
	for (int i = 0; i < 18; ++i) // TODO: increase as necessary
	{
		if ((steps & (1U << i)) != 0U)
		{
			lcg = lcg * lcgSkipParameters.multiply + lcgSkipParameters.add;
		}
		lcgSkipParameters.add *= lcgSkipParameters.multiply + 1U;
		lcgSkipParameters.multiply *= lcgSkipParameters.multiply;
	}
	return lcg;
}

const uint fnv1a16Init = 2166136261U;
uint fnv1a16(uint h, uint d)
{
	h = (h ^ ( d        & 0xFFU)) * 16777619U;
	h = (h ^ ((d >>  8) & 0xFFU)) * 16777619U;
	h = (h ^ ((d >> 16) & 0xFFU)) * 16777619U;
	h = (h ^ ((d >> 24) & 0xFFU)) * 16777619U;
    return h;
}

uint fnv1a16Finish(uint h)
{
	return (h >> 16) ^ h;
}

bool getReaction(uint a, uint b, out uint reaction)
{
	if (a == b)
	{
		// no reactions between identical elements for now because I haven't thought of a way to
		// decide which particle should turn into which reaction output if we can't tell them apart
		return false;
	}
	if (a > b)
	{
		uint t = a;
		a = b;
		b = t;
	}
	uint reactionsSize = uint(reactions.data.length());
	uint index = fnv1a16Finish(fnv1a16(fnv1a16(fnv1a16Init, a), b));
	uint step = 0U;
	while (step < reactions.header.longestChain)
	{
		index = (index + step) % reactionsSize;
		step += 1U;
		uint[2] inTypes = reactions.data[index].inTypes;
		if (inTypes[0] == 0 && inTypes[1] == 0)
		{
			return false;
		}
		if (inTypes[0] == a && inTypes[1] == b)
		{
			reaction = index;
			return true;
		}
	}
	return false;
}

struct AvailableReaction
{
	uint reaction;
	ivec3 d;
};
void checkNeighbour(inout AvailableReaction availableReactions[6], inout uint availableReactionsMax, uint self, ivec3 pos, ivec3 d)
{
	if (d == ivec3(0))
	{
		return;
	}
	ivec3 npos = pos + d;
	if (!(all(greaterThanEqual(npos, ivec3(0))) &&
	      all(lessThan        (npos, simSize ))))
	{
		return;
	}
	uint neighbour = sim.data[linear(npos)];
	uint reaction;
	if (getReaction(self, neighbour, reaction))
	{
		availableReactions[availableReactionsMax] = AvailableReaction(reaction, d);
		availableReactionsMax += 1;
	}
}

void main()
{
	uint lcg = lcgSkip(globalLcg, linear(gl_GlobalInvocationID));
	ivec3 pos = ivec3(gl_GlobalInvocationID);
	uint self = sim.data[linear(pos)];
	AvailableReaction availableReactions[6];
	uint availableReactionsMax = 0;
	checkNeighbour(availableReactions, availableReactionsMax, self, pos, ivec3(-1,  0,  0));
	checkNeighbour(availableReactions, availableReactionsMax, self, pos, ivec3( 1,  0,  0));
	checkNeighbour(availableReactions, availableReactionsMax, self, pos, ivec3( 0, -1,  0));
	checkNeighbour(availableReactions, availableReactionsMax, self, pos, ivec3( 0,  1,  0));
	checkNeighbour(availableReactions, availableReactionsMax, self, pos, ivec3( 0,  0, -1));
	checkNeighbour(availableReactions, availableReactionsMax, self, pos, ivec3( 0,  0,  1));
	uint ri = 0;
	if (availableReactionsMax > 0)
	{
		uint divisor = (0x80000000U + availableReactionsMax - 1U) / availableReactionsMax;
		AvailableReaction ar = availableReactions[lcg / 2U / divisor];
		ri = ( uint(ar.reaction)  << 6) |
		     ((uint(ar.d.z) & 3U) << 4) |
		     ((uint(ar.d.y) & 3U) << 2) |
		      (uint(ar.d.x) & 3U)      ;
		lcg = lcgStep(lcg);
	}
	rintent.data[linear(pos)] = ri;
}
