#!/usr/bin/python

import sys, os, subprocess as sp, glob
import tarfile, zipfile
import argparse
import operator
import time
from time import sleep
from datetime import datetime
import pandas as pd
import re
import statistics as stats
from subprocess import Popen, PIPE
import fnmatch

FPS = 400
FNULL = open(os.devnull, 'w')

def get_tests():
    tests    = [{"test_n":"","patt1":0,"patt2":0,"patt3":0} for k in range(11)]
    tests[0] = {"test_n": "b0c0"  , "patt1": 0x00, "patt2": 0    , "patt3": 0}
    tests[1] = {"test_n": "b0c0"  , "patt1": 0x55, "patt2": 0    , "patt3": 0}
    tests[2] = {"test_n": "b0c0"  , "patt1": 0xFF, "patt2": 0    , "patt3": 0}
    tests[3] = {"test_n": "b0b1c0", "patt1": 0x00, "patt2": 0x00 , "patt3": 0}
    tests[4] = {"test_n": "b0b1c0", "patt1": 0x00, "patt2": 0x55 , "patt3": 0}
    tests[5] = {"test_n": "b0b1c0", "patt1": 0x00, "patt2": 0xFF , "patt3": 0}
    tests[6] = {"test_n": "x0x1x2", "patt1": 0x00, "patt2": 0x55 , "patt3": 0xFF}
    tests[7] = {"test_n": "x0x1x2", "patt1": 0x00, "patt2": 0x55 , "patt3": 0xFF}
    tests[8] = {"test_n": "b0c0c1", "patt1": 0x00, "patt2": 0x00 , "patt3": 0}
    tests[9] = {"test_n": "b0c0c1", "patt1": 0x00, "patt2": 0x55 , "patt3": 0}
    tests[10] = {"test_n": "b0c0c1", "patt1": 0x00, "patt2": 0xFF , "patt3": 0}
    return tests

def main():
  args = parse_arguments(True)
  args.log = False
  tests = get_tests()
  clean_start()

  data_dict = {
      "test_id"  :[],
      "iter"     :[],
      "raw_data" :[],
      "min"      :[],
      "max"      :[],
      "ave"      :[],
      "stdev"    :[],
      "quality"  :[],
      "row"      :[]
  }

  dataframe = pd.DataFrame(columns=data_dict.keys())
  output_csv = "./out/"+args.dimm+"_"+str_timestamp()+".csv"
  print "["+str_timestamp()+"] Starting the experiments. Output: " + output_csv
  for test_id, test in enumerate(tests) :
    # if test_id > 5 :
    print test
    i = 0
    attempt = 0
    while i < args.iter and attempt < 3 * args.iter:
      attempt = attempt + 1
      clean_start("./img_out","*")
      run_binary(args.dimm,test["test_n"],args.row,test["patt1"],test["patt2"],test["patt3"])
      sleep(0.001)
      data_arr, quality = parse_video_parallel(args)
      run_binary(args.dimm,"stoploop")

      # raw_input("["+str_timestamp()+"] Soft reset the board and hit <Enter>")
      if (quality > args.qt):
        i = i + 1
        data_dict = append_data(data_dict, test_id, i, data_arr, quality, args.row)
        dataframe = dataframe.from_dict(data_dict)
        dataframe.to_csv(output_csv)
      else:
        print "["+str_timestamp()+"] ERROR: Poor vision! Quality: " + str(quality)
        # quit()

def run_binary(dimm,test,row=120,patt1=0,patt2=0,patt3=0):
  command  = "./bin/toggletest " + dimm
  command += " " + test
  command += " " + str(row)
  command += " " + str(patt1)
  command += " " + str(patt2)
  command += " " + str(patt3)

  print "[INFO ] Running: " + command 

  sp.call(command, shell=True)

def parse_video_parallel(args) :
  num_of_samples = 0
  data_arr = []
  max_reads = args.nos / args.qt

  # Capture Frames
  command  = "streamer -r "+str(FPS)+" -f jpeg -s 1920x1080 -q -t "+str(max_reads)
  command += " -o img_out/image00.jpeg > ./img_out/streamer.log"
  sp.call(command, shell=True, stdout=FNULL, stderr=sp.STDOUT)
  if args.log:
      print command

  # Detect 7 segments in parallel
  sp.call("./detectall", shell=True)

  # Parse the merged results
  with open('./img_out/values.log') as f:
    for line in f :
      l = line.strip('\n')[:5]  # Parse the first 5 characters. The 6th one is never stable anyways
      if l.isdigit():
        num_of_samples += 1
        num_str = l[0:3]+"."+l[3:5];
        retval = re.findall("\d+\.\d+", num_str)
        data_arr.append(float(retval[0]))

  quality = float(num_of_samples) / float(max_reads)
  return data_arr, quality

def parse_arguments(print_flag = False):
  parser = argparse.ArgumentParser()
  parser.add_argument('dimm', metavar='dimm', type=str, default="SANDBOX",
       help='DIMM Name')
  parser.add_argument('iter', metavar='iter', type=int, default=10,
       help='# of Iterations')
  parser.add_argument('row', metavar='row', type=int, default=17868,
       help='Row number')
  parser.add_argument('--qt', metavar='qt', type=float, default=0.9,
       help='Quality Threshold of Vision: [# of Samples] / [# of Reads]')
  parser.add_argument('--nos', metavar='nos', type=int, default=30,
       help='# of Samples per Read')

  args = parser.parse_args()

  if (print_flag) :
    print "DIMM Name         : " + args.dimm        
    print "# of Iterations   : " + str(args.iter)   
    print "Row number        : " + str(args.row)  
    print "Quality           : " + str(args.qt)   
    print "Number of Samples : " + str(args.nos)  

  return args

def clean_start(directory = "", regexp = "*"):
  if directory == "":
    clean_dir("./img_out",regexp)
    clean_dir("./log_out",regexp)
  else :
    clean_dir(directory,regexp)

def clean_dir(path, regexp = '*'):
  for file in os.listdir(path):
    if regexp in file or regexp == '*':
      os.remove(path+"/"+file)

def str_timestamp():
  d = datetime.fromtimestamp(time.time())
  return d.strftime("%d.%m.%y_%I:%M%p")

def append_data(data_dict, test_id, i, data_arr, quality,row):
  raw_data = ','.join(map(str, data_arr))
  data_dict["test_id"].append(test_id)
  data_dict["iter"].append(i)
  data_dict["raw_data"].append(raw_data)
  data_dict["min"].append(min(data_arr))
  data_dict["max"].append(max(data_arr))
  data_dict["ave"].append(stats.mean(data_arr))
  data_dict["stdev"].append(stats.pstdev(data_arr))
  data_dict["quality"].append(quality)
  data_dict["row"].append(row)
  return data_dict

main()