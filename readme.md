# Basic Info
Author: Sim Young-Bo

Email: ybsim@khu.ac.kr

CopyRight(c) MobileConvergence Lab. in KyungHee Univ. All rights reserved.

# Directories
* src: 데몬
* test-ipc: 데몬, Fon-Function간 테스트
* test-sendrecv: 노드-노드간 테스트
* aco: anti-colony optimization routing agent software
* tools: *Minunet* Emulation scripts(driver.mn, make_log.py),
         Plot makers(hist.py, stat.py, time.py, fit.py).

# Dependencies
Required:
* GCC 4.8.4
* glib 2.0 (NOT glibc)

...and options:
* Mininet 2.2.1 (for emulation)
* Python 2.7.6 (for plot)
* numpy (for plot)
* matplotlib.pyplot (for plot)

# FON
This section will be updated.

# Ant Colony Optimization Routing Agent(S/W)
It DOES NOT USE *IP protocol* instead uses our *FON* protocol and L2 Protocols.

In current version, our Agent can be run in IEEE 802 family environment -
eg. 802.3(Ethernet) or 802.11(WiFi).

These were implemented in our Agent:
 - Ant System, Ant Colony System (proposed by *M.Dorigo*)
 - Source Update (proposed in the paper, *A Parallel Ant Colony Optimization
   Algorithm for All-Pair Routing in MANETs*)
 - Ant Local (proposed by *Sim Young-Bo*)
 - Simple Backtracking Update (proposed by *Sim Young-Bo*)
 - Link Recovery (proposed by *Sim Young-Bo*)

## Emulation Example

### Topology
![Topology](/resource/topo.png)
*Topology*

### Comparison Ant System with Ant Local
![Time Seriese](/resource/time.png)
*Time Seriese Graph*

![Histogram](/resource/hist.png)
*Histogram*

The experiments was conduncted using our agnet in */aco/* and *Mininet*

*Source, node 0* and *Destination, node 15* are in depicted in Fig. *Topolog*.

These graphes was plotted using our scripts in */tools/*

### Link Recovery
This section will be update.
