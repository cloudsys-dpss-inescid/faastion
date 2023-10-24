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
figure, axis = plt.subplots(1,2)
# axis[0,0].rc('font', size=14)
axis[0,0].title("Transitions between Java code and Native Code (10 MB)")
axis[0,0].xlabel("No. of Transitions")
axis[0,0].ylabel("Time Elapsed (in seconds)")
axis[0,0].plot(transitions,java_elapsed_time)

# plt.figure(2)
axis[0,1].plot(transitions,data_overhead)
axis[0,1].title("Cost of Data Transfer between Java Code and Native Code (10 MB)")
axis[0,1].xlabel("No. of Transitions")
axis[0,1].ylabel("Time Elapsed (in seconds)")
plt.show()
