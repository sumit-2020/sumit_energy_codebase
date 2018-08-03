#!/usr/bin/python
import os, sys, re, time, copy, datetime
import numpy as np
import pandas as pd

from os         import listdir
from os.path    import isfile, join
from termcolor  import colored

class Histfile:
    # Static Variables
    keys_confs        = ['dimm_name','tRCD','tRP','temp','vdd']
    keys_cls_w_err    = ['dimm','bank0','bank1','bank2','bank3','bank4','bank5','bank6','bank7']
    keys_num_err_hist = []

    merged_directory  = "timing_histogram_mem"

    max_err   = 256
    num_banks = 8
    num_rows  = 32768
    num_cls   = 128

    # Static Functions
    @staticmethod
    def get_conf_keys():
        return Histfile.keys_confs

    @staticmethod
    def get_keys_cls_w_err_cntrs() :
        return Histfile.keys_cls_w_err

    @staticmethod
    def get_keys_num_err_hist_cntrs() :
        if not Histfile.keys_num_err_hist:
            Histfile.keys_num_err_hist = [] #list(Histfile.keys_confs)
            for i in range(Histfile.max_err + 1) :
                Histfile.keys_num_err_hist.append('de'+str(i))

            for i in range(Histfile.num_banks) :
                for j in range(Histfile.max_err + 1):
                    Histfile.keys_num_err_hist.append('b'+str(i)+'e'+str(j))
        return Histfile.keys_num_err_hist

    @staticmethod
    def get_keys_num_err_hist_row() :
        ret_keys = list(Histfile.get_conf_keys() + Histfile.get_keys_num_err_hist_cntrs())
        return ret_keys

    @staticmethod
    def get_keys_cls_w_err_row() :
        ret_keys = list(Histfile.get_conf_keys() + Histfile.get_keys_cls_w_err_cntrs())
        return ret_keys

    # Strategy
    merged_parsing = False

    # File properties
    path = ""
    dims = []
    found_max_err = 0

    #Data array and counters
    dataset = []
    dataset2 = []
    per_bank_err = []
    per_bank_cls_w_err = []
    per_dimm_cls_w_err = 0
    dimm_err = 0

    # Summary Line
    num_err_hist        = {}
    summary_cls_w_err   = {}
    summary_confs       = {}

    def __init__(self, path, merged_parsing):
        # Record the inputs
        self.path = path
        self.dims = [Histfile.num_banks, Histfile.num_rows, Histfile.num_cls]
        self.merged_parsing = merged_parsing

        self.dataset = np.ndarray(shape=(self.dims), dtype=np.uint8)
        if merged_parsing :
            self.dataset2 = np.ndarray(shape=(self.dims), dtype=np.uint8)

        self.per_bank_err = [0] * self.dims[0]
        self.per_bank_cls_w_err = [0] * self.dims[0]
        self.per_dimm_cls_w_err = 0
        self.found_max_err = 0

        self.num_err_hist = dict.fromkeys(Histfile.get_keys_num_err_hist_cntrs(),-1)
        self.summary_cls_w_err = dict.fromkeys(Histfile.get_keys_cls_w_err_cntrs(),0)
        self.summary_confs= dict.fromkeys(Histfile.get_conf_keys(),None)

        self.set_configurations(self.path)
        self.parse_histogram(self.path)

        # If the same file exists in the other directory, add it up.
        if merged_parsing :
            dir_parsing = self.path.split('/')
            if (dir_parsing[-3] != Histfile.merged_directory) and ('out_fix' not in dir_parsing):
                dir_parsing[-3] = Histfile.merged_directory
                merge_path = "/".join(dir_parsing)
                if os.path.isfile(merge_path):
                    log_it(colored(self.path+" has a sibling at "+merge_path+". Files are being merged",'yellow',attrs=['bold']),"\n")
                    self.parse_histogram(merge_path)

    # Increment the index of the cacheline in 3D space considering the boundary conditions
    def increment(self, indices) :
        for i in reversed(range(len(self.dims))) :
            indices[i] = indices[i] + 1
            if indices[i] < self.dims[i] :
                return indices
            else :
                indices[i] = 0
        return indices

    # Scan the binary histogram files and create a 3D list
    def parse_histogram(self,path) :
        log_it("Parsing starts for " + colored(path, attrs=['bold']) , "\n")
        byte_stream = np.fromfile(path, dtype=np.uint8)
        indices = [0] * len(self.dims)
        for b in byte_stream :
            self.dataset[indices[0]][indices[1]][indices[2]] += b
            self.per_bank_err[indices[0]] += b
            self.dimm_err += b
            indices = self.increment(indices)

    # Parse the file name and extract the experimental parameters
    def set_configurations(self,path) :
        dir_parsing = path.split('/')
        params = dir_parsing[-1].split('_')
        directory = dir_parsing[-3]+"/"+dir_parsing[-2]
        # self.summary_confs['directory'] = directory
        self.summary_confs['dimm_name'] = params[0]
        self.summary_confs['tRCD'] = int(re.findall('\d+',params[1])[0])
        self.summary_confs['tRP']  = int(re.findall('\d+',params[2])[0])
        self.summary_confs['temp'] = int(re.findall('\d+',params[4])[0])
        self.summary_confs['vdd']  = float(re.findall('\d+\.\d+',params[5])[0])
        # dict_log("Summary Conf Info ", self.summary_confs)

    def summary(self):
        for bank in range(self.dims[0]):
            print 'Bank %d: err = %d |' % (bank, self.per_bank_err[bank]),
        print 'Total err %d' % self.dimm_err

    # Count the number of cachelines which has at least 1 error
    def cnt_cls_w_err(self, bank):
        self.per_bank_cls_w_err[bank] = 0
        for r in self.dataset[bank] :
            for c in r :
                if c > 0 :
                    self.per_bank_cls_w_err[bank] += 1
        self.per_dimm_cls_w_err += self.per_bank_cls_w_err[bank]

    # Create a histogram, based on the number of errors in bank and dimm levels
    def get_num_err_histogram(self,bank):
        log_text  = "Generating dataframe for " + self.summary_confs['dimm_name']
        log_text += " bank "  + str(bank) + " @ "
        log_text += "  tRCD: " + str(self.summary_confs['tRCD'])
        log_text += "  tRP : " + str(self.summary_confs['tRP'])
        log_text += "  temp: " + str(self.summary_confs['temp'])
        log_text += "  vdd : " + str(self.summary_confs['vdd'])

        for r in self.dataset[bank] :
            for c in r :
                if self.num_err_hist['b'+str(bank)+'e'+str(c)] == -1 :
                    self.num_err_hist['b'+str(bank)+'e'+str(c)] = 0
                if self.num_err_hist['de'+str(c)] == -1 :
                    self.num_err_hist['de'+str(c)] = 0

                self.num_err_hist['b'+str(bank)+'e'+str(c)] += 1
                self.num_err_hist['de'+str(c)] += 1

                if c > self.found_max_err :
                    log_it("bank " + str(bank) + "max err is " + str(c))
                    log_it("related row : ","")
                    print r
                    self.found_max_err = c

        for r in self.dataset[bank] :
            for c in range(self.found_max_err) :
                if self.num_err_hist['de'+str(c)] == -1 :
                    self.num_err_hist['de'+str(c)] = 0
                if self.num_err_hist['b'+str(bank)+'e'+str(c)] == -1 :
                    self.num_err_hist['b'+str(bank)+'e'+str(c)] = 0

        log_text += " | max # of errors so far is: " + str(self.found_max_err)
        log_it(log_text)

    def get_cachelines_w_error(self):
        self.per_dimm_cls_w_err = 0
        for i in range(self.dims[0]):
            self.cnt_cls_w_err(i)
        self.summary_cls_w_err['dimm' ] = self.per_dimm_cls_w_err
        self.summary_cls_w_err['bank0'] = self.per_bank_cls_w_err[0]
        self.summary_cls_w_err['bank1'] = self.per_bank_cls_w_err[1]
        self.summary_cls_w_err['bank2'] = self.per_bank_cls_w_err[2]
        self.summary_cls_w_err['bank3'] = self.per_bank_cls_w_err[3]
        self.summary_cls_w_err['bank4'] = self.per_bank_cls_w_err[4]
        self.summary_cls_w_err['bank5'] = self.per_bank_cls_w_err[5]
        self.summary_cls_w_err['bank6'] = self.per_bank_cls_w_err[6]
        self.summary_cls_w_err['bank7'] = self.per_bank_cls_w_err[7]
        return merge_two_dicts(self.summary_confs, self.summary_cls_w_err)

    def get_num_err_hist_line(self) :
        for i in range(Histfile.num_banks) :
            self.get_num_err_histogram(i)
        return merge_two_dicts(self.summary_confs, self.num_err_hist)

