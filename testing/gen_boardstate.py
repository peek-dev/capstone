#!/usr/bin/env python3
from fenparser import Position

ptype_to_n = {c: i for (i, c) in enumerate('PRNBQK kqbnrp')}

extrasuffix = "w - - 15 44"
# example: "rnbqkbnr/ppppp1pp/5p2/8/8/8/PPPPPPPP/RNBQKBNR"
position = input("FEN Position: \n").strip()

pos = Position(f"{position} {extrasuffix}")


boardstate = []

for row in range(8):
    row_packed = 0
    for col in range(8):
        row_packed |= ptype_to_n[pos.board[7-row][col]] << (4*col)
    boardstate.append(row_packed)

print(boardstate)
