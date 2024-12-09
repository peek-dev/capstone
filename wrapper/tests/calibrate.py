#!/usr/bin/env python3

import serial
from datetime import datetime
import wrapper_util as wr
from pathlib import Path

outfile = 'calibration_data.txt'
srcdir = Path(__file__).parent
outpath = srcdir / outfile

def parse_file(packet: int) -> str:
    masked_file = (packet >> 29) & 0x7

    match (masked_file):
        case 0:
            return 'A'
        case 1:
            return 'B'
        case 2:
            return 'C'
        case 3:
            return 'D'
        case 4:
            return 'E'
        case 5:
            return 'F'
        case 6:
            return 'G'
        case 7:
            return 'H'
        case _:
            return 'X'


def parse_rank(packet: int) -> str:
    masked_rank = (packet >> 26) & 0x7
    return str(masked_rank + 1)


def parse_square(packet: int) -> str:
    return parse_file(packet) + parse_rank(packet)

def parse_min(packet: int) -> str:
    masked_min = (packet >> 16) & 0x1
    return str(bool(masked_min == 0))


def parse_max(packet: int) -> str:
    masked_max = (packet >> 16) & 0x1
    return str(bool(masked_max == 1))


def parse_value(packet: int) -> int:
    return (packet >> 4) & 0xFFF


def parse_color(packet: int) -> str:
    masked_color = (packet >> 3) & 0x1
    return 'B' if (masked_color == 1) else 'W'


def parse_ptype(packet: int) -> str:
    masked_type = packet & 0x7

    match (masked_type):
        case 1:
            return 'PAWN'
        case 2:
            return 'KNIGHT'
        case 3:
            return 'BISHOP'
        case 4:
            return 'ROOK'
        case 5:
            return 'QUEEN'
        case 6:
            return 'KING'
        case _:
            return 'X'

def heartbeat_response(uart):
    print("Got syn, sending synack.")
    uart.write(wr.SYNACK.to_bytes(4, 'little'))
    # "Flush out" Pi UART RX buffer until MSP's ACK is found
    sequence = uart.read_until(wr.MSP_ACK.to_bytes(4, 'little'))
    print("Got msp ack, starting.")

if __name__ == '__main__':
    if not outpath.exists():
        with open(outpath, 'a') as out:
            print("TIMESTAMP,SQUARE,COLOR,TYPE,VALUE,MAX?,MIN?", file=out)

    uart = serial.Serial('/dev/serial0', baudrate=115200)

    # "Heartbeat code" to establish connection with MSP
    heartbeat_msg = uart.read_until(wr.MSP_SYN.to_bytes(4, 'little'))

    print(f"Got heartbeat message: {heartbeat_msg}")

    heartbeat = int(heartbeat_msg[-1::-1].hex(), 16)

    print(f"Received heartbeat: {hex(heartbeat)}")

    uart.write(wr.SYNACK.to_bytes(4, 'little'))
    uart.write(wr.SYNACK.to_bytes(4, 'little'))

    print(f"Sent SYNACK: {hex(wr.SYNACK)}")

    read_sequence = uart.read_until(wr.MSP_ACK.to_bytes(4, 'little'))
    print(f"Got ACK sequence: {read_sequence}")

    while True:
        next_packet = int((uart.read(4))[-1::-1].hex(), 16)

        # Allow MSP restarts without restarting calibration code.
        if (next_packet == wr.MSP_SYN):
            heartbeat_response(uart)
            continue

        with open(outpath, 'a') as out:
            print(f"{datetime.now().isoformat()},{parse_square(next_packet)},{parse_color(next_packet)},{parse_ptype(next_packet)},{parse_value(next_packet)},{parse_max(next_packet)},{parse_min(next_packet)}", file=out)

        # read first packet
        # decode first packet
        # write first packet to file
        # read second packet
        # decode second packet
        # write second packet to file
