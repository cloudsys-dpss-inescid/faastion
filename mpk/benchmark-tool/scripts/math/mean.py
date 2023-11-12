#!/usr/bin/python3

import sys
import numpy

entries = []

for line in sys.stdin:
    entries.append(float(line.strip()))

print(numpy.mean(entries))
