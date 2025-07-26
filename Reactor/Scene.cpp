#include "Scene.hpp"
#include "Common/Point.hpp"
#include "Common/Log.hpp"
#include "Gl/VertexArray.hpp"
#include "Gl/Buffer.hpp"
#include "Gl/Program.hpp"
#include "Gl/ProgramBindings.hpp"
#include "Gl/Shader.hpp"
#include "Vert.glsl.hpp"
#include "Frag.glsl.hpp"
#include "Rintent.glsl.hpp"
#include "Rapply.glsl.hpp"
#include <array>
#include <iostream>
#include <cstdint>
#include <vector>
#include <random>
#include <set>

namespace Reactor
{
	namespace
	{
		thread_local Common::LogRealmHandle lrh("scene");

		constexpr glm::ivec3 simSize = { 64, 128, 64 };
		constexpr glm::ivec3 initBlockOrigin = { 0, 0, 0 };
		constexpr glm::ivec3 initBlockSize = { 64, 64, 64 };
		constexpr uint32_t elementTypes = 16;

		template<class Vec>
		size_t Index(Vec size, Vec index)
		{
			auto scale = typename Vec::value_type(1);
			auto linear = typename Vec::value_type(0);
			for (size_t i = 0U; i < size_t(index.length()); ++i)
			{
				linear += index[i] * scale;
				scale *= size[i];
			}
			return linear;
		}

		struct LcgParameters
		{
			uint32_t multiply;
			uint32_t add;
		} lcgParameters = { UINT32_C(2891336453), UINT32_C(1) };

		uint32_t LcgSkip(uint32_t lcg, uint32_t steps)
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


		const uint32_t fnv1a16Init = 2166136261U;
		uint32_t Fnv1a16(uint32_t h, uint32_t d)
		{
			h = (h ^ ( d        & 0xFFU)) * 16777619U;
			h = (h ^ ((d >>  8) & 0xFFU)) * 16777619U;
			h = (h ^ ((d >> 16) & 0xFFU)) * 16777619U;
			h = (h ^ ((d >> 24) & 0xFFU)) * 16777619U;
		    return h;
		}

		uint32_t Fnv1a16Finish(uint32_t h)
		{
			return (h >> 16) ^ h;
		}
	}

	using namespace Gl;

	struct SceneImpl
	{
		Scene &parent;

		VertexArray renderVao;
		ProgramBindings renderBindings;
		Buffer renderVbo;
		Buffer renderEvbo;
		Program renderProgram;

		ProgramBindings reactBindings;
		Program rintentProgram;
		Program rapplyProgram;
		GLint tickCount = 0;
		uint32_t lcg = 0;

		Buffer sim;
		Buffer simNext;
		Buffer simRender;
		Buffer rintent;
		Buffer reactions;
		Buffer colors;
		bool preserveSimForRendering = false;

