import sys
import os
import numpy
import operator

def read_log(logfile):
    l = list()
    with open(logfile, 'r') as f:
        lines = f.readlines()
        for line in lines:
            l.append((int)(line))

    return numpy.mean(l)

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

if __name__ == "__main__":

    if len(sys.argv) == 1:
        print "Prefixes must be entered.(*MUST BE Directory name)"
        sys.exit(1)

    for arg in sys.argv[1:]:
        l = read_all_logs('./%s/' % arg)
        min_index, min_value = min(enumerate(l), key=operator.itemgetter(1))
        max_index, max_value = max(enumerate(l), key=operator.itemgetter(1))

        print "Statistic info of %s:\n" % arg,      \
              "Total Average     : %f\n" % numpy.mean(l),  \
              "Min Average       : %f\n" % min_value,      \
              "Max Average       : %f\n" % max_value,      \
              "Index of Min Aver : %-4d\n" % min_index,      \
              "Index of Max Aver : %-4d\n\n" % max_index

