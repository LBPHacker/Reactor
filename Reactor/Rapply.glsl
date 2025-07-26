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
layout (std430) readonly buffer Sim
{
	uint data[];
} sim;
layout (std430) readonly buffer Rintent
{
	uint data[];
} rintent;
layout (std430) writeonly buffer SimNext
{
	uint data[];
} simNext;
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

void main()
{
	ivec3 pos = ivec3(gl_GlobalInvocationID);
	uint self = sim.data[linear(pos)];
	uint rintentS = rintent.data[linear(pos)];
	if (rintentS != 0U)
	{
		ivec3 d = ivec3(int( rintentS       & 3U),
		                int((rintentS >> 2) & 3U),
		                int((rintentS >> 4) & 3U)) << 30 >> 30; // force sign extension
		Reaction reaction = reactions.data[rintentS >> 6];
		uint rintentN = rintent.data[linear(pos + d)];
		uint inverseDPacked = rintentS & 0x3FU;
		inverseDPacked ^= (inverseDPacked & 0x15U) << 1;
		if ((rintentN & 0x3FU) == inverseDPacked)
		{
			if (reaction.inTypes[0] == self)
			{
				self = reaction.outTypes[0];
			}
			else
			{
				self = reaction.outTypes[1];
			}
		}
	}
	simNext.data[linear(pos)] = self;
}
