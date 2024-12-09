#!/usr/bin/env python3

import chess
import serial
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
MTYPE_PROMO = 1
MTYPE_CAPTURE = 2
MTYPE_CASTLE = 3

# Special status constants to indicate exceptional game conditions (outcomes,
# most often) for the MSP.
MSP_SYN = 0x00000008
SYNACK  = 0x00000001
MSP_ACK = 0xFFFFFFFF

MSP_REREQ       = 0x00000004
UNDO_EXHAUST    = 0x00000008

CHECK       = 0x00000011
CHECKMATE   = 0x00000031
STALEMATE   = 0x00000071


class ButtonEvent(Enum):
    NORMAL = 0
    RESTART = 1
    HINT = 2
    UNDO = 3


# Extracted here since the logic is more complex than a match-case.
def encode_movetype(move: chess.Move, board: chess.Board) -> int:
    global MTYPE_NORMAL
    global MTYPE_PROMO
    global MTYPE_CAPTURE
    global MTYPE_CASTLE

    if (move.promotion != None):
        return MTYPE_PROMO
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

    if (move.promotion):
        packet |= (move.promotion << PTYPE_SHIFT)
    else:
        packet |= board.piece_type_at(move.from_square) << PTYPE_SHIFT

    if (board.is_castling(move) or board.is_en_passant(move)):
        packet |= (1 << M2_SHIFT)
        if board.is_castling(move):
            packet |= (1 << M2_PTYPE_SHIFT)
            packet |= ((7*(board.turn==chess.BLACK)) << M2_SRC_RANK_SHIFT)
            packet |= ((7*(board.turn==chess.BLACK)) << M2_DEST_RANK_SHIFT)
            packet |= ((7*board.is_kingside_castling(move)) << M2_SRC_FILE_SHIFT)
            packet |= ((3 + 2*board.is_kingside_castling(move)) << M2_DEST_FILE_SHIFT)
        else:
            packet |= (0 << M2_PTYPE_SHIFT)
            packet |= (chess.square_file(move.to_square) << M2_SRC_FILE_SHIFT)
            packet |= (chess.square_rank(move.from_square) << M2_SRC_RANK_SHIFT)
            packet |= (chess.square_file(move.to_square) << M2_DEST_FILE_SHIFT)
            packet |= (chess.square_rank(move.from_square) << M2_DEST_RANK_SHIFT)

    packet |= encode_movetype(move, board) << MTYPE_SHIFT
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

    dest_file = chess.square_file(move.to_square)
    packet |= dest_file << DEST_FILE_SHIFT
    dest_rank = chess.square_rank(move.to_square)
    packet |= dest_rank << DEST_RANK_SHIFT

    packet |= board.piece_type_at(move.from_square) << PTYPE_SHIFT
    packet |= (1 << M2_SHIFT) if (board.is_castling(move) or board.is_capture(move)) else (0 << M2_SHIFT) 


    undone_ptype = None
    undone_pcolor = 0

    if board.is_en_passant(move):
        undone_ptype = chess.PAWN # en passant captures can only capture pawns
        dest_file = chess.square_file(move.to_square)
        dest_rank = chess.square_rank(move.from_square)
        packet |= (dest_file << M2_SRC_FILE_SHIFT) | (dest_file << M2_DEST_FILE_SHIFT)
        packet |= (dest_rank << M2_SRC_RANK_SHIFT) | (dest_rank << M2_DEST_RANK_SHIFT)
    elif (board.is_capture(move)):
        undone_ptype = board.piece_type_at(move.to_square)
        undone_pcolor = (1 if board.color_at(move.from_square) == chess.WHITE else 0) << 3
        packet |= (dest_file << M2_SRC_FILE_SHIFT) | (dest_file << M2_DEST_FILE_SHIFT)
        packet |= (dest_rank << M2_SRC_RANK_SHIFT) | (dest_rank << M2_DEST_RANK_SHIFT)
    elif (board.is_castling(move)):
        undone_pcolor = (1 if board.color_at(move.from_square) == chess.BLACK else 0) << 3
        undone_ptype = chess.ROOK # The second piece type for castling can only be a rook
        packet |= ((7*(board.turn==chess.BLACK)) << M2_SRC_RANK_SHIFT)
        packet |= ((7*(board.turn==chess.BLACK)) << M2_DEST_RANK_SHIFT)
        packet |= ((7*board.is_kingside_castling(move)) << M2_SRC_FILE_SHIFT)
        packet |= ((3 + 2*board.is_kingside_castling(move)) << M2_DEST_FILE_SHIFT)

    if undone_ptype != None:
        packet |= undone_ptype

    packet |= undone_pcolor

    return packet


def encode_check(board: chess.Board) -> int:
    global MTYPE_SHIFT
    global M2_DEST_FILE_SHIFT
    global M2_DEST_RANK_SHIFT
    global PTYPE_SHIFT

    check_packet = 4

    king_square = board.king(board.turn)
    check_packet |= (chess.square_file(king_square) << M2_DEST_FILE_SHIFT)
    check_packet |= (chess.square_rank(king_square) << M2_DEST_RANK_SHIFT)

    return check_packet


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

    # if packet is tagged as promotion move
    if ((packet >> MTYPE_SHIFT) & 0x3) == 1:
        match ((packet >> PTYPE_SHIFT) & 0x7):
            case 2:
                move_str += 'n' # promoting to knight
            case 3:
                move_str += 'b' # promoting to bishop
            case 4:
                move_str += 'r' # promoting to rook
            case 5:
                move_str += 'q' # promoting to queen
            case _:
                pass # move string does not need to be appended

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


def send_legal(board: chess.Board, shandle: serial.Serial, log: bool=True):
    possible_moves = board.legal_moves
    possible_count = possible_moves.count()

    moves_dict = {}
    move_src_square = ''

    # Group moves into contiguous "chunks" based on source square
    for move in possible_moves:
        # .square_name method returns str (okay for dictionary key)
        move_src_square = chess.square_name(move.from_square)
        
        # Categorize moves by source square.
        # Offload this task to the RPI for easier move processing on the MSP.
        if move_src_square not in moves_dict.keys():
            moves_dict[move_src_square] = []

        moves_dict[move_src_square].append(encode_packet(move, board))

    packet_no = 0

    for src_square in moves_dict.keys():
        for packet in moves_dict[src_square]:
            packet_no += 1

            if packet_no == possible_count:
                packet |= 1 # Set the packet's LSB to mark it as the last packet in series
            
            if log:
                print(f"[DEBUG]: (wr.send_legal): sending packet 0x{packet:08X}")

            shandle.write(packet.to_bytes(4, 'little'))


def init_board(board: chess.Board, shandle: serial.Serial, log:bool=True):
    # Clear playing stack and refresh list of valid moves
    board.reset()
    send_legal(board, shandle, log=log)


if __name__ == '__main__':
    pass
