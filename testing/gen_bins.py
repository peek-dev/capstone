#!/usr/bin/env python3
import numpy as np
from pathlib import Path
import sys
import re

maxes = np.zeros([8,8,13], dtype=np.int16)
mins = np.zeros([8,8,13], dtype=np.int16)
bins = np.zeros([8,8,13], dtype=np.int16)

ptype_to_pnum = {'PAWN': 0, 'ROOK': 1, 'KNIGHT': 2, 'BISHOP': 3, 'QUEEN': 4, 'KING': 5, 'X': 6}
def ptype_to_index(w_or_b, ptype):
    pnum = ptype_to_pnum[ptype]
    if w_or_b == 'W':
        return pnum
    else:
        return 12-pnum

def square_to_indices(square):
    row = int(square[1])-1
    col = ord(square[0])-ord('A')
    return (row, col)

def process_line(line):
    # Example line: "2024-12-05T18:13:44.170149,A2,B,PAWN,4062,True,False"
    fields = line.split(',')
    (row, col) = square_to_indices(fields[1])
    p_index = ptype_to_index(fields[2], fields[3])
    n = int(fields[4])
    is_max = fields[5] == 'True'
    is_min = fields[6] == 'True'
#    print(line)
    assert is_max != is_min
    if is_max:
        maxes[row,col,p_index] = n
    else:
        mins[row,col,p_index] = n


infile = Path(sys.argv[1])
if not infile.exists():
    print(f"Err: file does not exist: {sys.argv[1]}")
    exit(0)


with open(infile, 'r') as f:
    # Discard header line.
    f.readline()
    for line in f:
        process_line(line.strip())

# mins and maxes now contain all our calibration measurements.
for i in range(0,12):
    bins[:,:,i+1] = (maxes[:,:,i]+mins[:,:,i+1])/2

#print(mins)
#print(maxes)
print(bins)
bins_good = (bins[:,:,1:]-bins[:,:,:-1]) < 30
if np.any(bins_good):
    print("Error! Bins too small!")
    print(bins_good)

bins_str = f"{bins}"
bins_str_c = re.sub('},},},', '}}}', re.sub(r'\]', '},', re.sub(r'\[', r'{', bins_str)))
print("C format:")
print(f"uint16_t bins[][][] =\n{bins_str_c};")
