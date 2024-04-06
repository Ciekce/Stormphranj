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

#include "../types.h"

#include <array>

#include "nnue.h"
#include "../position/position.h"
#include "../core.h"

#include "../see.h"

namespace stormphranj::eval
{
	// black, white
	using Contempt = std::array<Score, 2>;

	inline auto materialBalance(const Position &pos)
	{
		const auto material = [&](Color c)
		{
			return pos.bbs(). pawns(c).popcount() * see::values::Pawn
				+ pos.bbs(). alfils(c).popcount() * see::values::Alfil
				+ pos.bbs(). ferzes(c).popcount() * see::values::Ferz
				+ pos.bbs().knights(c).popcount() * see::values::Knight
				+ pos.bbs().  rooks(c).popcount() * see::values::Rook;
		};

		return (material(pos.toMove()) - material(pos.opponent())) * 2;
	}

	inline auto scaleEval(const Position &pos, i32 eval)
	{
		eval = eval * (200 - pos.halfmove()) / 200;
		return eval;
	}

	template <bool Scale = true>
	inline auto adjustEval(const Position &pos, const Contempt &contempt, i32 eval)
	{
		if constexpr (Scale)
			eval = scaleEval(pos, eval);
		eval += contempt[static_cast<i32>(pos.toMove())];
		return std::clamp(eval, -ScoreWin + 1, ScoreWin - 1);
	}

	template <bool Scale = true>
	inline auto staticEval(const Position &pos,
		[[maybe_unused]] const NnueState &nnueState, const Contempt &contempt = {})
	{
	//	const auto nnueEval = nnueState.evaluate(pos.bbs(), pos.toMove());
		const auto material = materialBalance(pos);
		return adjustEval<Scale>(pos, contempt, material);
	}

	template <bool Scale = true>
	inline auto staticEvalOnce(const Position &pos, const Contempt &contempt = {})
	{
	//	const auto nnueEval = NnueState::evaluateOnce(pos.bbs(), pos.blackKing(), pos.whiteKing(), pos.toMove());
		const auto material = materialBalance(pos);
		return adjustEval<Scale>(pos, contempt, material);
	}
}
