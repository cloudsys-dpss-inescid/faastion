#!/usr/bin/python3

import os
import sys
import numpy as np

if len(sys.argv) < 2:
	sys.exit("Sytanx: " + sys.argv[0] + " <latency_breakdown.txt>")

filename = sys.argv[1]

with open(filename, 'r') as f:
    timestamps = [int(line.strip()) for line in f.readlines()]

line = 0
enter_domain_times = []
notify_worker_times = []
function_times = []
notify_user_times = []
leave_domain_times = []
call_gate_times = []
while line < len(timestamps):
    iteration_start = line
    enter_domain_time = timestamps[line+1] - timestamps[line]
    line += 1
    notify_worker_time = timestamps[line+1] - timestamps[line]
    line +=1
    function_time = timestamps[line+1] - timestamps[line]
    line += 1
    notify_user_time = timestamps[line+1] - timestamps[line]
    line += 1
    leave_domain_time = timestamps[line+1] - timestamps[line]
    call_gate_time = timestamps[line+1] - timestamps[iteration_start]
    line += 2
    enter_domain_times.append(enter_domain_time)
    notify_worker_times.append(notify_worker_time)
    function_times.append(function_time)
    notify_user_times.append(notify_user_time)
    leave_domain_times.append(leave_domain_time)
    call_gate_times.append(call_gate_time)

enter_domain_times_array = np.array(enter_domain_times)
notify_worker_times_array = np.array(notify_worker_times)
function_times_array = np.array(function_times)
notify_user_times_array = np.array(notify_user_times)
leave_domain_times_array = np.array(leave_domain_times)
call_gate_times_array = np.array(call_gate_times)

print("call gate time: " + str(np.mean(call_gate_times_array)))
print("enter domain time: " + str(np.mean(enter_domain_times_array)))
print("notify worker time: " + str(np.mean(notify_worker_times_array)))
print("function time: " + str(np.mean(function_times_array)))
print("notify user time: " + str(np.mean(notify_user_times_array)))
print("leave domain time: " + str(np.mean(leave_domain_times_array)))

