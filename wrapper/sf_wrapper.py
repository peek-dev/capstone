#!/usr/bin/env python3

import chess
import chess.engine
from enum import Enum


class ButtonEvent(Enum):
    NORMAL = 0
    RESTART = 1
    HINT = 2
    UNDO = 3


# TODO: build C-based UART extension module to enable importing here
# import rpi5_uartboost as uart

def encode_packet(move: chess.Move) -> int:
    # FIXME: implement full logic from stub
    return 0

def decode_packet(packet: int) -> chess.Move:
    # FIXME: implement full logic from stub
    return chess.Move.nullmove()

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

            # TODO: send packet

        # TODO: receive next packet from MSP

        next_move = chess.Move.null()

        match (parse_button_event(next_packet)):
            case ButtonEvent.RESTART:
                board.clear()
                continue
            case ButtonEvent.HINT:
                # TODO: send best move as packet again?
            case ButtonEvent.UNDO:
                undone_move = board.pop()
                # TODO: encode and send undo-formatted packet
                continue
            case _:
                next_move = # TODO: assign to result of packet decoding + chess.Move.from_uci(decoded_string)
                pass # No extra logic needed for standard move

        board.push_uci(next_move)

        # TODO: check post-move conditions: check, checkmate, stalemate

        # TODO: send specially-formatted packets to correspond to endgame conditions

    sf.quit()


