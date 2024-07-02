#!/bin/python

import matplotlib.pyplot as plt

import numpy as np
import pandas as pd
import sys

import math

def convert_size(size_bytes):
   if size_bytes == 0:
       return "0B"
   size_name = ("B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB")
   i = int(math.floor(math.log(size_bytes, 1024)))
   p = math.pow(1024, i)
   s = int(size_bytes / p)
   return "%s %s" % (s, size_name[i])


if len(sys.argv) < 2:
    print(f'usage: {sys.argv[0]} csv_to_plot')
    sys.exit(1)

res = pd.read_csv(sys.argv[1])

x = list(map(convert_size, res['block_size']))
y1 = res['dd+netcat']
y2 = res['dd+openssl+netcat']

X_axis = np.arange(len(x))

plt.bar(X_axis - 0.2, y1, 0.4, label = 'Plain')
plt.bar(X_axis + 0.2, y2, 0.4, label = 'Encrypted')

plt.xticks(X_axis, x)
plt.xlabel("Block size")
plt.ylabel("t (ms)")
plt.legend()

plt.savefig('plot.png')