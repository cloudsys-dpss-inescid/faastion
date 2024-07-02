#!/usr/bin/python
import os
import pathlib
from datetime import datetime
import matplotlib.pyplot as plt

class syscall_dump:
    def __init__(self, file):
        self.read_dump(file)

    def read_dump(self, file):
        try:
            f = open(file, 'r')
            data = f.read().split('\n')
            self.syscalls = {}
            for i in range(2, len(data)-3):
                cols = data[i].split()
                syscall_name = cols[-1]
                self.syscalls[syscall_name] = int(cols[-2]) if len(cols) == 5 else int(cols[-3])  
            # execve is called by strace
            self.syscalls["execve"] -= 1
            if self.syscalls["execve"] == 0:
                del self.syscalls["execve"]
            f.close()
        except IOError as e:
            print(e)
    
    def plot_histogram(self):
        plt.rcParams.update({'figure.autolayout': True})
        plt.style.use('ggplot')
        fig = plt.figure()
        plt.tight_layout()
        plt.xticks(fontsize = 8, rotation = 90)
        plt.xlabel("Syscall intercepted")
        plt.ylabel("Frequency")
        plt.title("Micronaut Shopcart HTTP Post")
        plt.bar(list(self.syscalls.keys()), list(self.syscalls.values()), color='steelblue')
        output = datetime.now().strftime("HISTOGRAM_%Y%m%d_%H_%M_%S.jpg")
        plt.savefig(os.path.join("plots", output), dpi=300)

if __name__ == "__main__":
    try:
        pathlib.Path('plots').mkdir()
    except FileExistsError:
        pass
    
    syscalls = syscall_dump(os.path.join("strace", "strace.out"))
    syscalls.plot_histogram()
