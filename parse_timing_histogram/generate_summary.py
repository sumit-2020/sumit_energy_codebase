#!/usr/bin/python
import histfile as hf
from histfile import Histfile
import os, sys, re, time, copy
import numpy as np
import pandas as pd

from os         import listdir
from os.path    import isfile, join
from termcolor  import colored

def main():
	# Initialize
	keys = Histfile.get_keys_cls_w_err_row()
	summary_file = "summary_cachelines_with_error.csv" 
	results = {k:[] for k in keys}
	scanned_file_list = hf.get_list_of_summarized_files(summary_file,keys)
	old_df = None
	file_list = []
	file_list = hf.get_histogram_files(file_list,"../timing_histogram/out",scanned_file_list,False)
	file_list = hf.get_histogram_files(file_list,"../timing_histogram_mem/out",scanned_file_list,False)
	
	# Walk through the list of files
	for f in file_list :
		my_histfile = Histfile(f,False)
		new_line = my_histfile.get_cachelines_w_error();
		for key , value in new_line.iteritems() :
	 		results[key].append(value)
	
	# Create dataframe of new files		
	df = pd.DataFrame(results,columns=Histfile.get_keys_cls_w_err_row())

	# Read the CSV file to update
	if os.path.isfile(summary_file) :
		old_df = pd.read_csv(summary_file,names=Histfile.get_keys_cls_w_err_row(), 
                                skipinitialspace=True, header=0 )
		df = pd.concat([df, old_df], ignore_index=True)
	df.to_csv(summary_file, index=False)

main()