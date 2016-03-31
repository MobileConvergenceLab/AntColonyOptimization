# -*- coding: utf-8 -*- 
import os
import numpy
import matplotlib.pyplot as plt

TEST_COUNT      = 10

def experiment(aco, test_count, logfile_prefix):
    l = list()
    for i in range(0, test_count):
        logfile = '%s_%02d.log' % (logfile_prefix, i)
        os.system("sudo ./driver.mn %s ./topo/topo4 -l %s" % (aco, logfile))
        os.system("sync")

experiment("./aco", TEST_COUNT, '')
