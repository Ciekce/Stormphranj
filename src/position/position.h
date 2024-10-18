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

#include <string>
#include <vector>
#include <stack>
#include <optional>
#include <utility>

#include "boards.h"
#include "../move.h"
#include "../attacks/attacks.h"
#include "../ttable.h"
#include "../eval/nnue.h"
#include "../rays.h"
#include "../keys.h"

namespace stormphranj
{
	struct BoardState
	{
		PositionBoards boards{};

		u64 key{};

		Bitboard checkers{};
		Bitboard pinned{};
		Bitboard threats{};

		Move lastMove{NullMove};

		u16 halfmove{};

		std::array<Square, 2> kings{Square::None, Square::None};

		[[nodiscard]] inline auto blackKing() const
		{
			return kings[0];
		}

		[[nodiscard]] inline auto whiteKing() const
		{
			return kings[1];
		}

		[[nodiscard]] inline auto king(Color c) const
		{
			return kings[static_cast<i32>(c)];
		}

		[[nodiscard]] inline auto king(Color c) -> auto &
		{
			return kings[static_cast<i32>(c)];
		}
	};

	static_assert(sizeof(BoardState) == 168);

	[[nodiscard]] inline auto squareToString(Square square)
	{
		constexpr auto Files = std::array{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
		constexpr auto Ranks = std::array{'1', '2', '3', '4', '5', '6', '7', '8'};

		const auto s = static_cast<u32>(square);
		return std::string{Files[s % 8], Ranks[s / 8]};
	}

	class Position;

	template <bool UpdateNnue>
	class HistoryGuard
	{
	public:
		explicit HistoryGuard(Position &pos, eval::NnueState *nnueState)
			: m_pos{pos},
			  m_nnueState{nnueState} {}
		inline ~HistoryGuard();

	private:
		Position &m_pos;
		eval::NnueState *m_nnueState;
	};

	class Position
	{
	public:
		Position();
		~Position() = default;

		Position(const Position &) = default;
		Position(Position &&) = default;

		auto resetToStarting() -> void;
		auto resetFromFen(const std::string &fen) -> bool;

		auto copyStateFrom(const Position &other) -> void;

		// Moves are assumed to be legal
		template <bool UpdateNnue = true, bool StateHistory = true>
		auto applyMoveUnchecked(Move move, eval::NnueState *nnueState) -> void;

		// Moves are assumed to be legal
		template <bool UpdateNnue = true>
		[[nodiscard]] inline auto applyMove(Move move, eval::NnueState *nnueState)
		{
			if constexpr (UpdateNnue)
				assert(nnueState != nullptr);

			applyMoveUnchecked<UpdateNnue>(move, nnueState);

			return HistoryGuard<UpdateNnue>{*this, UpdateNnue ? nnueState : nullptr};
		}

		template <bool UpdateNnue = true>
		auto popMove(eval::NnueState *nnueState) -> void;

		auto clearStateHistory() -> void;

		[[nodiscard]] auto isPseudolegal(Move move) const -> bool;
		[[nodiscard]] auto isLegal(Move move) const -> bool;

	private:
		[[nodiscard]] inline auto currState() -> auto & { return m_states.back(); }
		[[nodiscard]] inline auto currState() const -> const auto & { return m_states.back(); }

	public:
		[[nodiscard]] inline auto boards() const -> const auto & { return currState().boards; }
		[[nodiscard]] inline auto bbs() const -> const auto & { return currState().boards.bbs(); }

		[[nodiscard]] inline auto toMove() const
		{
			return m_blackToMove ? Color::Black : Color::White;
		}

		[[nodiscard]] inline auto opponent() const
		{
			return m_blackToMove ? Color::White : Color::Black;
		}

		[[nodiscard]] inline auto halfmove() const { return currState().halfmove; }
		[[nodiscard]] inline auto fullmove() const { return m_fullmove; }

		[[nodiscard]] inline auto key() const { return currState().key; }

		[[nodiscard]] inline auto roughKeyAfter(Move move) const
		{
			assert(move);

			const auto &state = currState();

			const auto moving = state.boards.pieceAt(move.src());
			assert(moving != Piece::None);

			const auto captured = state.boards.pieceAt(move.dst());

			auto key = state.key;

			key ^= keys::pieceSquare(moving, move.src());
			key ^= keys::pieceSquare(moving, move.dst());

			if (captured != Piece::None)
				key ^= keys::pieceSquare(captured, move.dst());

			key ^= keys::color();

			return key;
		}

		[[nodiscard]] inline auto allAttackersTo(Square square, Bitboard occupancy) const
		{
			assert(square != Square::None);

			const auto &bbs = this->bbs();

			Bitboard attackers{};

			const auto rooks = bbs.rooks();
			attackers |= rooks & attacks::getRookAttacks(square, occupancy);

			attackers |= bbs.blackPawns() & attacks::getPawnAttacks(square, Color::White);
			attackers |= bbs.whitePawns() & attacks::getPawnAttacks(square, Color::Black);

			const auto alfils = bbs.alfils();
			attackers |= alfils & attacks::getAlfilAttacks(square);

			const auto ferzes = bbs.ferzes();
			attackers |= ferzes & attacks::getFerzAttacks(square);

			const auto knights = bbs.knights();
			attackers |= knights & attacks::getKnightAttacks(square);

			const auto kings = bbs.kings();
			attackers |= kings & attacks::getKingAttacks(square);

			return attackers;
		}

		[[nodiscard]] inline auto attackersTo(Square square, Color attacker) const
		{
			assert(square != Square::None);

			const auto &bbs = this->bbs();

			Bitboard attackers{};

			const auto occ = bbs.occupancy();

			const auto pawns = bbs.pawns(attacker);
			attackers |= pawns & attacks::getPawnAttacks(square, oppColor(attacker));

			const auto alfils = bbs.alfils(attacker);
			attackers |= alfils & attacks::getAlfilAttacks(square);

			const auto ferzes = bbs.ferzes(attacker);
			attackers |= ferzes & attacks::getFerzAttacks(square);

			const auto knights = bbs.knights(attacker);
			attackers |= knights & attacks::getKnightAttacks(square);

			const auto rooks = bbs.rooks(attacker);
			attackers |= rooks & attacks::getRookAttacks(square, occ);

			const auto kings = bbs.kings(attacker);
			attackers |= kings & attacks::getKingAttacks(square);

			return attackers;
		}

		template <bool ThreatShortcut = true>
		[[nodiscard]] static inline auto isAttacked(const BoardState &state,
			Color toMove, Square square, Color attacker)
		{
			assert(toMove != Color::None);
			assert(square != Square::None);
			assert(attacker != Color::None);

			if constexpr (ThreatShortcut)
			{
				if (attacker != toMove)
					return state.threats[square];
			}

			const auto &bbs = state.boards.bbs();

			const auto occ = bbs.occupancy();

			if (const auto knights = bbs.knights(attacker);
				!(knights & attacks::getKnightAttacks(square)).empty())
				return true;

			if (const auto ferzes = bbs.ferzes(attacker);
				!(ferzes & attacks::getFerzAttacks(square)).empty())
				return true;

			if (const auto pawns = bbs.pawns(attacker);
				!(pawns & attacks::getPawnAttacks(square, oppColor(attacker))).empty())
				return true;

			if (const auto alfils = bbs.alfils(attacker);
				!(alfils & attacks::getAlfilAttacks(square)).empty())
				return true;

			if (const auto kings = bbs.kings(attacker);
				!(kings & attacks::getKingAttacks(square)).empty())
				return true;

			if (const auto rooks = bbs.rooks(attacker);
				!(rooks & attacks::getRookAttacks(square, occ)).empty())
				return true;

			return false;
		}

		template <bool ThreatShortcut = true>
		[[nodiscard]] inline auto isAttacked(Square square, Color attacker) const
		{
			assert(square != Square::None);
			assert(attacker != Color::None);

			return isAttacked<ThreatShortcut>(currState(), toMove(), square, attacker);
		}

		[[nodiscard]] inline auto anyAttacked(Bitboard squares, Color attacker) const
		{
			assert(attacker != Color::None);

			if (attacker == opponent())
				return !(squares & currState().threats).empty();

			while (squares)
			{
				const auto square = squares.popLowestSquare();
				if (isAttacked(square, attacker))
					return true;
			}

			return false;
		}

		[[nodiscard]] inline auto blackKing() const { return currState().blackKing(); }
		[[nodiscard]] inline auto whiteKing() const { return currState().whiteKing(); }

		template <Color C>
		[[nodiscard]] inline auto king() const
		{
			return currState().king(C);
		}

		[[nodiscard]] inline auto king(Color c) const
		{
			assert(c != Color::None);
			return currState().king(c);
		}

		template <Color C>
		[[nodiscard]] inline auto oppKing() const
		{
			return currState().king(oppColor(C));
		}

		[[nodiscard]] inline auto oppKing(Color c) const
		{
			assert(c != Color::None);
			return currState().king(oppColor(c));
		}

		[[nodiscard]] inline auto isCheck() const
		{
			return !currState().checkers.empty();
		}

		[[nodiscard]] inline auto checkers() const { return currState().checkers; }
		[[nodiscard]] inline auto pinned() const { return currState().pinned; }
		[[nodiscard]] inline auto threats() const { return currState().threats; }

		[[nodiscard]] auto hasCycle(i32 ply) const -> bool;

		[[nodiscard]] inline auto isBareKingWin() const
		{
			const auto &bbs = this->bbs();

			const auto us = toMove();
			const auto them = oppColor(toMove());

			return bbs.occupancy(us) != bbs.kings(us)
				&& bbs.occupancy(them) == bbs.kings(them);
		}

		[[nodiscard]] inline auto isDrawn(bool threefold) const
		{
			const auto halfmove = currState().halfmove;

			// TODO handle mate
			if (halfmove >= 140)
				return true;

			const auto currKey = currState().key;
			const auto limit = std::max(0, static_cast<i32>(m_keys.size()) - halfmove - 2);

			i32 repetitionsLeft = threefold ? 2 : 1;

			for (auto i = static_cast<i32>(m_keys.size()) - 4; i >= limit; i -= 2)
			{
				if (m_keys[i] == currKey
					&& --repetitionsLeft == 0)
					return true;
			}

			const auto &bbs = this->bbs();

			// KK
			if (bbs.occupancy() == bbs.kings())
				return true;

			//TODO more?

			return false;
		}

		[[nodiscard]] inline auto lastMove() const
		{
			return m_states.empty() ? NullMove : currState().lastMove;
		}

		[[nodiscard]] inline auto captureTarget(Move move) const
		{
			assert(move != NullMove);
			return boards().pieceAt(move.dst());
		}

		[[nodiscard]] inline auto isNoisy(Move move) const
		{
			assert(move != NullMove);

			return move.isPromo()
				|| boards().pieceAt(move.dst()) != Piece::None;
		}

		[[nodiscard]] inline auto noisyCapturedPiece(Move move) const -> std::pair<bool, Piece>
		{
			assert(move != NullMove);

			const auto captured = boards().pieceAt(move.dst());
			return {captured != Piece::None || move.isPromo(), captured};
		}

		[[nodiscard]] auto toFen() const -> std::string;

		[[nodiscard]] inline auto operator==(const Position &other) const
		{
			const auto &ourState = currState();
			const auto &theirState = other.m_states.back();

			// every other field is a function of these
			return ourState.boards == theirState.boards
				&& ourState.halfmove == theirState.halfmove
				&& m_fullmove == other.m_fullmove;
		}

		[[nodiscard]] inline auto deepEquals(const Position &other) const
		{
			return *this == other
				&& currState().kings == other.m_states.back().kings
				&& currState().checkers == other.m_states.back().checkers
				&& currState().pinned == other.m_states.back().pinned
				&& currState().threats == other.m_states.back().threats
				&& currState().key == other.m_states.back().key;
		}

		auto regen() -> void;

#ifndef NDEBUG
		auto printHistory(Move last = NullMove) -> void;
		auto verify() -> bool;
#endif

		[[nodiscard]] auto moveFromUci(const std::string &move) const -> Move;

		[[nodiscard]] inline auto plyFromStartpos() const -> u32
		{
			return m_fullmove * 2 - (m_blackToMove ? 0 : 1) - 1;
		}
		[[nodiscard]] inline auto totalMaterial() const -> u32
		{
			const auto &bbs = this->bbs();
			const auto numpawns = bbs.pawns().popcount();
			const auto numalfils = bbs.alfils().popcount();
			const auto numferzes = bbs.ferzes().popcount();
			const auto numknights = bbs.knights().popcount();
			const auto numrooks = bbs.rooks().popcount();
			return numpawns + numalfils + 2 * numferzes + 4 * numknights + 6 * numrooks;
		}
		auto operator=(const Position &) -> Position & = default;
		auto operator=(Position &&) -> Position & = default;

		[[nodiscard]] static auto starting() -> Position;
		[[nodiscard]] static auto fromFen(const std::string &fen) -> std::optional<Position>;

	private:
		template <bool UpdateKeys = true>
		auto setPiece(Piece piece, Square square) -> void;
		template <bool UpdateKeys = true>
		auto removePiece(Piece piece, Square square) -> void;
		template <bool UpdateKeys = true>
		auto movePieceNoCap(Piece piece, Square src, Square dst) -> void;

		template <bool UpdateKeys = true, bool UpdateNnue = true>
		[[nodiscard]] auto movePiece(Piece piece, Square src, Square dst, eval::NnueUpdates &nnueUpdates) -> Piece;

		template <bool UpdateKeys = true, bool UpdateNnue = true>
		auto promotePawn(Piece pawn, Square src, Square dst, eval::NnueUpdates &nnueUpdates) -> Piece;

		[[nodiscard]] inline auto calcCheckers() const
		{
			const auto color = toMove();
			const auto &state = currState();

			return attackersTo(state.king(color), oppColor(color));
		}

		[[nodiscard]] inline auto calcPinned() const
		{
			const auto color = toMove();
			const auto &state = currState();

			Bitboard pinned{};

			const auto king = state.king(color);
			const auto opponent = oppColor(color);

			const auto &bbs = state.boards.bbs();

			const auto ourOcc = bbs.occupancy(color);
			const auto oppOcc = bbs.occupancy(opponent);

			auto potentialAttackers = attacks::getRookAttacks(king, oppOcc) & bbs.rooks(opponent);

			while (potentialAttackers)
			{
				const auto potentialAttacker = potentialAttackers.popLowestSquare();
				const auto maybePinned = ourOcc & orthoRayBetween(potentialAttacker, king);

				if (maybePinned.one())
					pinned |= maybePinned;
			}

			return pinned;
		}

		[[nodiscard]] inline auto calcThreats() const
		{
			const auto us = toMove();
			const auto them = oppColor(us);

			const auto &state = currState();
			const auto &bbs = state.boards.bbs();

			Bitboard threats{};

			const auto occ = bbs.occupancy();

			auto alfils = bbs.alfils(them);
			while (alfils)
			{
				const auto alfil = alfils.popLowestSquare();
				threats |= attacks::getAlfilAttacks(alfil);
			}

			auto ferzes = bbs.ferzes(them);
			while (ferzes)
			{
				const auto ferz = ferzes.popLowestSquare();
				threats |= attacks::getFerzAttacks(ferz);
			}

			auto knights = bbs.knights(them);
			while (knights)
			{
				const auto knight = knights.popLowestSquare();
				threats |= attacks::getKnightAttacks(knight);
			}

			auto rooks = bbs.rooks(them);
			while (rooks)
			{
				const auto rook = rooks.popLowestSquare();
				threats |= attacks::getRookAttacks(rook, occ);
			}

			const auto pawns = bbs.pawns(them);
			if (them == Color::Black)
				threats |= pawns.shiftDownLeft() | pawns.shiftDownRight();
			else threats |= pawns.shiftUpLeft() | pawns.shiftUpRight();

			threats |= attacks::getKingAttacks(state.king(them));

			return threats;
		}

		bool m_blackToMove{};

		u32 m_fullmove{1};

		std::vector<BoardState> m_states{};
		std::vector<u64> m_keys{};
	};

	template <bool UpdateNnue>
	HistoryGuard<UpdateNnue>::~HistoryGuard()
	{
		m_pos.popMove<UpdateNnue>(m_nnueState);
	}

	[[nodiscard]] auto squareFromString(const std::string &str) -> Square;
}
