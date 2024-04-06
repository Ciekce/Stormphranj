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

#include <cmath>
#include <utility>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <array>

#include "util/bitfield.h"
#include "util/cemath.h"

namespace stormphranj
{
	enum class Piece : u8
	{
		BlackPawn = 0,
		WhitePawn,
		BlackAlfil,
		WhiteAlfil,
		BlackFerz,
		WhiteFerz,
		BlackKnight,
		WhiteKnight,
		BlackRook,
		WhiteRook,
		BlackKing,
		WhiteKing,
		None
	};

	enum class PieceType
	{
		Pawn = 0,
		Alfil,
		Ferz,
		Knight,
		Rook,
		King,
		None
	};

	enum class Color : i8
	{
		Black = 0,
		White,
		None
	};

	[[nodiscard]] constexpr auto oppColor(Color color)
	{
		assert(color != Color::None);
		return static_cast<Color>(!static_cast<i32>(color));
	}

	[[nodiscard]] constexpr auto colorPiece(PieceType piece, Color color)
	{
		assert(piece != PieceType::None);
		assert(color != Color::None);

		return static_cast<Piece>((static_cast<i32>(piece) << 1) + static_cast<i32>(color));
	}

	[[nodiscard]] constexpr auto pieceType(Piece piece)
	{
		assert(piece != Piece::None);
		return static_cast<PieceType>(static_cast<i32>(piece) >> 1);
	}

	[[nodiscard]] constexpr auto pieceColor(Piece piece)
	{
		assert(piece != Piece::None);
		return static_cast<Color>(static_cast<i32>(piece) & 1);
	}

	[[nodiscard]] constexpr auto flipPieceColor(Piece piece)
	{
		assert(piece != Piece::None);
		return static_cast<Piece>(static_cast<i32>(piece) ^ 0x1);
	}

	[[nodiscard]] constexpr auto copyPieceColor(Piece piece, PieceType target)
	{
		assert(piece != Piece::None);
		assert(target != PieceType::None);

		return colorPiece(target, pieceColor(piece));
	}

	[[nodiscard]] constexpr auto pieceFromChar(char c)
	{
		switch (c)
		{
		case 'p': return Piece::  BlackPawn;
		case 'P': return Piece::  WhitePawn;
		case 'b': return Piece:: BlackAlfil;
		case 'B': return Piece:: WhiteAlfil;
		case 'q': return Piece::  BlackFerz;
		case 'Q': return Piece::  WhiteFerz;
		case 'n': return Piece::BlackKnight;
		case 'N': return Piece::WhiteKnight;
		case 'r': return Piece::  BlackRook;
		case 'R': return Piece::  WhiteRook;
		case 'k': return Piece::  BlackKing;
		case 'K': return Piece::  WhiteKing;
		default : return Piece::       None;
		}
	}

	[[nodiscard]] constexpr auto pieceToChar(Piece piece)
	{
		switch (piece)
		{
		case Piece::       None: return ' ';
		case Piece::  BlackPawn: return 'p';
		case Piece::  WhitePawn: return 'P';
		case Piece:: BlackAlfil: return 'b';
		case Piece:: WhiteAlfil: return 'B';
		case Piece::  BlackFerz: return 'q';
		case Piece::  WhiteFerz: return 'Q';
		case Piece::BlackKnight: return 'n';
		case Piece::WhiteKnight: return 'N';
		case Piece::  BlackRook: return 'r';
		case Piece::  WhiteRook: return 'R';
		case Piece::  BlackKing: return 'k';
		case Piece::  WhiteKing: return 'K';
		default: return ' ';
		}
	}

	[[nodiscard]] constexpr auto pieceTypeFromChar(char c)
	{
		switch (c)
		{
		case 'p': return PieceType::  Pawn;
		case 'b': return PieceType:: Alfil;
		case 'q': return PieceType::  Ferz;
		case 'n': return PieceType::Knight;
		case 'r': return PieceType::  Rook;
		case 'k': return PieceType::  King;
		default : return PieceType::  None;
		}
	}

	[[nodiscard]] constexpr auto pieceTypeToChar(PieceType piece)
	{
		switch (piece)
		{
		case PieceType::  None: return ' ';
		case PieceType::  Pawn: return 'p';
		case PieceType:: Alfil: return 'b';
		case PieceType::  Ferz: return 'q';
		case PieceType::Knight: return 'n';
		case PieceType::  Rook: return 'r';
		case PieceType::  King: return 'k';
		default: return ' ';
		}
	}

	// upside down
	enum class Square : u8
	{
		A1, B1, C1, D1, E1, F1, G1, H1,
		A2, B2, C2, D2, E2, F2, G2, H2,
		A3, B3, C3, D3, E3, F3, G3, H3,
		A4, B4, C4, D4, E4, F4, G4, H4,
		A5, B5, C5, D5, E5, F5, G5, H5,
		A6, B6, C6, D6, E6, F6, G6, H6,
		A7, B7, C7, D7, E7, F7, G7, H7,
		A8, B8, C8, D8, E8, F8, G8, H8,
		None
	};

	[[nodiscard]] constexpr auto toSquare(u32 rank, u32 file)
	{
		assert(rank < 8);
		assert(file < 8);

		return static_cast<Square>((rank << 3) | file);
	}

	[[nodiscard]] constexpr auto squareRank(Square square)
	{
		assert(square != Square::None);
		return static_cast<i32>(square) >> 3;
	}

	[[nodiscard]] constexpr auto squareFile(Square square)
	{
		assert(square != Square::None);
		return static_cast<i32>(square) & 0x7;
	}

	[[nodiscard]] constexpr auto flipSquare(Square square)
	{
		assert(square != Square::None);
		return static_cast<Square>(static_cast<i32>(square) ^ 0x38);
	}

	[[nodiscard]] constexpr auto squareBit(Square square)
	{
		assert(square != Square::None);
		return U64(1) << static_cast<i32>(square);
	}

	[[nodiscard]] constexpr auto squareBitChecked(Square square)
	{
		if (square == Square::None)
			return U64(0);

		return U64(1) << static_cast<i32>(square);
	}

	template <Color C>
	constexpr auto relativeRank(i32 rank)
	{
		assert(rank >= 0 && rank < 8);

		if constexpr (C == Color::Black)
			return 7 - rank;
		else return rank;
	}

	constexpr auto relativeRank(Color c, i32 rank)
	{
		assert(rank >= 0 && rank < 8);
		return c == Color::Black ? 7 - rank : rank;
	}

	using Score = i32;

	constexpr auto ScoreInf = 32767;
	constexpr auto ScoreMate = 32766;
	constexpr auto ScoreWin = 25000;

	constexpr i32 MaxDepth = 255;

	constexpr auto ScoreMaxMate = ScoreMate - MaxDepth;
}
