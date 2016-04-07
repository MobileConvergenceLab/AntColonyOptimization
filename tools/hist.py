# -*- coding: utf-8 -*- 
import os
import sys
import numpy
import matplotlib.pyplot as plt

def read_log(logfile_prefix):
    l = list()
    i = 0
    try:
        while(True):
            logfile = '%s_%04d.log' % (logfile_prefix, i)
            with open(logfile, 'r') as f:
                lines = f.readlines()
                for line in lines:
                    l.append((int)(line))
            i = i+1
    except:
        pass

    return l

if __name__=='__main__':

    if len(sys.argv) == 1:
        print "Prefixes must be entered.(*MUST BE Directory name)"
        sys.exit(1)

    l = list()
    labels = list()
    
    for arg in sys.argv[1:]:
        l.append(read_log('./%s/' % arg))
        labels.append(arg)

    MAX = max(l[0])
    MIN = min(l[0])
    RANGE = numpy.arange(MIN-0.5, MAX+1.5)

    # make hist
    plt.hist(l, RANGE, histtype='bar', label=labels, alpha=0.5)

    # fill text
    plt.legend(loc='upper right')
    #plt.title(r'Comparison the proposal with the AS')
    plt.xlabel('The length of the path')
    plt.ylabel('The number of received packets(ants)')

    #
    plt.grid(True)
    plt.xticks(numpy.arange(MIN, MAX+1, 1.0))
    locs, labels = plt.xticks()
    plt.setp(labels, rotation=60)
    #plt.yscale('log', nonposy='clip')

    plt.savefig("hist.png", format="png")






