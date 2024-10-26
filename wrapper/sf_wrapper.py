#!/usr/bin/env python3

import chess
import chess.engine
from enum import Enum

# include bespoke Raspberry Pi UART booster library
import rpi5_uartboost as uart 

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
    # FIXME: implement full logic from stub
    return 0

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


def send_move(move: chess.Move, is_last_move: bool=False) -> None:
    return


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

        # TODO: send best move "a priori" either at very beginning 
        # or very end (?)
        for move in possible_moves:
            if (move == best_move):
                # Special logic: skip?
                continue

            uart.uart_sendpacket(encode_packet(move))

        uart.uart_sendpacket(encode_packet(best_move, True))

        next_packet = uart.uart_recvpacket()
        next_move = chess.Move.null()

        match (parse_button_event(next_packet)):
            case ButtonEvent.RESTART:
                board.clear()
                continue
            case ButtonEvent.HINT:
                # TODO: send best move as packet again?
            case ButtonEvent.UNDO:
                try:
                    undone_move = board.pop()
                    uart.uart_sendpacket(encode_packet(undone_move, True))
                    continue
                except IndexError:
                    continue # Silently handle IndexError (move stack empty) since cause innocuous
            case _:
                next_move = decode_packet(next_packet) 
                pass # No extra logic needed for standard move

        board.push_uci(next_move)

        # TODO: check post-move conditions: check, checkmate, stalemate

        # TODO: send specially-formatted packets to correspond to endgame conditions

    sf.quit()


