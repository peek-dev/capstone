#!/usr/bin/env python3

import chess
import chess.engine
from enum import Enum
import wrapper_util as wr # include UART-based protocol-specific constants

# include bespoke Raspberry Pi UART booster library
import rpi_uartboost as uart 


class ButtonEvent(Enum):
    NORMAL = 0
    RESTART = 1
    HINT = 2
    UNDO = 3


# Extracted here since the logic is more complex than a match-case.
def encode_movetype(move: chess.Move, board: chess.Board) -> int:
    if (board.is_check(move)):
        return wr.MTYPE_CHECK
    elif (board.is_capture(move)):
        return wr.MTYPE_CAPTURE
    if (board.is_castling(move)):
        return wr.MTYPE_CASTLE

    return wr.MTYPE_NORMAL


def encode_packet(move: chess.Move, board: chess.Board, last_move: bool=False) -> int:

    packet = 0

    packet |= chess.square_file(move.from_square) << wr.SRC_FILE_SHIFT
    packet |= chess.square_rank(move.from_square) << wr.SRC_RANK_SHIFT
    packet |= chess.square_file(move.to_square) << wr.DEST_FILE_SHIFT
    packet |= chess.square_rank(move.to_square) << wr.DEST_RANK_SHIFT
    packet |= (move.drop if (move.drop != None) else 0) << wr.PTYPE_SHIFT
    packet |= (1 << wr.M2_SHIFT) if board.is_castling(move) else (0 << wr.M2_SHIFT) 
    packet |= encode_movetype(move, board) << wr.MTYPE_SHIFT
    packet |= (1 if last_move else 0)

    return packet


def encode_undo(move: chess.Move, board: chess.Board) -> int:
    packet = 0
    packet |= chess.square_file(move.from_square) << wr.SRC_FILE_SHIFT
    packet |= chess.square_rank(move.from_square) << wr.SRC_RANK_SHIFT
    packet |= chess.square_file(move.to_square) << wr.DEST_FILE_SHIFT
    packet |= chess.square_rank(move.to_square) << wr.DEST_RANK_SHIFT
    packet |= (move.drop if (move.drop != None) else 0) << wr.PTYPE_SHIFT
    packet |= (1 << wr.M2_SHIFT) if board.is_castling(move) else (0 << wr.M2_SHIFT) 

    packet |= (1 if board.color_at(move.to_square) == chess.BLACK else 0) << 3
    undone_ptype = chess.PieceType(board.piece_at(move.to_square))
    packet |= undone_ptype if (undone_ptype != None) else 0

    return packet


def decode_packet(packet: int) -> chess.Move:
    move_str = ''

    move_str += chess.FILE_NAMES[(packet >> wr.SRC_FILE_SHIFT) & 0x7]
    move_str += chess.RANK_NAMES[(packet >> wr.SRC_RANK_SHIFT) & 0x7]
    move_str += chess.FILE_NAMES[(packet >> wr.DEST_FILE_SHIFT) & 0x7]
    move_str += chess.RANK_NAMES[(packet >> wr.DEST_RANK_SHIFT) & 0x7]

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


if __name __ == '__main__':

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

    # Cue extension libary to prepare UART for reading and writing
    uart.uart_init()

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
            else:
                moves_dict[move_src_square].append(encode_packet(move, board))

        packet_no = 0

        for src_square in moves_dict.keys():
            for packet in moves_dict[src_square]:
                packet_no += 1

                # Reshuffle makes pinpointing last move trickier than simple 
                # is-move-last-in-list logic
                if packet_no == possible_count:
                    uart.uart_sendpacket(packet | 0x1)
                else:
                    uart.uart_sendpacket(packet)

        # Block until the MSP sends back the next event it receives on the 
        # chessboard.
        next_packet = uart.uart_recvpacket()
        next_move = chess.Move.null()

        # Three distinct possible kinds of packets (in a sense) exist to prompt
        # further action from the RPI:
        # - RESTART: clear game state (rewind entire game)
        # - HINT: send best move
        # - UNDO: rewind game state one move at a time
        match (parse_button_event(next_packet)):
            case ButtonEvent.RESTART:
                board.clear()
                continue
            case ButtonEvent.HINT:
                uart.uart_sendpacket(encode_packet(best_move, board, True))
            case ButtonEvent.UNDO:
                try:
                    # Rewinds the board state---no further action needed.
                    undone_move = board.pop()
                    # Indicate to the MSP which specific move has been undone
                    uart.uart_sendpacket(encode_undo(undone_move, board))
                    continue
                except IndexError:
                    # IndexError innocuous; stack is empty, but not an error
                    continue 
            case _:
                # Standard move
                next_move = decode_packet(next_packet) 

        # Add move to the playing stack for Stockfish to use
        board.push(next_move)

        # TODO: check post-move conditions: check, checkmate, stalemate

        # TODO: send specially-formatted packets to correspond to endgame conditions

    sf.quit()


