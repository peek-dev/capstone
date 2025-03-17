#!/usr/bin/env python3
from collections import deque
import sys

class PseudoSerial:
    def __init__(self, reader = sys.stdin.buffer, writer = sys.stdout.buffer):
        self.r = reader
        self.w = writer

    def read(self, n):
        data = bytearray()
        while len(data) != n:
            newbytes = self.r.read(n-len(data))
            if len(newbytes) == 0:
                raise EOFError()
            data.extend(newbytes)
        return bytes(data)

    def write(self, data):
        out = self.w.write(data)
        self.w.flush()
        return out

    def read_until(self, patn):
        target = deque(patn)
        data = deque(self.read(len(patn)))
        all = bytearray()
        while True:
            # Check: found pattern?
            if data == target:
                break
            # Nope, grab another byte
            data.append(self.read(1)[0])
            all.append(data.popleft())
        all.extend(data)
        return bytes(all)

