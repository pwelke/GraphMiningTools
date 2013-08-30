#!/usr/bin/env python

import os, sys, traceback
import getpass
from threading import Thread
from subprocess import *

if(sys.hexversion < 0x03000000):
	import Queue
else:
	import queue as Queue
	
	
# executable stuff
command = "python plotroc.py"

# parameter stuff
pass_through_string = ""
dataset_pathname = sys.argv[-1]

# parallel stuff
telnet_workers = []
ssh_workers = []
nr_local_worker = 8

# range stuff
c_begin, c_end, c_step = 1, 10, 2
w_begin, w_end, w_step = 1, 10, 2
fold = 3


def range_f(begin,end,step):
    # like range, but works on non-integer too
    seq = []
    while True:
        if step > 0 and begin > end: break
        if step < 0 and begin < end: break
        seq.append(begin)
        begin = begin + step
    return seq

def calculate_jobs_old():
    c_seq = permute_sequence(range_f(c_begin,c_end,c_step))
    w_seq = permute_sequence(range_f(w_begin,w_end,w_step))
    nr_c = float(len(c_seq))
    nr_w = float(len(w_seq))
    i = 0
    j = 0
    jobs = []

    while i < nr_c or j < nr_w:
        if i/nr_c < j/nr_w:
            # increase C resolution
            line = []
            for k in range(0,j):
                line.append((c_seq[i],w_seq[k]))
            i = i + 1
            jobs.append(line)
        else:
            # increase g resolution
            line = []
            for k in range(0,i):
                line.append((c_seq[k],w_seq[j]))
            j = j + 1
            jobs.append(line)
    return jobs

def calculate_jobs():
	c_seq = range_f(c_begin, c_end, c_step)
	w_seq = range_f(w_begin, w_end, w_step)
	jobs = []
	for c in c_seq:
		jobs += [(c, w) for w in w_seq]
	return jobs

class WorkerStopToken:  # used to notify the worker to stop
        pass

class Worker(Thread):

    def __init__(self, name, job_queue, result_queue):
        Thread.__init__(self)
        self.name = name
        self.job_queue = job_queue
        self.result_queue = result_queue

    def run(self):
        while True:
            (c,w) = self.job_queue.get()
            if c is WorkerStopToken:
                self.job_queue.put((c,w))
                # print('worker {0} stop.'.format(self.name))
                break
            try:
                rate = self.run_one(c, w)
                if rate is None: raise RuntimeError("get no rate")
            except:
                # we failed, let others do that and we just quit
                traceback.print_exception(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2])
                
                self.job_queue.put((c,w))
                print('worker {0} quit.'.format(self.name))
                break
            else:
                self.result_queue.put((self.name, c, w, rate))

class LocalWorker(Worker):
    def run_one(self,c,w):
        cmdline = '{0} -v {3} -c {1} -w1 {2} -t 0  {4} {5}'.format \
          (command, c, w, fold, pass_through_string, dataset_pathname)
        result = Popen(cmdline, shell=True, stdout=PIPE).stdout
	for line in result: 
		print line
	return result

def main():

	# put jobs in queue
	jobs = calculate_jobs()
	job_queue = Queue.Queue(0)
	result_queue = Queue.Queue(0)

	for task in jobs:
		job_queue.put(task)
	for i in range(nr_local_worker):
		job_queue.put(WorkerStopToken())

	# hack the queue to become a stack --
	# this is important when some thread
	# failed and re-put a job. It we still
	# use FIFO, the job will be put
	# into the end of the queue, and the graph
	# will only be updated in the end
 
	job_queue._put = job_queue.queue.appendleft
	
	# fire local workers
	for i in range(nr_local_worker):
		LocalWorker('local',job_queue,result_queue).start()
		
main()
	 
