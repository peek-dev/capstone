#!/usr/bin/env python3

import chess
import chess.engine
from enum import Enum

class Event(Enum):
    NORMAL = 0,

# TODO: build C-based UART extension module to enable importing here
# import rpi5_uartboost

def send_move(move: chess.Move, is_last_move: bool=False) -> None:
    return


def get_next_move() -> chess.Move:
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

    while True:
        # Process main loop

        # 1. Fetch next packet from UART and decode

        # IF undo event:
        # undone_move = `board.pop()` or similar
        # Catch undo-on-empty-stack condition (IndexError)
        # if undone_move exists (i.e., IndexError not raised), encode undone move
        # and transmit to MSP over UART
        # else, encode NULL packet and send (?)

        # 1a. Validate move (???)

        # 2. push move to board stack: `board.push_uci()` or similar
        pass

    sf.quit()


