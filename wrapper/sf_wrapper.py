#!/usr/bin/env python3

import chess
import chess.engine
import wrapper_util as wr # include helpers and UART protocol constants
import rpi_uartboost as uart # include bespoke Raspberry Pi UART booster library
import serial

if __name__ == '__main__':
    board = chess.Board()

    # Open Stockfish in subprocess.
    # NOTE: on this build of Raspberry Pi OS/Debian, Stockfish is installed at
    # /usr/games/, NOT /usr/bin/
    sf = chess.engine.SimpleEngine.popen_uci("/usr/games/stockfish")

    # Device-specific customizations.
    # Code originally developed for Raspberry Pi 5 with 4 cores and 8GB RAM
    # (hence 'Hash' --> memory allocation --> 4096 MiB = 4 GiB).
    sf.configure({'Hash': 4096})
    sf.configure({'Threads': 4})

    # 250ms per-turn computation time limit
    sf_limit = chess.engine.Limit(time=0.250)

    uart = serial.Serial('/dev/serial0', baudrate=921600)

    # The main loop of the program, which will never exit except in unusual 
    # circumstances.
    while True:
        next_result = sf.play(board, sf_limit)
        best_move = next_result.move

        # Board has *not* been updated using .play() (only updated on .push()
        # call). 
        # 
        # So, find the best move and prepare to send that to the MSP alongside
        # *all* possible moves for the current board.
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

            moves_dict[move_src_square].append(wr.encode_packet(move, board).to_bytes(4, 'little'))

        packet_no = 0

        for src_square in moves_dict.keys():
            for packet in moves_dict[src_square]:
                packet_no += 1

                # Reshuffle makes pinpointing last move trickier than simple 
                # is-move-last-in-list logic
                if packet_no == possible_count:
                    packet = (int(packet.hex(), 16) | 0x01000000).to_bytes(4, 'little')
                    uart.write(packet)
                else:
                    uart.write(packet)

        # Block until the MSP sends back the next event it receives on the 
        # chessboard.
        next_packet = uart.read(4)
        next_move = chess.Move.null()

        # Three distinct possible kinds of packets (in a sense) exist to prompt
        # further action from the RPI:
        # - RESTART: clear game state (rewind entire game)
        # - HINT: send best move
        # - UNDO: rewind game state one move at a time
        match (wr.parse_button_event(int(next_packet.hex()))):
            case wr.ButtonEvent.RESTART:
                board.clear()
                continue
            case wr.ButtonEvent.HINT:
                uart.uart_sendpacket(wr.encode_packet(best_move, board, True).to_bytes(4, 'little'))
            case wr.ButtonEvent.UNDO:
                try:
                    # Rewinds the board state---no further action needed.
                    undone_move = board.pop()
                    # Indicate to the MSP which specific move has been undone
                    uart.uart_sendpacket(wr.encode_undo(undone_move, board).to_bytes(4, 'little'))
                    continue
                except IndexError:
                    # IndexError innocuous; stack is empty, but not an error
                    continue 
            case _:
                # Standard move
                next_move = wr.decode_packet(int(next_packet.hex(), 16)) 

        # Add move to the playing stack for Stockfish to use
        board.push(next_move)

        # Check endgame conditions and indicate to MSP if present
        if (board.is_checkmate()):
            uart.uart_sendpacket(wr.CHECKMATE.to_bytes(4, 'little'))
        elif (board.is_stalemate()):
            uart.uart_sendpacket(wr.STALEMATE.to_bytes(4, 'little'))

        # ...then proceed to the next turn

    sf.quit() # Code should never reach this---game loops forever.

