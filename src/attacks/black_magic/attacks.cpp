/*
 * Stormphranj, a UCI shatranj engine
 * Copyright (C) 2024 Ciekce
 *
 * Stormphranj is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stormphranj is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stormphranj. If not, see <https://www.gnu.org/licenses/>.
 */

#include "../attacks.h"

#if !SPJ_HAS_BMI2
namespace stormphranj::attacks
{
	using namespace black_magic;

	namespace
	{
		auto generateRookAttacks()
		{
			std::array<Bitboard, RookData.tableSize> dst{};

			for (u32 square = 0; square < 64; ++square)
			{
				const auto &data = RookData.data[square];

				const auto invMask = ~data.mask;
				const auto maxEntries = 1 << invMask.popcount();

				for (u32 i = 0; i < maxEntries; ++i)
				{
					const auto occupancy = util::pdep(i, invMask);
					const auto idx = getIdx(occupancy, static_cast<Square>(square));

					if (!dst[data.offset + idx].empty())
						continue;

					for (const auto dir : {offsets::Up, offsets::Down, offsets::Left, offsets::Right})
					{
						dst[data.offset + idx]
							|= internal::generateSlidingAttacks(static_cast<Square>(square), dir, occupancy);
					}
				}
			}

			return dst;
		}
	}

	const std::array<Bitboard, RookData.tableSize> RookAttacks = generateRookAttacks();
}
#endif // !SPJ_HAS_BMI2