		SceneImpl(Scene &newParent) : parent(newParent)
		{
			std::random_device rd;
			std::mt19937 gen(rd());

			renderProgram = Program(std::in_place);
			{
				auto vert = Shader(GL_VERTEX_SHADER);
				vert.Source(std::string(Resource::Vert.data.begin(), Resource::Vert.data.end()));
				vert.Compile();
				renderProgram.AttachShader(vert);
				auto frag = Shader(GL_FRAGMENT_SHADER);
				frag.Source(std::string(Resource::Frag.data.begin(), Resource::Frag.data.end()));
				frag.Compile();
				renderProgram.AttachShader(frag);
				renderProgram.Link();
			}
			renderProgram.SetUniform("simSize", simSize);

			renderVao = VertexArray(std::in_place);
			renderBindings.DeclareStorage("Sim");
			renderBindings.DeclareStorage("Colors");
			renderBindings.SetProgramBindings(renderProgram);
			renderVbo = Buffer(std::in_place);
			renderEvbo = Buffer(std::in_place);
			{
				static const std::array<std::array<float, 2>, 4> posData = {{
					{{ -1.f, -1.f }},
					{{  1.f, -1.f }},
					{{  1.f,  1.f }},
					{{ -1.f,  1.f }},
				}};
				glNamedBufferData(renderVbo, sizeof(posData), posData.data(), GL_STATIC_DRAW);
				auto renderPosLocation = glGetAttribLocation(renderProgram, "pos");
				glEnableVertexArrayAttrib(renderVao, renderPosLocation);
				glVertexArrayAttribFormat(renderVao, renderPosLocation, 2, GL_FLOAT, GL_FALSE, 0);
				glVertexArrayVertexBuffer(renderVao, renderPosLocation, renderVbo, 0, sizeof(float) * 2);
				static const std::array<GLuint, 4> indexData = {{ 0, 1, 2, 3 }};
				glNamedBufferData(renderEvbo, sizeof(indexData), indexData.data(), GL_STATIC_DRAW);
			}

			colors = Buffer(std::in_place);
			{
				std::uniform_real_distribution<float> dist;
				std::vector<glm::vec3> data;
				for (uint32_t i = 0; i < elementTypes; ++i)
				{
					data.push_back({ dist(gen), dist(gen), dist(gen) });
				}
				glNamedBufferData(colors, sizeof(glm::vec3) * data.size(), data.data(), GL_STATIC_DRAW);
			}

			sim = Buffer(std::in_place);
			{
				std::uniform_int_distribution<uint32_t> dist(0U, elementTypes - 1U);
				std::vector<uint32_t> data(simSize.x * simSize.y * simSize.z, 0);
				for (int32_t z = 0; z < initBlockSize.z; ++z)
				for (int32_t y = 0; y < initBlockSize.y; ++y)
				for (int32_t x = 0; x < initBlockSize.x; ++x)
				{
					data[Index(simSize, glm::ivec3{ x, y, z } + initBlockOrigin)] = dist(gen);
				}
				glNamedBufferData(sim, sizeof(uint32_t) * data.size(), data.data(), GL_DYNAMIC_DRAW);
			}

			simNext = Buffer(std::in_place);
			glNamedBufferData(simNext, sizeof(uint32_t) * simSize.x * simSize.y * simSize.z, nullptr, GL_DYNAMIC_DRAW);

			simRender = Buffer(std::in_place);
			glNamedBufferData(simRender, sizeof(uint32_t) * simSize.x * simSize.y * simSize.z, nullptr, GL_DYNAMIC_DRAW);

			rintent = Buffer(std::in_place);
			glNamedBufferData(rintent, sizeof(uint32_t) * simSize.x * simSize.y * simSize.z, nullptr, GL_STREAM_COPY);

			reactions = Buffer(std::in_place);
			{
				struct ReactionHeader
				{
					uint32_t longestChain = 0;
					uint32_t padding1 = 0;
					uint32_t padding2 = 0;
					uint32_t padding3 = 0;
				};
				struct Reaction
				{
					std::array<uint32_t, 2> inTypes{ 0, 0 };
					std::array<uint32_t, 2> outTypes{ 0, 0 };

					bool Empty() const
					{
						return inTypes[0] == 0 && inTypes[1] == 0;
					}

					auto operator <=>(const Reaction &other) const
					{
						return inTypes <=> other.inTypes;
					}
				};
				std::uniform_int_distribution<uint32_t> dist(0U, elementTypes - 1U);
				std::set<Reaction> reactionsRaw;
				for (uint32_t i = 0; i < elementTypes * 4U; ++i)
				{
					Reaction r;
					while (true)
					{
						r.inTypes[0] = dist(gen);
						r.inTypes[1] = dist(gen);
						if (!(r.inTypes[0] < r.inTypes[1]))
						{
							continue;
						}
						r.outTypes[0] = dist(gen);
						r.outTypes[1] = dist(gen);
						if (!reactionsRaw.insert(r).second)
						{
							continue;
						}
						break;
					}
				}
				uint32_t powerOf2Size = 1;
				while (powerOf2Size < reactionsRaw.size() * 4U / 3U) // 75% max load factor
				{
					powerOf2Size *= 2;
					assert(powerOf2Size);
				}
				ReactionHeader reactionHeader;
				std::vector<Reaction> reactionsData(powerOf2Size);
				for (auto &r : reactionsRaw)
				{
					uint32_t index = Fnv1a16Finish(Fnv1a16(Fnv1a16(fnv1a16Init, r.inTypes[0]), r.inTypes[1]));
					bool ok = false;
					uint32_t step = 0U;
					for (uint32_t i = 0; i < powerOf2Size; ++i)
					{
						index = (index + step) % powerOf2Size;
						step += 1U;
						if (reactionsData[index].Empty())
						{
							reactionsData[index] = r;
							ok = true;
							break;
						}
					}
					reactionHeader.longestChain = std::max(reactionHeader.longestChain, step);
					assert(ok);
				}
				lrh("longestChain ", reactionHeader.longestChain);
				glNamedBufferData(reactions, sizeof(ReactionHeader) + sizeof(Reaction) * reactionsData.size(), nullptr, GL_STATIC_DRAW);
				glNamedBufferSubData(reactions, 0, sizeof(ReactionHeader), &reactionHeader);
				glNamedBufferSubData(reactions, sizeof(ReactionHeader), sizeof(Reaction) * reactionsData.size(), reactionsData.data());
			}

			rintentProgram = Program(std::in_place);
			{
				auto comp = Shader(GL_COMPUTE_SHADER);
				comp.Source(std::string(Resource::Rintent.data.begin(), Resource::Rintent.data.end()));
				comp.Compile();
				rintentProgram.AttachShader(comp);
				rintentProgram.Link();
			}
			rintentProgram.SetUniform("simSize", simSize);

			rapplyProgram = Program(std::in_place);
			{
				auto comp = Shader(GL_COMPUTE_SHADER);
				comp.Source(std::string(Resource::Rapply.data.begin(), Resource::Rapply.data.end()));
				comp.Compile();
				rapplyProgram.AttachShader(comp);
				rapplyProgram.Link();
			}
			rapplyProgram.SetUniform("simSize", simSize);

			reactBindings.DeclareStorage("Reactions");
			reactBindings.DeclareStorage("Rintent"  );
			reactBindings.DeclareStorage("Sim"      );
			reactBindings.DeclareStorage("SimNext"  );
			reactBindings.SetProgramBindings(rintentProgram);
			reactBindings.SetProgramBindings(rapplyProgram);

			glMemoryBarrier(GL_ALL_BARRIER_BITS);
		}
	};

