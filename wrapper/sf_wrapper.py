#!/usr/bin/env python3

import chess
import chess.engine
import wrapper_util as wr # include helpers and UART protocol constants
from wrapper_util import ButtonEvent
import serial
from datetime import datetime
import sys  # for basic argparsing: argv[1] 


def push_msg(msg: str):
    print("[DEBUG]:", msg)


if __name__ == '__main__':
    debug = None

    try:
        if (sys.argv[1] == '-s') or ('-suppress' in sys.argv[1]):
            debug = False
        else:
            debug = True
    except IndexError:
        debug = True


    if debug:
        push_msg(f"STARTED at: {datetime.now().isoformat()}")

    MSP_BAUDRATE=115200 # UART state constant
    board = chess.Board()

    # Open Stockfish in subprocess.
    # NOTE: on this build of Raspberry Pi OS/Debian, Stockfish is installed at
    # /usr/games/, NOT /usr/bin/
    sf = chess.engine.SimpleEngine.popen_uci("/usr/games/stockfish")

    if debug:
        push_msg("Opened stockfish subprocess.")

    # Device-specific customizations.
    # Code originally developed for Raspberry Pi 4B with 4 cores and 8GB RAM
    # (hence 'Hash' --> memory allocation --> 4096 MiB = 4 GiB).
    sf.configure({'Hash': 4096})
    sf.configure({'Threads': 4})

    # 250ms per-turn computation time limit
    sf_limit = chess.engine.Limit(time=0.250)

    if debug:
        push_msg("Configured stockfish (memory, threads, time limit)")

    uart = serial.Serial('/dev/serial0', baudrate=MSP_BAUDRATE)

    if debug:
        push_msg("Waiting for heartbeat...")

    # "Heartbeat code" to establish connection with MSP
    heartbeat_msg = uart.read_until(wr.MSP_SYN.to_bytes(4, 'little'))

    if debug:
        push_msg(f"Got heartbeat message: {heartbeat_msg}")

    heartbeat = int(heartbeat_msg[-1::-1].hex(), 16)

    if debug:
        push_msg(f"Received heartbeat: {hex(heartbeat)}")

    uart.write(wr.SYNACK.to_bytes(4, 'little'))

    if debug:
        push_msg(f"Sent SYNACK: {hex(wr.SYNACK)}")

    # "Flush out" Pi UART RX buffer until MSP's ACK is found
    uart.read_until(wr.MSP_ACK.to_bytes(4, 'little'))
    
    if debug:
        push_msg(f"Received ACK ({hex(wr.MSP_ACK)}) from MSP.")

    # Now synchronized with MSP. Proceed with setup.
    wr.init_board(board, uart, log=debug)

    if debug:
        push_msg("Initialized board.")

    next_result = sf.play(board, sf_limit)
    best_move = next_result.move

    if debug:
        push_msg(f"Got initial best move: {best_move.uci()}")

    current_timestamp = datetime.now().isoformat()
    
    # The main loop of the program, which will never exit except in unusual 
    # circumstances.
    while True:
        if debug:
            push_msg("Waiting for next packet...")

        # Block until the MSP sends back the next event it receives on the 
        # chessboard.
        next_packet = int((uart.read(4))[-1::-1].hex(), 16)

        if debug:
            push_msg(f"Got packet: {hex(next_packet)}")

        # Sentinel move/dummy move ("done undoing"). No useful information encoded here, so continue.
        if (next_packet == 0x0):
            if debug:
                push_msg("Packet is sentinel move/dummy move (\"done undoing\").")

            wr.send_legal(board, uart, log=debug)
            continue

        next_move = chess.Move.null()

        # Three distinct possible kinds of packets (in a sense) exist to prompt
        # further action from the RPI:
        # - RESTART: clear game state (rewind entire game)
        # - HINT: send best move
        # - UNDO: rewind game state one move at a time
        match (wr.parse_button_event(next_packet)):
            case ButtonEvent.RESTART:
                if debug:
                    push_msg("Packet indicates RESTART.")

                data_handle = f"/opt/chessboard/{current_timestamp}-game.txt"
                
                with open(data_handle, 'w') as game_data:
                    for game_move in board.move_stack:
                        print(game_move.uci(), file=game_data)

                if debug:
                    push_msg(f"(Attempted to) log game data in file \"{data_handle}\".")
                    push_msg("Resetting board state...")

                wr.init_board(board, uart, log=log_path)

                if debug:
                    push_msg(f"Reset board state. Board now: TURN: {board.turn}, MOVES: {board.move_stack}")

                current_timestamp = datetime.now().isoformat()
                continue
            case ButtonEvent.HINT:
                if debug:
                    push_msg("Packet indicates HINT.")

                uart.write(wr.encode_packet(best_move, board, True).to_bytes(4, 'little'))
            case ButtonEvent.UNDO:
                try:
                    # Rewinds the board state---no further action needed.
                    undone_move = board.pop()

                    if debug:
                        push_msg(f"Popped move: {undone_move.uci()}")

                    # Indicate to the MSP which specific move has been undone
                    uart.write(wr.encode_undo(undone_move, board).to_bytes(4, 'little'))
                    continue
                except IndexError:
                    # IndexError innocuous; stack is empty, but not an error
                    continue 
            case _:
                # Standard move. Add move to playing stack and update state
                next_move = wr.decode_packet(next_packet)

                if debug:
                    push_msg("Packet is standard move.")
                    push_msg(f"Decoded as move \"{next_move.uci()}.\"")

                board.push(next_move)

                next_result = sf.play(board, sf_limit)
                best_move = next_result.move

                if debug:
                    push_msg(f"Got best move {best_move.uci()} in response to new move.")

                # Check endgame conditions and indicate to MSP if present
                if (board.is_checkmate()):
                    if debug:
                        push_msg("ENDGAME: board indicates CHECKMATE!")

                    uart.write(wr.CHECKMATE.to_bytes(4, 'little'))
                elif (board.is_stalemate()):
                    if debug:
                        push_msg("ENDGAME: board indicates STALEMATE!")

                    uart.write(wr.STALEMATE.to_bytes(4, 'little'))
                elif (board.is_check()):
                    check_packet = wr.encode_check(board)
                    
                    if debug:
                        push_msg(f"CHECK! Sending packet {hex(check_packet)}")

                    uart.write(check_packet.to_bytes(4, 'little'))

                # Send all legal moves for the new board state
                wr.send_legal(board, uart, log=debug)

        # ...then proceed to the next turn

    sf.quit() # Code should never reach this---game loops forever.

    if debug:
        push_msg("CRITICAL ERROR: script exited!")


