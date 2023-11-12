import matplotlib.pyplot as plt
import numpy as np

x_axis = []
y_axis = []
idx = 0
with open('exec.txt', 'r') as file:
    for line in file:
        y_axis.append(float(line.strip()))
        x_axis.append(idx)
        idx+=1
plt.rc('font', size=14)
plt.plot(x_axis,y_axis)
plt.yticks(np.arange(0.05, max(y_axis)+0.05, step=0.005))
plt.xlabel('No. of Iterations')
plt.ylabel('Time Elapsed (milliseconds)')
plt.text(110, 0.12, 'Representation of NOP execution with no parameters')
plt.show()
