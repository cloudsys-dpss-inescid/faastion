#!/usr/bin/python

import sys
import numpy

entries = []

for line in sys.stdin:
    entries.append(float(line.strip()))

print(numpy.std(entries))
