#pragma once
#include <array>
#include <cstddef>
#include <type_traits>

namespace Reactor::Common
{
	template<class Item, size_t Dimensions>
	struct Rect;

	template<class Item, size_t Dimensions>
	struct Point
	{
		std::array<Item, Dimensions> data;

		constexpr Item &operator [](size_t index)
		{
			return data[index];
		}

		constexpr const Item &operator [](size_t index) const
		{
			return data[index];
		}

		constexpr std::enable_if_t<std::is_integral_v<Item>, Item> Linear(Point index) const
		{
			auto scale = Item(1);
			auto linear = Item(0);
			for (size_t i = 0U; i < Dimensions; ++i)
			{
				linear += index[i] * scale;
				scale *= data[i];
			}
			return linear;
		}

		constexpr Rect<Item, Dimensions> OriginRect() const
		{
			return { Point{}, *this };
		}
	};

	template<class Item, size_t Dimensions>
	struct Rect
	{
		Point<Item, Dimensions> origin, size;

		constexpr Item Volume() const
		{
			auto volume = Item(1);
			for (size_t i = 0U; i < Dimensions; ++i)
			{
				volume *= size[i];
			}
			return volume;
		}
	};

	using Point3i = Point<int, 3>;
}
