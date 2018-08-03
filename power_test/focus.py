#!/usr/bin/python

import sys, os, stat, subprocess as sp, glob
import tarfile, zipfile
import argparse
import operator
import time
from datetime import datetime
import pandas as pd
import re
from subprocess import Popen, PIPE
import fnmatch
import seaborn as sns
import matplotlib.pyplot as plt

str_def = "none"
int_def = -1
flt_def = -1.0

def main():
  args = parse_arguments()

  df = pd.read_csv("./out/combined.csv")
  df = pd.read_csv("./out/safe/combined.csv")

  df = filter(df, "dimm"      ,args.dimm      ,str_def)
  df = filter(df, "test_name" ,args.test      ,str_def)
  df = filter(df, "row"       ,args.row       ,int_def)
  df = filter(df, "bank"      ,args.bank      ,int_def)
  df = filter(df, "pattern"   ,args.pattern   ,int_def)
  df = filter(df, "one_ratio" ,args.one_ratio ,flt_def)

  # df.to_csv("./out/filter_out.csv")
  # print df

  if args.pattern == int_def :
    _args = args
    for pattern, pattern_group in df.groupby("pattern") :
      ave_current = pattern_group.ave_current.mean()
      std_current = pattern_group.ave_current.std()
      
      _args.pattern = pattern
      _args.one_ratio = pattern_group.iloc[0].one_ratio
      print_args(_args, False)
      print " Ave: " + str(ave_current),
      print " Std: " + str(std_current)
      print pattern_group.raw_data


  return 0


def parse_arguments():
  parser = argparse.ArgumentParser()
  parser.add_argument('--dimm', metavar='dimm', type=str, default=str_def,
       help='DIMM Name')
  parser.add_argument('--test', metavar='test', type=str, default=str_def,
       help='Test Name')
  parser.add_argument('--row', metavar='row', type=int, default=int_def,
       help='Row Address')
  parser.add_argument('--bank', metavar='bank', type=int, default=int_def,
       help='Bank Address')
  parser.add_argument('--pattern', metavar='pattern', type=int, default=int_def,
       help='Pattern')
  parser.add_argument('--one_ratio', metavar='one_ratio', type=float, default=flt_def,
       help='Proportion of Ones in Pattern')

  args = parser.parse_args()

  return args

def filter(df,arg,val,default):
  if val != default :
    df = df[(df[arg] == val)]
  return df

def print_args(args, new_line):
  print "[" + args.dimm + " " + args.test + "]",
  print " Bank: " + str(args.bank) + " Row: " + str(args.row),
  print " Pattern: " + str(hex(args.pattern)[0:-1]) + " 1s: " + str(args.one_ratio),

  if new_line == True :
    print "" 

main()
