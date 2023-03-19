#!/usr/bin/python3

import numpy
import matplotlib.pyplot as plt

transitions = numpy.loadtxt('results/transitions.dat')
percentages = numpy.loadtxt('results/percentages.dat')
labels      = numpy.loadtxt('results/benchmarks.dat', dtype='str')

def save_plot(subject, y_label):
    plt.bar(labels, eval(subject), label=subject)
    plt.ylim(ymin=0)
    plt.legend()
    plt.xlabel('Benchmarks')
    plt.xticks(rotation=45)
    plt.yscale('log')
    plt.ylabel(y_label)
    plt.tight_layout()
    plt.savefig(f'{subject}.pdf')

if __name__ == "__main__":
    save_plot("transitions", "Number of transitions per second")
    plt.clf()
    save_plot("percentages", "Time spent in native execution (%)")