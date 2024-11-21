#!/usr/bin/env python3

import chess
from enum import Enum

# Various constants based on packet scheme. See
# `/uart-protocol/uart_bidir_protocol.h` and `.c` for more information.
SRC_FILE_SHIFT = 29
SRC_RANK_SHIFT = 26
DEST_FILE_SHIFT = 23
DEST_RANK_SHIFT = 20
PTYPE_SHIFT = 17

M2_SHIFT = 16
M2_SRC_FILE_SHIFT = 13
M2_SRC_RANK_SHIFT = 10
M2_DEST_FILE_SHIFT = 7
M2_DEST_RANK_SHIFT = 4
M2_PTYPE_SHIFT = 1

MTYPE_SHIFT = 2
MTYPE_NORMAL = 0
MTYPE_CHECK = 1
MTYPE_CAPTURE = 2
MTYPE_CASTLE = 3

# Special status constants to indicate exceptional game conditions (outcomes,
# most often) for the MSP.
CHECK       = 0x00000010
CHECKMATE   = 0x00000030
STALEMATE   = 0x00000070


class ButtonEvent(Enum):
    NORMAL = 0
    RESTART = 1
    HINT = 2
    UNDO = 3


# Extracted here since the logic is more complex than a match-case.
def encode_movetype(move: chess.Move, board: chess.Board) -> int:
    global MTYPE_NORMAL
    global MTYPE_CHECK
    global MTYPE_CAPTURE
    global MTYPE_CASTLE

    if (board.gives_check(move)):
        return MTYPE_CHECK
    elif (board.is_capture(move)):
        return MTYPE_CAPTURE
    elif (board.is_castling(move)):
        return MTYPE_CASTLE

    return MTYPE_NORMAL


def encode_packet(move: chess.Move, board: chess.Board, last_move: bool=False) -> int:
    global SRC_FILE_SHIFT
    global SRC_RANK_SHIFT
    global DEST_FILE_SHIFT
    global DEST_RANK_SHIFT
    global PTYPE_SHIFT
    global M2_SHIFT
    global MTYPE_SHIFT

    packet = 0

    packet |= chess.square_file(move.from_square) << SRC_FILE_SHIFT
    packet |= chess.square_rank(move.from_square) << SRC_RANK_SHIFT
    packet |= chess.square_file(move.to_square) << DEST_FILE_SHIFT
    packet |= chess.square_rank(move.to_square) << DEST_RANK_SHIFT
    packet |= board.piece_type_at(move.from_square) << PTYPE_SHIFT
    packet |= (1 << M2_SHIFT) if (board.is_castling(move) or board.is_capture(move)) else (0 << M2_SHIFT) 
    packet |= encode_movetype(move, board) << MTYPE_SHIFT
    packet |= 0x00000002 if board.is_castling(move) else 0x00000000
    packet |= (1 if last_move else 0)

    return packet


def encode_undo(move: chess.Move, board: chess.Board) -> int:
    global SRC_FILE_SHIFT
    global SRC_RANK_SHIFT
    global DEST_FILE_SHIFT
    global DEST_RANK_SHIFT
    global PTYPE_SHIFT
    global M2_SHIFT

    packet = 0
    packet |= chess.square_file(move.from_square) << SRC_FILE_SHIFT
    packet |= chess.square_rank(move.from_square) << SRC_RANK_SHIFT
    packet |= chess.square_file(move.to_square) << DEST_FILE_SHIFT
    packet |= chess.square_rank(move.to_square) << DEST_RANK_SHIFT
    packet |= (move.drop if (move.drop != None) else 0) << PTYPE_SHIFT
    packet |= (1 << M2_SHIFT) if board.is_castling(move) else (0 << M2_SHIFT) 

    packet |= (1 if board.color_at(move.to_square) == chess.BLACK else 0) << 3
    undone_ptype = chess.PieceType(board.piece_at(move.to_square))
    packet |= undone_ptype if (undone_ptype != None) else 0

    return packet


def decode_packet(packet: int) -> chess.Move:
    global SRC_FILE_SHIFT
    global SRC_RANK_SHIFT
    global DEST_FILE_SHIFT
    global DEST_RANK_SHIFT

    move_str = ''

    move_str += chess.FILE_NAMES[(packet >> SRC_FILE_SHIFT) & 0x7]
    move_str += chess.RANK_NAMES[(packet >> SRC_RANK_SHIFT) & 0x7]
    move_str += chess.FILE_NAMES[(packet >> DEST_FILE_SHIFT) & 0x7]
    move_str += chess.RANK_NAMES[(packet >> DEST_RANK_SHIFT) & 0x7]

    return chess.Move.from_uci(move_str)


# A separate helper to detect exceptional button presses indicating special user
# input on the chessboard.
#
# If restart/hint/undo requested, there's no move to decode within the packet.
def parse_button_event(packet: int) -> ButtonEvent:
    match (packet & 0x3):
        case 0x1:
            return ButtonEvent.RESTART
        case 0x2:
            return ButtonEvent.HINT
        case 0x3:
            return ButtonEvent.UNDO
        case _:
            return ButtonEvent.NORMAL


if __name__ == '__main__':
    pass
