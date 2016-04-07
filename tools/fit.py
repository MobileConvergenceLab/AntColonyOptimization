# -*- coding: utf-8 -*- 
import os
import sys
import numpy
import numpy as np
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt

def read_log(logfile):
    l = list()
    with open(logfile, 'r') as f:
        lines = f.readlines()
        for line in lines:
            l.append((int)(line))

    return l

def read_all_logs(logfile_prefix):
    l = list()
    nfile = 0
    try:
        while(True):
            logfile = '%s_%04d.log' % (logfile_prefix, nfile)
            l.append(read_log(logfile))
            nfile = nfile + 1
    except:
        pass

    return l

def mean_log(logfile_prefix):
    all_logs = read_all_logs(logfile_prefix)
    mean = list()
    nlogs = len(all_logs)

    for i in range(0, len(all_logs[0])):
        s = 0
        for log in all_logs:
            s = s + log[i]
        mean.append(s)

    return [(i/float(nlogs))  for i in mean]


if __name__ == "__main__":
    l10 = mean_log(sys.argv[1])

    x = np.array( [ float(x) for x in range(0, len(l10))])
    y = np.array( l10 )

    def func(x, a, b, c):
        return a * np.exp(-b * x) + c

    popt, pcov = curve_fit(func, x, y)

    plt.figure()
    plt.plot(x, y, 'bo')
    plt.plot(x, func(x, *popt), 'r-', label="Fitted Curve", linewidth=5.0)
    plt.legend()
    plt.show()
