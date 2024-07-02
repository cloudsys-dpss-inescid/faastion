#!/usr/bin/python

import matplotlib
import matplotlib.pyplot as plt
import numpy as np

def read_avg_invocations(path):
    invocations = []
    try:
        with open(path) as file:
            for line in file:
                invocations.append(float(line.strip()))
    except Exception as e:
        print("Error processing " + path + ":")
        raise e
    return invocations

def plot_invocations(userfile, funcfile, outprefix):
    avg_user_invocations = read_avg_invocations(userfile)
    avg_func_invocations = read_avg_invocations(funcfile)

    x_user = np.sort(avg_user_invocations)
    x_func = np.sort(avg_func_invocations)
  
    # Get the cdf values of y.
    y_user = np.arange(len(avg_user_invocations)) / float(len(avg_user_invocations))
    y_func = np.arange(len(avg_func_invocations)) / float(len(avg_func_invocations))
  
    matplotlib.rcParams.update({'font.size': 16})
    plt.rcParams["figure.figsize"] = (10, 4)
    plt.plot(x_user, y_user, linestyle='--',   linewidth=3, label="User Invocations")
    plt.plot(x_func, y_func, linestyle='-',  linewidth=3, label="Function Invocations")
    plt.grid()
    plt.xlabel('Invocations per Minute')
    plt.ylabel('Cumulative Distribution Function')
    plt.ylim(ymin=0, ymax=1)
    plt.xscale('log', base=10)
    plt.xlim(xmax=1000)
    plt.legend(loc='lower right')
    plt.savefig(outprefix + '.pdf', bbox_inches='tight')
    plt.savefig(outprefix + '.png', bbox_inches='tight', dpi=300)
    plt.clf()

plot_invocations('avg_user_invocations_d01.dat', 'avg_func_invocations_d01.dat', 'azure-cdf-invocations-d01')
plot_invocations('avg_user_invocations_d02.dat', 'avg_func_invocations_d02.dat', 'azure-cdf-invocations-d02')
plot_invocations('avg_user_invocations_d03.dat', 'avg_func_invocations_d03.dat', 'azure-cdf-invocations-d03')
