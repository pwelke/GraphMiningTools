#!/usr/bin/env python

import os, sys, traceback, pickle, time
import getpass
from threading import Thread
from subprocess import *

import plotroc

if(sys.hexversion < 0x03000000):
	import Queue
else:
	import queue as Queue
	
	
# executable stuff
command = "python plotroc.py"

# parameter stuff
pass_through_string = " ".join(sys.argv[1:-1])
print(pass_through_string)
dataset_pathname = sys.argv[-1]

# kernel type
# -t kernel_type : set type of kernel function (default 2)
#     0 -- linear: u'*v
#     1 -- polynomial: (gamma*u'*v + coef0)^degree
#     2 -- radial basis function: exp(-gamma*|u-v|^2)
#     3 -- sigmoid: tanh(gamma*u'*v + coef0)
kernel_type = 0

# results dumped to
auc_result_file = dataset_pathname + "-t {0}_aucResults".format(kernel_type) # will be set to something resembling input file name later

# parallel stuff
telnet_workers = []
ssh_workers = []
nr_local_worker = 7

# range stuff
c_begin, c_end, c_step = -5, 15, 2
w_begin, w_end, w_step = -5, 10, 2
# g_begin, g_end, g_step = -15, 5, 2
fold = 3


def range_f(begin,end,step):
	'''Like range, but works on non-integer too'''
	seq = []
	while True:
		if step > 0 and begin > end: break
		if step < 0 and begin < end: break
		seq.append(begin)
		begin = begin + step
	return seq

def calculate_jobs_exp():
	'''Exponentially increasing values of c and w'''
	c_seq = range_f(c_begin, c_end, c_step)
	w_seq = range_f(w_begin, w_end, w_step)
	jobs = []
	for c in c_seq:
	    jobs += [(2**c, 2**w) for w in w_seq]
	return jobs

def calculate_jobs_lin():
	'''Linearly increasing values of c and w'''
	c_seq = range_f(c_begin, c_end, c_step)
	g_seq = range_f(g_begin, g_end, g_step)
	jobs = []
	for c in c_seq:
		jobs += [(c, g) for g in g_seq]
	return jobs

class WorkerStopToken:  
	'''used to notify a Worker to stop'''
	pass

class Worker(Thread):

	def __init__(self, name, job_queue, result_queue, auc_dict):
		Thread.__init__(self)
		self.name = name
		self.job_queue = job_queue
		self.result_queue = result_queue
		self.auc_dict = auc_dict

	def run(self):
		while True:
			(c,g) = self.job_queue.get()
			if c is WorkerStopToken:
				self.job_queue.put((c,g))
				# print('worker {0} stop.'.format(self.name))
				break
			try:
				rate = self.run_one(c, g)
				if rate is None: 
					raise RuntimeError("get no rate")
				else:
					self.auc_dict[(c,g)] = rate
			except:
				# we failed, let others do that and we just quit
				traceback.print_exception(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2])
				
				self.job_queue.put((c,g))
				print('worker {0} quit.'.format(self.name))
				break
			else:
				self.result_queue.put((self.name, c, g, rate))

class LocalWorker(Worker):
	def run_one(self, c, g):
		# cmdline = '{0} -c {1} -g {2} -v {3} -t {4} {5} {6}'.format \
			# ('plotroc.py', c, g, fold, kernel_type, dataset_pathname, pass_through_string)
		# print 'cmdLine: ' + cmdline
		# result = Popen(cmdline, shell=True, stdout=PIPE).stdout
		param = '-c {0} -g {1} -t {2} {3}'.format \
			(c, g, kernel_type, pass_through_string)
		train_file = dataset_pathname
		test_file = None
		result = plotroc.main(param,fold,train_file,test_file)
		# for line in result: 
		# 	print line
		return result

def main():

	# put jobs in queue
	jobs = calculate_jobs_exp()
	job_queue = Queue.Queue(0)
	result_queue = Queue.Queue(0)
	for task in jobs:
		job_queue.put(task)

	# add as many stop tokens as there are workers,
	# such that every thread stops in the end.
	for i in range(nr_local_worker):
		job_queue.put(WorkerStopToken())

	# hack the queue to become a stack --
	# this is important when some thread
	# failed and re-put a job. It we still
	# use FIFO, the job will be put
	# into the end of the queue, and the graph
	# will only be updated in the end
	job_queue._put = job_queue.queue.appendleft

	# create dict for returned aucs
	auc_dict = dict()
	
	# fire local workers
	for i in range(nr_local_worker):
		LocalWorker('local', job_queue, result_queue, auc_dict).start()


	while (True):
		time.sleep(10)
		if (job_queue.qsize() == 0):
			print auc_dict

			f = open(auc_result_file + '.dict', 'wb')
			pickle.dump(auc_dict, f)
			f.close()

			# c_begin, c_end, c_step = -5, 15, 2
			# g_begin, g_end, g_step = -15, 5, 2
			auc_array = [range_f(g_begin, g_end, g_step) for c in range_f(c_begin, c_end, c_step)]
			for c in range_f(c_begin, c_end, c_step):
				for g in range_f(g_begin, g_end, g_step):
					# print str((c - c_begin) / c_step) + " " + str((g - g_begin) / g_step)
					auc_array[(c - c_begin) / c_step][(g - g_begin) / g_step] = auc_dict[(2**c,2**g)]

			f = open(auc_result_file + '.array', 'wb')
			pickle.dump(auc_array, f)
			f.close()

			return


if __name__ == "__main__":	
	main()
