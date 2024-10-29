#!/usr/bin/env python3

import chess
import chess.engine
from enum import Enum

# include bespoke Raspberry Pi UART booster library
import rpi_uartboost as uart 

SRC_FILE_SHIFT = 29
SRC_RANK_SHIFT = 26
DEST_FILE_SHIFT = 23
DEST_RANK_SHIFT = 20

CHECK       = 0x00000010
CHECKMATE   = 0x00000030
STALEMATE   = 0x00000070


class ButtonEvent(Enum):
    NORMAL = 0
    RESTART = 1
    HINT = 2
    UNDO = 3


def encode_packet(move: chess.Move, last_move: bool=False) -> int:
    packet = 0

    packet |= chess.square_file(move.from_square) << SRC_FILE_SHIFT
    packet |= chess.square_rank(move.from_square) << SRC_RANK_SHIFT
    packet |= chess.square_file(move.to_square) << DEST_FILE_SHIFT
    packet |= chess.square_rank(move.to_square) << DEST_RANK_SHIFT

    # FIXME: implement full logic from stub
    return packet

def decode_packet(packet: int) -> chess.Move:
    move_str = ''

    move_str += chess.FILE_NAMES[(packet >> SRC_FILE_SHIFT) & 0x7]
    move_str += chess.RANK_NAMES[(packet >> SRC_RANK_SHIFT) & 0x7]
    move_str += chess.FILE_NAMES[(packet >> DEST_FILE_SHIFT) & 0x7]
    move_str += chess.RANK_NAMES[(packet >> DEST_RANK_SHIFT) & 0x7]

    return chess.Move.from_uci(move_str)


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


if __name __ == '__main__':

    board = chess.Board()

    # Open Stockfish in subprocess.
    # NOTE: on this build of Raspberry Pi OS/Debian, Stockfish is installed at
    # /usr/games, NOT /usr/bin
    sf = chess.engine.SimpleEngine.popen_uci("/usr/games/stockfish")

    # Provision 4 threads and 4096 MB (4 GB) of RAM to Stockfish. Raspberry Pi 5's 
    # ARM Cortex-A76 has 4 cores, and the targeted RPi5 has 8GB RAM available.
    sf.configure({'Hash': 4096})
    sf.configure({'Threads': 4})

    # A limit of 250 ms for Stockfish's per-turn compute time, wrapped in the
    # python-chess API.
    sf_limit = chess.engine.Limit(time=0.250)

    # Cue extension libary to prepare UART for reading and writing
    uart.uart_init()

    while True:
        next_result = sf.play(board, sf_limit)
        best_move = next_result.move
        possible_moves = board.legal_moves
        possible_count = possible_moves.count()

        moves_dict = {}
        move_src_square = ''

        # Group moves into contiguous "chunks" based on source square
        for move in possible_moves:
            move_src_square = chess.square_name(move.from_square)
            if move_src_square not in moves_dict.keys():
                moves_dict[move_src_square] = []
            else:
                moves_dict[move_src_square].append(encode_packet(move))

        packet_no = 0

        for src_square in moves_dict.keys():
            for packet in moves_dict[src_square]:
                packet_no += 1

                if packet_no == possible_count:
                    uart.uart_sendpacket(packet | 0x1)
                else:
                    uart.uart_sendpacket(packet)

        next_packet = uart.uart_recvpacket()
        next_move = chess.Move.null()

        match (parse_button_event(next_packet)):
            case ButtonEvent.RESTART:
                board.clear()
                continue
            case ButtonEvent.HINT:
                uart.uart_sendpacket(encode_packet(best_move, True))
            case ButtonEvent.UNDO:
                try:
                    undone_move = board.pop()
                    uart.uart_sendpacket(encode_packet(undone_move, True))
                    continue
                except IndexError:
                    continue # Silently handle IndexError (move stack empty) since cause innocuous
            case _:
                # No extra logic needed for standard move---decode and continue
                next_move = decode_packet(next_packet) 

        board.push(next_move)

        # TODO: check post-move conditions: check, checkmate, stalemate

        # TODO: send specially-formatted packets to correspond to endgame conditions

    sf.quit()


