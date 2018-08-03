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
import math, serial
import tweepy

# Required for Meter Connection (USB)
import serial
sys.path.insert(0, '../src/')
import usbtmc as tmc

FPS = 40
FNULL = open(os.devnull, 'w')

def stringify_time(t_sec):
    t_min = int(t_sec) / 60
    t_hrs = int(t_min) / 60
    t_day = int(t_hrs) / 24

    t_hrs = int(t_hrs) % 24
    t_min = int(t_min) % 60
    t_sec = int(t_sec) % 60

    t_str = ""
    if t_day > 0:
        t_str += str(t_day) + "d "
    if t_hrs > 0:
        t_str += str(t_hrs) + "h "
    if t_min > 0:
        t_str += str(t_min) + "m "
    if t_sec > 0:
        t_str += str(t_sec) + "s "

    return t_str


def measure_loop(args) :
  quality = 0
  read_try = 0

  while quality < args.qt and read_try < 3:
    data_arr, quality = read_meter(args)
    read_try += 1

  return data_arr, quality

def init_meter_connection(sandbox=False):
    if not sandbox:
        node_info = pd.read_csv("../node.info")
        SERIAL = node_info[(node_info.arg == "usbport")].iloc[0].value
        conn  = serial.Serial(SERIAL, 460800, timeout=0.1)
        meter = tmc.instrument(tmc.AGILENT_34461A)
        meter.write('*RST')
    else:
        meter = False
    return meter

def get_measurements(meter, sandbox = False) :
  values = []
  if not sandbox:
    for i in range(5):
      values.append(float(meter.ask('MEAS:VOLT:DC? AUTO'))*1000)
  else:
    values = [0,1,2,3,4]
  return values

def get_measurements_arr(meter, num_samples=20, sandbox = False):
  values = []
  if not sandbox:
    commands  ="CONF:DCV\n"
    commands +="TRIG:COUNT "+str(num_samples)+"\n"
    commands +="TRIG:SOUR IMM\n"
    commands +="INIT\n"
    meter.write(commands)
    ret_string = meter.ask("DATA:REM? "+str(num_samples)+", WAIT")
    for line in ret_string.split("\n"):
      values.append(float(line.split(",")[0]))
  else:
    values = [-1] * num_samples

  return values

def read_meter(args) :
  num_of_samples = 0
  data_arr = []

  if args.nocam:
    command = "sudo python readmeter.py > ./img_out/values.log"
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

  quality = 0
  if data_arr:
    if args.nocam :
      quality = 1.0
    else:
      quality = float(num_of_samples) / float(max_reads)
  return data_arr, quality

def str_timestamp():
  d = datetime.fromtimestamp(time.time())
  return d.strftime("%d.%m.%y_%I:%M%p")

def clean_start(directory = "", regexp = "*"):
  if directory == "":
    clean_dir("./img_out",regexp)
    # clean_dir("./out",regexp)
    clean_dir("./log_out",regexp)
  else :
    clean_dir(directory,regexp)

def clean_dir(path, regexp = '*'):
  for file in os.listdir(path):
    if regexp in file or regexp == '*':
      os.remove(path+"/"+file)

def tweet (message):
  node_info = pd.DataFrame(columns=["arg","value"])
  node_info = node_info.from_csv("../node.info")
  CONSUMER_KEY    = node_info[(node_info.arg == "twitter.consumer.key")].iloc[0].value
  CONSUMER_SECRET = node_info[(node_info.arg == "twitter.consumer.secret")].iloc[0].value
  ACCESS_KEY      = node_info[(node_info.arg == "twitter.access.key")].iloc[0].value
  ACCESS_SECRET   = node_info[(node_info.arg == "twitter.access.secret")].iloc[0].value
  NODE_NAME       = node_info[(node_info.arg == "name")].iloc[0].value

  auth = tweepy.OAuthHandler(CONSUMER_KEY, CONSUMER_SECRET)
  auth.set_access_token(ACCESS_KEY, ACCESS_SECRET)
  api = tweepy.API(auth)
  # print "Twitting now"
  try:
    status = api.update_status(status="#"+NODE_NAME+": "+message)
  except tweepy.error.TweepError:
    pass
