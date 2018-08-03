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

FNULL = open(os.devnull, 'w')

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

def get_measurements(meter,sandbox=False):
  values = []
  for i in range(5):
    if sandbox:
      values.append(-1)
    else:
      values.append(float(meter.ask('MEAS:VOLT:DC? AUTO'))*1000)
  return values

def str_timestamp():
  d = datetime.fromtimestamp(time.time())
  return d.strftime("%d.%m.%y_%I:%M%p")

def clean_start(directory = "", regexp = "*"):
  if directory == "":
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
