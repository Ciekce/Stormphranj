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

#pragma once

#include "types.h"

#include "wdl.h"

namespace stormphranj
{
	namespace opts
	{
		constexpr i32 DefaultNormalizedContempt = 0;

		struct GlobalOptions
		{
			bool showWdl{true};

			i32 contempt{wdl::unnormalizeScoreMove32(DefaultNormalizedContempt)};
		};

		auto mutableOpts() -> GlobalOptions &;
	}

	extern const opts::GlobalOptions &g_opts;
}
