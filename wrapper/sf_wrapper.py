#!/usr/bin/env python3

import chess
import chess.engine
import wrapper_util as wr # include helpers and UART protocol constants
from wrapper_util import ButtonEvent
import serial
from datetime import datetime

if __name__ == '__main__':
 
    MSP_BAUDRATE=115200 # UART state constant
    board = chess.Board()

    # Open Stockfish in subprocess.
    # NOTE: on this build of Raspberry Pi OS/Debian, Stockfish is installed at
    # /usr/games/, NOT /usr/bin/
    sf = chess.engine.SimpleEngine.popen_uci("/usr/games/stockfish")

    # Device-specific customizations.
    # Code originally developed for Raspberry Pi 4B with 4 cores and 8GB RAM
    # (hence 'Hash' --> memory allocation --> 4096 MiB = 4 GiB).
    sf.configure({'Hash': 4096})
    sf.configure({'Threads': 4})

    # 250ms per-turn computation time limit
    sf_limit = chess.engine.Limit(time=0.250)

    uart = serial.Serial('/dev/serial0', baudrate=MSP_BAUDRATE)

    # "Heartbeat code" to establish connection with MSP
    heartbeat = int((uart.read(4))[-1::-1].hex(), 16)

    if (heartbeat == wr.MSP_SYN):
        uart.write(wr.SYNACK.to_bytes(4, 'little'))

    # "Flush out" Pi UART RX buffer until MSP's ACK is found
    sequence = uart.read_until(wr.MSP_ACK.to_bytes(4, 'little'))
    
    # Now synchronized with MSP. Proceed with setup.
    wr.init_board()
    next_result = sf.play(board, sf_limit)
    best_move = next_result.move

    current_timestamp = datetime.now().isoformat()
    
    # The main loop of the program, which will never exit except in unusual 
    # circumstances.
    while True:

        # Block until the MSP sends back the next event it receives on the 
        # chessboard.
        next_packet = int((uart.read(4))[-1::-1].hex(), 16)
        next_move = chess.Move.null()

        # Three distinct possible kinds of packets (in a sense) exist to prompt
        # further action from the RPI:
        # - RESTART: clear game state (rewind entire game)
        # - HINT: send best move
        # - UNDO: rewind game state one move at a time
        match (wr.parse_button_event(next_packet)):
            case ButtonEvent.RESTART:
                data_handle = f"{current_timestamp}-game.txt"
                
                with open(data_handle, 'w') as game_data:
                    for game_move in board.move_stack:
                        print(uci(game_move), file=game_data)

                board.reset()
                current_timestamp = datetime.now().isoformat()
                continue
            case ButtonEvent.HINT:
                uart.write(wr.encode_packet(best_move, board, True).to_bytes(4, 'little'))
            case ButtonEvent.UNDO:
                try:
                    # Rewinds the board state---no further action needed.
                    undone_move = board.pop()
                    # Indicate to the MSP which specific move has been undone
                    uart.write(wr.encode_undo(undone_move, board).to_bytes(4, 'little'))
                    continue
                except IndexError:
                    # IndexError innocuous; stack is empty, but not an error
                    continue 
            case _:
                # Standard move. Add move to playing stack and update state.
                next_move = wr.decode_packet(next_packet)
                board.push(next_move)

                next_result = sf.play(board, sf_limit)
                best_move = next_result.move

                # Check endgame conditions and indicate to MSP if present
                if (board.is_checkmate()):
                    uart.write(wr.CHECKMATE.to_bytes(4, 'little'))
                elif (board.is_stalemate()):
                    uart.write(wr.STALEMATE.to_bytes(4, 'little'))

                # Send all legal moves for the new board state
                wr.send_legal(board, uart)

        # TODO: add normal status packet as "heartbeat"?


        # ...then proceed to the next turn

    sf.quit() # Code should never reach this---game loops forever.

