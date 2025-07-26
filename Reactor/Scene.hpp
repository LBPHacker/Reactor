#pragma once
#include "Cfu.hpp"
#include <memory>

namespace Reactor
{
	struct SceneImpl;

	class Scene
	{
		std::unique_ptr<SceneImpl> impl;

	public:
		Scene();
		Scene(const Scene &) = delete;
		Scene &operator =(const Scene &) = delete;
		~Scene();

		void Tick();
		void Render(Cfu cfu, float fov, float aspectRatio);
	};
}
