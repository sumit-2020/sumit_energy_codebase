#!/usr/bin/python

import os, sys, re, time, copy
import numpy as np
import pandas as pd
import re

from os import listdir
from os.path import isfile, join
from termcolor import colored

def main() :
    temp = int(get_node_config(feature = "temp"))
    name = get_node_config(feature = "name")[0]
    df = parse_logs()
    df["temp"] = temp
    print df
    df.to_csv(name+".csv")

# Get the temperature of the node from config file
def get_node_config(feature = "") :
    configfile = "/home/dram/dram_codebase/node.config"
    config = pd.read_csv(configfile)
    if feature == "" :
        return config
    else:
        return config[(config["config"]==feature)]["value"]

def parse_logs() :
    dir_list = next(os.walk('.'))[1]
    ret_df = pd.DataFrame(columns=["dimm","vdd","temp","ret_time","vendor",
            "b0","b1","b2","b3","b4","b5","b6","b7","total","date"])
    for d in dir_list :
        if "newout_" in d :
            d_parts = d.split('_')
            dimm = d_parts[1]
            date = "WedNov0921:25:30EST2016"
            if len(d_parts) > 3 :
                date = d_parts[3]
            vendor = get_vendor(dimm)
            flts = re.findall("\d+\.\d+",str(d.split('_')[2]))
            vdd = float(flts[0])
            #print dimm + " " + str(vdd) + " : "
            for f in os.listdir(d) :
                ret_time  = int(re.findall("\d+",f)[0])
                bank_errors = fetch_errors(d+"/"+f)
                total_errors = sum(bank_errors)
                row = {
                    "dimm"      : dimm,
                    "vdd"       : vdd,
                    "ret_time"  : ret_time,
                    "vendor"    : vendor,
                    "date"      : date,
                    "b0": bank_errors[0],
                    "b1": bank_errors[1],
                    "b2": bank_errors[2],
                    "b3": bank_errors[3],
                    "b4": bank_errors[4],
                    "b5": bank_errors[5],
                    "b6": bank_errors[6],
                    "b7": bank_errors[7],
                    "total" : total_errors
                     }
                row_series = pd.Series(row)
                #print row_series
                ret_df = ret_df.append(row_series, ignore_index=True)
    return ret_df

def fetch_errors(fname):
    #print fname
    bank_errors = [0]*8
    with open(fname, 'r') as fin:
        for line in fin :
            if "Error" in line :
                banks =  re.findall("Bank: \d+",line)
                bank = int(re.findall("\d+",banks[0])[0])
                bank_errors[bank] += 1
    return bank_errors

def get_vendor(name):
    if 'crucial' in name or 'micron' in name:
        return 'A'
    elif 'samsung' in name:
        return 'B'
    elif 'hynix' in name:
        return 'C'
    return 'Other'

main()
