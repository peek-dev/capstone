#!/usr/bin/env python3

# Various constants based on packet scheme. See
# `/uart-protocol/uart_bidir_protocol.h` and `.c` for more information.
SRC_FILE_SHIFT = 29
SRC_RANK_SHIFT = 26
DEST_FILE_SHIFT = 23
DEST_RANK_SHIFT = 20
PTYPE_SHIFT = 17

M2_SHIFT = 16
M2_SRC_FILE_SHIFT = 13
M2_SRC_RANK_SHIFT = 10
M2_DEST_FILE_SHIFT = 7
M2_DEST_RANK_SHIFT = 4
M2_PTYPE_SHIFT = 1

MTYPE_SHIFT = 2
MTYPE_NORMAL = 0
MTYPE_CHECK = 1
MTYPE_CAPTURE = 2
MTYPE_CASTLE = 3

# Special status constants to indicate exceptional game conditions (outcomes,
# most often) for the MSP.
CHECK       = 0x00000010
CHECKMATE   = 0x00000030
STALEMATE   = 0x00000070


if __name__ == '__main__':
    pass
