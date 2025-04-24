#!/usr/bin/env python3

#
# Copyright (C) 2024 Paul D. Karhnak
#
# sf_wrapper.py: the main event loop that the Raspberry Pi runs to process 
# and compute chess logic for the C.H.E.S.S.B.O.A.R.D. and communicate with 
# the MSPM0G3507.
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>
#

import chess
import chess.engine
import serial

from datetime import datetime
import sys  # for basic argparsing: argv[1] 
import time
import signal

import stdio_serial
import wrapper_util as wr # include helpers and UART protocol constants
from wrapper_util import ButtonEvent
from sqd_sim import *

if __name__ == '__main__':
    prevent_ctrlc_in_sim()
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
    sf.configure({'Hash': 1024})
    sf.configure({'Threads': 4})

    # 250ms per-turn computation time limit
    sf_limit = chess.engine.Limit(time=0.250)

    if debug:
        push_msg("Configured stockfish (memory, threads, time limit)")

    # If we are running in the simulator, don't use a real serial port.
    # Instead, use the stdio to communicate. The simulator will redirect
    # the channels appropriately.
    if IN_SIM:
        uart = stdio_serial.PseudoSerial()
    else:
        uart = serial.Serial('/dev/serial0', baudrate=MSP_BAUDRATE)

    msp_alive = True

    while True:

        if debug:
            push_msg("Waiting for heartbeat...")

        # "Heartbeat code" to establish connection with MSP
        heartbeat_msg = uart.read_until(wr.MSP_SYN.to_bytes(4, 'little'))

        if debug:
            push_msg(f"Got heartbeat message: {heartbeat_msg}")

        heartbeat = int(heartbeat_msg[-1:-5:-1].hex(), 16)

        if debug:
            push_msg(f"Received heartbeat: {hex(heartbeat)}")

        uart.write(wr.SYNACK.to_bytes(4, 'little'))
        uart.write(wr.SYNACK.to_bytes(4, 'little'))

        if debug:
            push_msg(f"Sent SYNACK: {hex(wr.SYNACK)}")

        uart.read_until(wr.MSP_ACK.to_bytes(4, 'little'))

        if debug:
            push_msg(f"Received ACK ({hex(wr.MSP_ACK)}) from MSP.")

        # Now synchronized with MSP. Proceed with setup.
        wr.init_board(board, uart, log=debug)

        if debug:
            push_msg("Initialized board.")

        next_result = None

        while (next_result == None):
            try:
                next_result = sf.play(board, sf_limit)
            except TimeoutError:
                pass

        best_move = next_result.move

        if debug:
            push_msg(f"Got initial best move: {best_move.uci()}")

        current_timestamp = datetime.now().isoformat()
        current_timestamp = current_timestamp.replace(':', '-')
        if IN_SIM:
            data_handle_dir = os.environ.get('SF_WRAPPER_PY_GAMEDIR', '')
        else:
            data_handle_dir = "/opt/chessboard/"
        data_handle = f"{data_handle_dir}{current_timestamp}-game.txt"
    
        # The main loop of the program, which will never exit except in unusual 
        # circumstances.
        while msp_alive:
            if debug:
                push_msg("Waiting for next packet...")

            # Block until the MSP sends back the next event it receives on the 
            # chessboard.
            next_packet = int((uart.read(4))[-1::-1].hex(), 16)

            if debug:
                push_msg(f"Got packet: {hex(next_packet)}")

            # Sentinel move/dummy move ("done undoing"). No useful information encoded here, so continue.
            if (next_packet == wr.MSP_SYN):
                push_msg("Packet indicates MSP reset! Resetting state...")
                msp_alive = False
                continue
            if (next_packet == wr.MSP_REREQ):
                if debug:
                    push_msg("Packet is sentinel move/dummy move (\"done undoing\").")

                wr.send_legal(board, uart, log=debug)
                continue
            if IN_SIM and (next_packet == wr.SIM_EXIT):
                if debug:
                    push_msg("Sim requests program exit, closing out.")
                sf.quit()
                exit(0)

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
 
                    with open(data_handle, 'w') as game_data:
                        for game_move in board.move_stack:
                            print(game_move.uci(), file=game_data)

                    if debug:
                        push_msg(f"(Attempted to) log game data in file \"{data_handle}\".")
                        push_msg("Resetting board state...")

                    wr.init_board(board, uart, log=debug)

                    next_result = None

                    while (next_result == None):
                        try:
                            next_result = sf.play(board, sf_limit)
                        except TimeoutError:
                            pass 

                    best_move = next_result.move

                    if debug:
                        push_msg(f"Reset board state. Board now: TURN: {board.turn}, MOVES: {board.move_stack}")

                    current_timestamp = datetime.now().isoformat()
                    current_timestamp = current_timestamp.replace(':', '-')
                    data_handle = f"{data_handle_dir}{current_timestamp}-game.txt"
                    continue
                case ButtonEvent.HINT:
                    if debug:
                        push_msg("Packet indicates HINT.")

                    if (board.is_checkmate() or board.is_stalemate()): 
                        continue

                    encoded_best_move = wr.encode_packet(best_move, board, True)

                    if debug:
                        push_msg(f"Providing {encoded_best_move:08x} as best move.")

                    uart.write(encoded_best_move.to_bytes(4, 'little'))
                case ButtonEvent.UNDO:
                    try:
                        # Rewinds the board state---no further action needed.
                        undone_move = board.pop()
                        undone_move_packet = wr.encode_undo(undone_move, board)

                        if debug:
                            push_msg(f"Popped move: {undone_move.uci()}")
                            if undone_move != None:
                                push_msg(f"Sending undone move: 0x{undone_move_packet:08x}")

                        next_result = None

                        while (next_result == None):
                            try:
                                next_result = sf.play(board, sf_limit)
                            except TimeoutError:
                                pass

                        best_move = next_result.move
                        
                        # Indicate to the MSP which specific move has been undone
                        uart.write(undone_move_packet.to_bytes(4, 'little'))
                    except IndexError:
                        # IndexError innocuous; stack is empty, but not an error
                        push_msg("Move stack empty--can't undo! Sending undo exhaust sentinel {wr.UNDO_EXHAUST:08x}.")
                        uart.write(wr.UNDO_EXHAUST.to_bytes(4, 'little'))
                    finally:
                        # Log the updated move transcript
                        with open(data_handle, 'w') as game_data:
                            for game_move in board.move_stack:
                                print(game_move.uci(), file=game_data)

                        continue
                case _:
                    # Standard move. Add move to playing stack and update state
                    next_move = wr.decode_packet(next_packet)

                    if debug:
                        push_msg("Packet is standard move.")
                        push_msg(f"Decoded as move \"{next_move.uci()}.\"")

                    board.push(next_move)

                    with open(data_handle, 'w') as game_data:
                        for game_move in board.move_stack:
                            print(game_move.uci(), file=game_data)

                    next_result = None

                    while (next_result == None):
                        try:
                            if debug:
                                push_msg("Trying to play move.")
                            next_result = sf.play(board, sf_limit)
                        except TimeoutError:
                            pass

                    best_move = next_result.move

                    if debug:
                        if best_move:
                            push_msg(f"Got best move {best_move.uci()} in response to new move.")
                        else:
                            push_msg(f"Best move is null. Checkmate or stalemate present.")

                    # Check endgame conditions and indicate to MSP if present
                    if (board.is_checkmate()):
                        if debug:
                            push_msg("ENDGAME: board indicates CHECKMATE!")

                        uart.write(wr.CHECKMATE.to_bytes(4, 'little'))
                    elif (board.is_stalemate() or board.is_insufficient_material() or board.is_seventyfive_moves() or board.is_fivefold_repetition()):
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


