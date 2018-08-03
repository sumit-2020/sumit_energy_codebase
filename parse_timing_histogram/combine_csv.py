#!/usr/bin/python

import pandas as pd
import os
import argparse

parser = argparse.ArgumentParser(description='Process csv files.')
parser.add_argument('--input', metavar='F', type=str, required=True,
        help='cls or hist')
args = parser.parse_args()

# combine all the csv files
csv_list = []
if "cls" in args.input:
	file_name = "combined_summary_cachelines_with_error"
elif "hist" in args.input :
	file_name = "combined_summary_hist_num_errors"

for i in range(8) :
	csv_list.append(file_name+".csv_"+str(i))

dfs = [pd.read_csv(csv) for csv in csv_list]
combined_df = pd.concat(dfs, ignore_index=True)
combined_df = combined_df.drop_duplicates(cols=["dimm_name","tRCD","tRP","temp","vdd"],
        take_last=True)
combined_df.to_csv(file_name+".csv", index=False)
print "Number of data points %d" % len(combined_df.index)