#####################################################################
### Helper Functions                                              ###
#####################################################################
def get_histogram_files(file_list,directory,scanned_file_list,ignore_directory):
    hist_files = os.listdir(directory)
    for f in hist_files :
        full_path = directory + "/" + f
        modified_date = os.path.getmtime(full_path)
        now = time.time() #datetime.fromtimestamp(time.time())
        if f[-5:] == ".hist" and "int" in f :
            log_it(f,'')
            if check_file_hist(full_path,scanned_file_list) and ('out_fix' not in directory):
                print " the file is scanned before"

            elif ignore_directory and any(f in fl for fl in file_list) :
                print colored(" the file will be scanned in another process",'yellow')

            else :
                # check if the file is old enough
                if (now - modified_date > 24 * 60 * 60) :
                    print colored(" old_enough --> appended",'green')
                    file_list.append(full_path)
                else :
                    print colored(" too young",'red')
    return file_list

def get_list_of_summarized_files(summary_file, columns):
    scanned_file_list = []
    print "Parsing " + colored(summary_file,attrs=['bold']) + " ..."
    if os.path.isfile(summary_file) :
        csv_data = pd.read_csv(summary_file,names=columns,
                        skipinitialspace=True, header=0)
        for i,name in enumerate(csv_data['dimm_name']) :
            # file_name  = str(csv_data.iloc[i]['directory']) + "/"
            file_name  = str(csv_data.iloc[i]['dimm_name'])
            file_name += "_rcd"  + str(int(csv_data.iloc[i]['tRCD']))
            file_name += "_rp"   + str(int(csv_data.iloc[i]['tRP']))
            file_name += "_ret"  + str(0)
            file_name += "_temp" + str(int(csv_data.iloc[i]['temp']))
            file_name += "_volt" + str(csv_data.iloc[i]['vdd'])
            # print "Summary found for " + file_name
            scanned_file_list.append(file_name)
    else :
        print "Summary file does not exist"
    return scanned_file_list

def check_file_hist(file_name, file_list) :
    for fl in file_list :
        if fl in file_name :
            return True
    return False

# Given two dicts, merge them into a new dict as a shallow copy.
def merge_two_dicts(x, y):
    z = x.copy()
    z.update(y)
    return z

def dict_log(name,dictionary):
    print "\n" + colored(name + " log :",attrs=['bold'])
    print "======================================"
    for key , value in dictionary.iteritems() :
        print colored(key + "\t: ",'green',attrs=['bold']) + str(value) + "\t",
    print ""

def log_it(text, end="\n") :
    timestamp = time.strftime("%m/%d/%y %H:%M:%S")
    print "[" + timestamp + "] : " + text + end,
