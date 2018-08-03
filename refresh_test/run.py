#!/usr/bin/python

import os, sys
import argparse
import time
import subprocess as sp
import datetime

def get_args ():
    parser = argparse.ArgumentParser()
    parser.add_argument('dimm', metavar='dimm', type=str  , default="test"
        , help='DIMM name')
    parser.add_argument('vdd' , metavar='vdd' , type=float, default=1.35
        , help='VDD')
    parser.add_argument('iter', metavar='iter', type=int  , default=1
        , help='# of iterations for the experiment')

    args = parser.parse_args()
    return args
def run_drain_buffers(iter):
    for i in range(iter) :
        sp.call("../drain_buffers/bin/safari_mc_test",shell=True)

def main():
    args = get_args()

    trcd_vals = {
        1.1     : 8,
        1.125   : 8,
        1.15    : 7,
        1.2     : 6,
        1.35    : 6
    }

    trp_vals = {
        1.1     : 8,
        1.125   : 8,
        1.15    : 7,
        1.2     : 6,
        1.35    : 6
    }

    trcd = trcd_vals[args.vdd]
    trp = trp_vals[args.vdd]

    for i in range(args.iter) :
        now = datetime.datetime.now()
        dayonfolder  = str(now.year) + "." + str(now.month) +"."+ str(now.day)
        timeonfolder = str(now.hour) + "." + str(now.minute)
        dateonfolder = dayonfolder + "." + timeonfolder
        foldername = "newout_"+args.dimm+"_"+str(args.vdd)

        if not os.path.isdir(foldername) :
            print "Creating directory: " + foldername
            os.mkdir(foldername)
        print
        for ret_time in [64,128,256,512,1024,1536,2048] :
            ret_val = 1
            max_try = 3
            try_num = 0
            while ret_val !=0 and try_num < max_try :
                run_drain_buffers(3)
                shell_command = "./bin/ret_test"
                shell_command += " " + str(ret_time)
                shell_command += " " + str(trcd)
                shell_command += " " + str(trp)
                printlog("Starting '"+shell_command+"'")
                f=open(foldername+"/"+ str(ret_time) + "ms_err_"+dateonfolder+".log", 'w')
                g=open(foldername+"/"+ str(ret_time) + "ms_out_"+dateonfolder+".log", 'w')
                FNULL=open(os.devnull, 'w')
                ret_val = sp.call(shell_command, shell=True,stderr=f, stdout=g)
                printlog("Returned: " + str(ret_val))
                try_num += 1


def printlog(text) :
    now = datetime.datetime.now()
    datetext  = "[" + str(now.year) + "/" + str(now.month) + "/" + str(now.day)
    datetext += " " + str(now.hour) + ":" + str(now.minute) + "]"

    print datetext + " " + text

main()
