#!/usr/bin/python
import histfile as hf
from histfile import Histfile
import os, sys, re, time, copy
import numpy as np
import pandas as pd
import argparse
import math

from os         import listdir
from os.path    import isfile, join
from termcolor  import colored

parser = argparse.ArgumentParser(description='Process csv files.')
parser.add_argument('--thread', metavar='N', type=int, nargs='+', required=True,
        help='Give a thread id to select the correct subset of the files')
parser.add_argument('--total', metavar='N', type=int, nargs='+', required=True,
        help='Give a thread id to select the correct subset of the files')

args = parser.parse_args()

def main():
    # Thread ID
    thread_id = int(args.thread[0])
    num_of_threads = int(args.total[0])

    # Initialize
    hist_file = "combined_summary_hist_num_errors.csv_"+str(thread_id)
    hist_keys = Histfile.get_keys_num_err_hist_row()
    hist_results = {k:[] for k in hist_keys}
    hist_old_list = hf.get_list_of_summarized_files(hist_file,hist_keys)

    summ_file = "combined_summary_cachelines_with_error.csv_"+str(thread_id)
    summ_keys = Histfile.get_keys_cls_w_err_row()
    summ_results = {k:[] for k in summ_keys}
    summ_old_list = hf.get_list_of_summarized_files(summ_file,summ_keys)

    old_hist_df = None
    old_summ_df = None

    file_list = []
    #file_list = hf.get_histogram_files(file_list,"../timing_histogram/out"          , hist_old_list, False)
    #file_list = hf.get_histogram_files(file_list,"../timing_histogram_mem/out"      , hist_old_list, True)
    file_list = hf.get_histogram_files(file_list,"../timing_histogram_mem/out_fix"  , hist_old_list, False)

    # Get portion of this thread
    num_files_per_thread = math.ceil(float(len(file_list)) / float(num_of_threads))
    start_index = int(thread_id * num_files_per_thread)
    stop_index = int(min( (len(file_list)-1) , (thread_id+1) * num_files_per_thread ))
    file_list = file_list[ start_index : stop_index ]

    print num_files_per_thread
    print start_index
    print stop_index

    if file_list :
        # Walk through the list of files
        for f in file_list :
            my_histfile = Histfile(f,True)

            if not hf.check_file_hist(f,hist_old_list) :
                new_hist_line = my_histfile.get_num_err_hist_line()
                for key , value in new_hist_line.iteritems() :
                    hist_results[key].append(value)

            if not hf.check_file_hist(f,summ_old_list) :
                new_summ_line = my_histfile.get_cachelines_w_error()
                for key , value in new_summ_line.iteritems() :
                    summ_results[key].append(value)

        hf.log_it("Results of all files are being merged!")

        # Create dataframe of new files
        hist_df = pd.DataFrame( hist_results , columns=Histfile.get_keys_num_err_hist_row() )
        summ_df = pd.DataFrame( summ_results , columns=Histfile.get_keys_cls_w_err_row() )

        # Read the CSV file to update
        if os.path.isfile(hist_file) :
            old_hist_df = pd.read_csv(hist_file,names=Histfile.get_keys_num_err_hist_row(),
                                        skipinitialspace=True, header=0 )
            hist_df = pd.concat([hist_df, old_hist_df], ignore_index=True)

        # Read the CSV file to update
        if os.path.isfile(summ_file) :
            old_summ_df = pd.read_csv(summ_file,names=Histfile.get_keys_cls_w_err_row(),
                                        skipinitialspace=True, header=0 )
            summ_df = pd.concat([summ_df, old_summ_df], ignore_index=True)

        hist_df.to_csv(hist_file, index=False)
        summ_df.to_csv(summ_file, index=False)

        hf.log_it(colored("Dumped the hist file into " + hist_file,"green",attrs=['bold']))
        hf.log_it(colored("Dumped the summ file into " + summ_file,"green",attrs=['bold']))
    else:
        hf.log_it(colored("There is no new file to parse. All of them or already in the following files: ",attrs=['bold']),",")
        hf.log_it(colored(hist_file,"green",attrs=['bold']))
        hf.log_it(colored(summ_file,"green",attrs=['bold']))
main()
