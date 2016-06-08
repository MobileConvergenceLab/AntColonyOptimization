# -*- coding: utf-8 -*-
"""
경로 복구 알고리즘의 성능 검증 분석을 위한
스크립트. 로그파일을 읽어서 손실된 패킷의 평균과 분산을 구한다.
"""

import numpy
import subprocess

# number of log files
MAX = 50

# number of send packets
SEND_PKT = 3000

l = list()

for i in range(0, MAX):
    logfile = '%04d.log' % (i)
    wc = subprocess.Popen(['wc', logfile, '-l'], stdout=subprocess.PIPE)
    out, error = wc.communicate()

    # number of recv(reponse) packets
    recv_pkt = int(out.split()[0])

    # number of loss packets
    loss_pkt = SEND_PKT - recv_pkt

    l.append(loss_pkt)

print 'mean:', numpy.mean(l)
print 'var:', numpy.std(l)
