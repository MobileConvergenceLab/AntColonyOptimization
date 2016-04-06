# -*- coding: utf-8 -*- 
import os
import sys
import numpy
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
            logfile = '%s_%02d.log' % (logfile_prefix, nfile)
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

if __name__=='__main__':

    if len(sys.argv) == 1:
        print "Prefixes must be entered.(*MUST BE Directory name)"
        sys.exit(1)

    for arg in sys.argv[1:]:
        l = mean_log('./%s/' % (arg))
        plt.plot(l, label=arg)

    plt.legend(loc='upper right')
    plt.grid(True)
    plt.title(r'No title')
    plt.xlabel('N-th packet(ant)')
    plt.ylabel('The average length of the path')

    plt.savefig("time.png", format="png")



