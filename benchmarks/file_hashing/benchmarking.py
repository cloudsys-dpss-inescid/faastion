import subprocess
import datetime

command = ["make"]


subprocess.run(command, check=True)


with open('result.txt', 'r') as file:
    lines = file.readlines()
    elapsed_seconds = float(lines[-1].strip())   
    transitions = float(lines[-2].strip())

print("Elapsed time:", elapsed_seconds)
print("Number of transitions per second:", transitions/elapsed_seconds)
