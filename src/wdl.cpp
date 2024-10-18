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

#include "wdl.h"

#include <array>
#include <numeric>

namespace stormphranj::wdl
{
	auto wdlParams(u32 material) -> std::pair<f64, f64>
	{
		constexpr auto As = std::array {
			12.86611189, -1.56947052, -105.75177291, 247.30758159
		};
		constexpr auto Bs = std::array {
			-7.31901285, 36.79299424, -14.98330140, 64.14426025
		};

		static_assert(Material32NormalizationK == static_cast<i32>(std::reduce(As.begin(), As.end())));

		const auto m = std::max(4.0, std::min(64.0, static_cast<f64>(material))) / 32.0;

		return {
			(((As[0] * m + As[1]) * m + As[2]) * m) + As[3],
			(((Bs[0] * m + Bs[1]) * m + Bs[2]) * m) + Bs[3]
		};
	}

	auto wdlModel(Score povScore, u32 material) -> std::pair<i32, i32>
	{
		const auto [a, b] = wdlParams(material);

		const auto x = std::clamp(static_cast<f64>(povScore), -4000.0, 4000.0);

		return {
			static_cast<i32>(std::round(1000.0 / (1.0 + std::exp((a - x) / b)))),
			static_cast<i32>(std::round(1000.0 / (1.0 + std::exp((a + x) / b))))
		};
	}
}
