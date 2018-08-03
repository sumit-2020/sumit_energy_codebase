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
import math

FPS = 40
FNULL = open(os.devnull, 'w')

def get_experiments():
    patterns = [
       #0x00,   # one_ratio: 0000_0000  0
        # 0x01,   # one_ratio: 0000_0001  12.5
       #0x03,   # one_ratio: 0000_0011  25
       # 0x11,   # one_ratio: 0001_0001  25
        # 0x92,   # one_ratio: 1001_0010  37.5
        # 0x07,   # one_ratio: 0000_0111  37.5
       #0x0f,   # one_ratio: 0000_1111  50
       0xaa,   # one_ratio: 1010_1010  50
        # # 0xab,   # one_ratio: 1010_1011  67.5
        # # 0xe3,   # one_ratio: 1110_0011  67.5
       #0xe7,   # one_ratio: 1110_0111  75
       #0xdb,   # one_ratio: 1101_1011  75
        # # 0xfe,   # one_ratio: 1111_1110  87.5
       #0xff    # one_ratio: 1111_1111 100
    ]

    test_names = [
#        "idd4r",
#        "idd4w",
        #"idd0",
        "idd1",
        #"actrdrdpre",
        #"actrdrdrdpre"
    ]

    banks = [0,1,2,3,4,5,6,7]
    row_step = 128
    num_rows = int(math.floor(32765 / row_step))
    rows = range(1,num_rows+1)
    rows = map(lambda x: x * row_step, rows)
    
    confs = {
        "test_names"    : test_names,
        "patterns"      : patterns,
        "banks"         : banks,
        "rows"          : rows,
        "row_step"      : row_step
    }

    return confs

def main():
    args = parse_arguments(True)
    args.log = False
    confs = get_experiments()
    clean_start()

    data_dict = {
        "test_name" :[],
        "pattern"   :[],
        "iter"      :[],
        "raw_data"  :[],
        "min"       :[],
        "max"       :[],
        "ave"       :[],
        "stdev"     :[],
        "quality"   :[],
        "bank"      :[],
        "row"       :[]
    }

    dataframe = pd.DataFrame(columns=data_dict.keys())
    output_csv = "./out/"+args.dimm+"_"+str_timestamp()+".csv"
    print "["+str_timestamp()+"] Starting the experiments. Output: " + output_csv
    for test_name in confs["test_names"]:
        for pattern in confs["patterns"]:
            for row in confs["rows"]    :
                for bank in confs["banks"]:
                    i = 0
                    attempt = 0
                    while i < args.iter and attempt < 3 * args.iter:
                        attempt = attempt + 1
                        clean_start("./img_out","*")
                        run_binary(i,args.dimm,test_name,bank,row,pattern)
                        sleep(0.001)
                        data_arr, quality = parse_video_parallel(args)
                        run_binary(i,args.dimm,"stoploop")

                        # raw_input("["+str_timestamp()+"] Soft reset the board and hit <Enter>")
                        if (quality > args.qt):
                            i = i + 1
                            data_dict = append_data(data_dict, test_name, pattern, i, data_arr, quality, bank, row)
                            dataframe = dataframe.from_dict(data_dict)
                            dataframe.to_csv(output_csv)
                        else:
                            print "["+str_timestamp()+"] ERROR: Poor vision! Quality: " + str(quality)
                            # quit()



    print "["+str_timestamp()+"] Experiments are completed! Check " + output_csv + " out."

def append_data(data_dict, test_name, pattern, i, data_arr, quality,bank,row):
    raw_data = ','.join(map(str, data_arr))
    data_dict["test_name"].append(test_name)
    data_dict["pattern"].append(pattern)
    data_dict["iter"].append(i)
    data_dict["raw_data"].append(raw_data)
    data_dict["min"].append(min(data_arr))
    data_dict["max"].append(max(data_arr))
    data_dict["ave"].append(stats.mean(data_arr))
    data_dict["stdev"].append(stats.pstdev(data_arr))
    data_dict["quality"].append(quality)
    data_dict["bank"].append(bank)
    data_dict["row"].append(row)
    return data_dict

