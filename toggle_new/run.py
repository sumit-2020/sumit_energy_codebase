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
import serial
import tweepy
#sys.path.insert(0, '../src/')
#import usbtmc as tmc

FPS = 400
FNULL = open(os.devnull, 'w')

def get_tests():
  test_types = ["singlebankmulticol", "multibankmulticol"]
  addr_sets  = {
    "singlebankmulticol":
    [
      "r01", "r02", "r03", "r04", "r17", "r18", "r19", "r20"
    ],
    "multibankmulticol":
    [
      "r01", "r02", "r03", "r04", "r05", "r06", "r07", "r08", "r09", "r10",
      "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19", "r20"
    ]
  }

  tests = []#      = [{ "test_n":"", "addr_set":"", "patt1":0, "patt2":0, "patt3":0 } for k in range(60)]
  for test in addr_sets:
    for addr_set in addr_sets[test]:
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x00 , "patt3": 0x00})
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x55, "patt2": 0x55 , "patt3": 0x55})
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xAA, "patt2": 0xAA , "patt3": 0xAA})
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xFF, "patt2": 0xFF , "patt3": 0xFF})
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xAA , "patt3": 0xFF})
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0xFF , "patt3": 0x00})
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xCC, "patt2": 0xCC , "patt3": 0xCC})
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xF0, "patt2": 0xF0 , "patt3": 0xF0})
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xA0, "patt2": 0xA0 , "patt3": 0xA0})
      tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x0A, "patt2": 0x0A , "patt3": 0x0A})
  return tests

def main():
  print "starting"
  args = parse_arguments(True)
  args.log = False
  tests = get_tests()
  clean_start()

  #define meter connection
  #args.conn  = serial.Serial('/dev/ttyS0', 460800, timeout=0.1)
  #args.meter = tmc.instrument(tmc.AGILENT_34461A)

  #if args.nocam :
  #  args.meter.write('SYSTem:FPReset')

  data_dict = {
      "test_id"  :[],
      "test_n"   :[],
      "addr_set" :[],
      "patt1"    :[],
      "patt2"    :[],
      "patt3"    :[],
      "iter"     :[],
      "raw_data" :[],
      "min"      :[],
      "max"      :[],
      "ave"      :[],
      "stdev"    :[],
      "quality"  :[],
      "row"      :[]
  }
  print "before tweet"
  
  #tweet(args, "starting now!")

  dataframe = pd.DataFrame(columns=data_dict.keys())
  output_csv = "./out/"+args.dimm+"_"+str_timestamp()+".csv"
  print "["+str_timestamp()+"] Starting the experiments. Output: " + output_csv
  for test_id, test in enumerate(tests) :
    i = 0
    attempt = 0
    while i < args.iter and attempt < 3 * args.iter:
      attempt = attempt + 1
      clean_start("./img_out","*")
      run_binary(args.dimm,test["test_n"],test["addr_set"],args.row,test["patt1"],test["patt2"],test["patt3"])
      sleep(0.001)
      data_arr, quality = read_meter(args)
      run_binary(args.dimm,"stoploop")

      # raw_input("["+str_timestamp()+"] Soft reset the board and hit <Enter>")
      data_arr, quality = read_meter(args)
      run_binary(args.dimm,"stoploop")

      # raw_input("["+str_timestamp()+"] Soft reset the board and hit <Enter>")
      if (quality > args.qt):
        i = i + 1
        data_dict = append_data(data_dict, test_id, test, i, data_arr, quality, args.row)
        dataframe = dataframe.from_dict(data_dict)
        dataframe.to_csv(output_csv)
      else:
        print "["+str_timestamp()+"] ERROR: Poor vision! Quality: " + str(quality)
        # quit()
  tweet(args, " is done!")

def tweet (args,message):
  #enter the corresponding information from your Twitter application:
  CONSUMER_KEY = 'RmOpgbmyN4XprksbIlD1xVlOv'#keep the quotes, replace this with your consumer key
  CONSUMER_SECRET = 'Twkgk0RwSCiFX9TdLM9JoOgLwKQDTcol86RwTJGC8uO16PNp3F'#keep the quotes, replace this with your consumer secret key
  ACCESS_KEY = '835608707698413573-38Gl8q6gTPGa0djtbuCHLE9HMdouiN6'#keep the quotes, replace this with your access token
  ACCESS_SECRET = 'Vombb2Ls6cv1tkR3MEpeTvSAZTDVUbzivEcyr9rKE7XK8'#keep the quotes, replace this with your access token secret
  auth = tweepy.OAuthHandler(CONSUMER_KEY, CONSUMER_SECRET)
  auth.set_access_token(ACCESS_KEY, ACCESS_SECRET)
  api = tweepy.API(auth)
  print "Twitting now"
  api.update_status("#"+args.dimm+" "+message)
 
def run_binary(dimm,test,addr_set="r01",row=17868,patt1=0,patt2=0,patt3=0):
  command  = "./bin/toggletest " + dimm
  command += " " + test
  command += " " + addr_set
  command += " " + str(row)
  command += " " + str(patt1)
  command += " " + str(patt2)
  command += " " + str(patt3)
  # command += " " + str(1)

  command += "  > ./log_out/" + "_" + dimm
  command += "_" + test
  command += "_" + addr_set
  command += "_" + str(row)
  command += "_" + str(patt1)
  command += "_" + str(patt2)
  command += "_" + str(patt3) + ".log"

  if test != "stoploop":
    print "["+str_timestamp()+"] Running: ", command

  sp.call(command, shell=True)

def read_meter(args) :
  num_of_samples = 0
  data_arr = []

  if args.nocam:
    command = "python readmeter.py > ./img_out/values.log"    
    sp.call(command, shell=True, stdout=FNULL, stderr=sp.STDOUT)
  
  else :   
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
      if args.nocam:
        data_arr.append(float(line))
      
      else:
        l = line.strip('\n')[:5]  # Parse the first 5 characters. The 6th one is never stable anyways
        if l.isdigit():
          num_of_samples += 1
          num_str = l[0:3]+"."+l[3:5];
          retval = re.findall("\d+\.\d+", num_str)
          data_arr.append(float(retval[0]))

  if args.nocam :
    quality = 1.0
  else: 
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
  parser.add_argument('--nocam', action='store_true', default=False,
       help='No Camera in the measurement part')
  parser.add_argument('--qt', metavar='qt', type=float, default=0.6,
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

def append_data(data_dict, test_id, test, i, data_arr, quality,row):
  raw_data = ','.join(map(str, data_arr))
  data_dict["test_id"].append(test_id)
  data_dict["test_n"].append(test["test_n"])
  data_dict["addr_set"].append(test["addr_set"])
  data_dict["patt1"].append(test["patt1"])
  data_dict["patt2"].append(test["patt2"])
  data_dict["patt3"].append(test["patt3"])
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
