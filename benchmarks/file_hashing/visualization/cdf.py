import matplotlib.pyplot as plt
import numpy as np

transitions = []
c_elapsed_time = []
java_elapsed_time = []
idx = 0

with open('../result.txt', 'r') as file:
    file_contents = file.read()

lines = file_contents.split('\n')


for i in range(0, len(lines), 3):
    transitions.append(lines[i])

for i in range(1, len(lines), 3):
    c_elapsed_time.append(float(lines[i]))

for i in range(2, len(lines), 3):
    java_elapsed_time.append(float(lines[i]))

transitions.pop()
transitions = [float(x) for x in transitions]
java_elapsed_time = [float(x) for x in java_elapsed_time]
c_elapsed_time = [float(x) for x in c_elapsed_time]
java_elapsed_time_array = np.array(java_elapsed_time)
c_elapsed_time_array = np.array(c_elapsed_time)
data_overhead = java_elapsed_time_array - c_elapsed_time_array

count, bins_count = np.histogram(data_overhead,bins=100000)
pdf = count / sum(count)
cdf = np.cumsum(pdf)
plt.xlabel("Time Elapsed for Data Transfer between Java and C (10 MB)")
plt.ylabel("Probability")
plt.plot(bins_count[1:], cdf, label="CDF") 
plt.xlim(0, 0.00002)  # Set custom X-axis limits
plt.ylim(0, 1)
plt.legend()
plt.show()