def run_binary(i, dimm, test_name, bank=0, row=0, pattern=0x00):
    args = [dimm,test_name,str(pattern),str(bank),str(row)]
    command  = "./bin/iddtest " + " ".join(args)
    command += "  > ./log_out/" + "_".join(args) + ".log"
    if test_name != "stoploop":
        print "["+str_timestamp()+"] Exp. "+str(i+1)+": ", command
    sp.call(command, shell=True)

def str_timestamp():
    d = datetime.fromtimestamp(time.time())
    return d.strftime("%d.%m.%y_%I:%M%p")

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
            l = line.strip('\n')
            if l.isdigit():
                num_of_samples += 1
                num_str = l[0:3]+"."+l[3:6];
                retval = re.findall("\d+\.\d+", num_str)
                data_arr.append(float(retval[0]))

    quality = float(num_of_samples) / float(max_reads)
    return data_arr, quality

def parse_video(args) :
    num_of_samples = 0
    data_arr = []

    # Read the Data
    max_reads = args.nos / args.qt

    # Capture Frames
    command  = "streamer -r "+str(FPS)+" -f jpeg -s 1920x1080 -q -t "+str(max_reads)
    command += " -o img_out/image00.jpeg > ./img_out/streamer.log"
    sp.call(command, shell=True, stdout=FNULL, stderr=sp.STDOUT)
    if args.log:
        print command

    for file in os.listdir('./img_out/'):
        if fnmatch.fnmatch(file, 'image*.jpeg'):

            # Cover dot with a red square.
            command  = 'convert ./img_out/'+file+' -strokewidth 0 '
            command += '-fill "rgba( 255, 0, 0 , 1 )" '
            command += '-draw "rectangle 869,540 950,650" '
            command += './img_out/'+file
            sp.call(command, shell=True)

            if args.log:
                print command

            # Apply green threshold and detect 7 segment
            command  = "ssocr -d 6 -o ./img_out/out"+file[-7:-5]+".jpeg "
            command += "crop 100 250 1650 450 remove_isolated "
            command += "g_threshold invert ./img_out/"+file
            process = sp.Popen(command, shell=True, stdout=sp.PIPE)

            if args.log:
                print command

            # Parse the output
            proc_tuple = process.communicate()
            stdout = proc_tuple[0]

            # If there is '_', at least one digit is missing. Discard it!
            if '_' not in stdout :
                stdout = stdout[0:3]+"."+stdout[3:6];
                retval = re.findall("\d+\.\d+", stdout)

                # If there is a float number in the output add it into the list
                if (len(retval) > 0) :
                    num_of_samples += 1
                    retval_float = float(retval[0])
                    data_arr.append(retval_float)


    # TODO Traverse the list and filter the wrong detections.

    # Calculate Quality
    quality = num_of_samples / max_reads
    return data_arr, quality

def parse_arguments(print_flag = False):
    parser = argparse.ArgumentParser()
    parser.add_argument('dimm', metavar='dimm', type=str, default="SANDBOX",
         help='DIMM Name')
    parser.add_argument('iter', metavar='iter', type=int, default=20,
         help='# of Iterations')
    parser.add_argument('--qt', metavar='qt', type=float, default=0.8,
         help='Quality Threshold of Vision: [# of Samples] / [# of Reads]')
    parser.add_argument('--nos', metavar='nos', type=int, default=10,
         help='# of Samples per Read')
    # parser.add_argument('--parallel', metavar='parallel', type=int, default=0,
    #      help='Flag to run video parsing in parallel')
    args = parser.parse_args()

    if (print_flag) :
        print "|Configuration     | Value                    |"
        print "|:-----------------|:-------------------------|"
        print "|DIMM Name         |" + args.dimm        + "  |"
        print "|# of Iterations   |" + str(args.iter)   + "  |"
        print "|# of Measurements |" + str(args.nos)    + "  |"
        print "|Required Quality: |" + str(args.qt*100) + "% |"

    return args

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

main()