	Scene::Scene() : impl(std::make_unique<SceneImpl>(*this))
	{
	}

	Scene::~Scene() = default;

	void Scene::Tick()
	{
		impl->rintentProgram.SetUniform("globalLcg", impl->lcg);
		{
			if (impl->preserveSimForRendering)
			{
				std::swap(impl->sim, impl->simRender);
			}
			impl->reactBindings.BindStorageBuffer("Reactions", impl->reactions);
			impl->reactBindings.BindStorageBuffer("Rintent", impl->rintent);
			impl->reactBindings.BindStorageBuffer("Sim", impl->preserveSimForRendering ? impl->simRender : impl->sim);
			impl->reactBindings.BindStorageBuffer("SimNext", impl->simNext);
			impl->preserveSimForRendering = false;
			{
				auto programUsage = impl->rintentProgram.Use();
				glDispatchCompute(simSize.x / 32, simSize.y, simSize.z);
			}
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			{
				auto programUsage = impl->rapplyProgram.Use();
				glDispatchCompute(simSize.x / 32, simSize.y, simSize.z);
			}
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}
		std::swap(impl->sim, impl->simNext);
		impl->tickCount += 1;
		impl->lcg = LcgSkip(impl->lcg, 7263);
	}

	void Scene::Render(Cfu cfu, float fov, float aspectRatio)
	{
		impl->renderProgram.SetUniform("cfu", cfu);
		impl->renderProgram.SetUniform("fov", fov);
		impl->renderProgram.SetUniform("aspectRatio", aspectRatio);
		{
			auto programUsage = impl->renderProgram.Use();
			auto vaoBinding = impl->renderVao.Bind();
			impl->renderBindings.BindStorageBuffer("Sim", impl->sim);
			impl->renderBindings.BindStorageBuffer("Colors", impl->colors);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
		impl->preserveSimForRendering = true;
	}
}
