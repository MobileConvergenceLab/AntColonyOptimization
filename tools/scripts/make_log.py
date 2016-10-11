#!/usr/bin/env python
import os
import argparse

def conduct_experiments(topo, num, dir):

    l = list()
    for i in range(0, num):
        logfile = './%s/' % dir + '%04d.log' % i
        os.system("sudo ./driver.mn %s -l %s" % (topo, logfile))
        os.system("sync")

def return_args():
    parser = argparse.ArgumentParser()

    parser.add_argument('topo', metavar='topo', type=str,
                        help='filename or path that describes topology')

    parser.add_argument('num', metavar='num', type=int,
                        help='Number of experiments')

    parser.add_argument('--dir', type=str, required=False, default='.',
                        help='the name of dir where logfile will be stored')

    return parser.parse_args()

def args_hndler_dir(dir):
    os.system("mkdir -p %s" % dir)


if __name__ == '__main__':
    args = return_args()

    args_hndler_dir(args.dir)

    conduct_experiments(args.topo, args.num, args.dir)


