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
from pprint import pprint

COIL_TURNS = 5
COMB_CSV_FILE = "combined.csv"
CSV_DIRECTORY = "out"

def main():
    plot_settings()
    (df, data_dep, colsw_dep, colsw_data_dep) = get_combined_csv()

    # Plot Data Dependency 
    fig, axarr1 = plt.subplots( 1, 2, sharey=True, sharex=False, figsize=(16,10))
    plt_datadep = sns.barplot( 
        x="vendor", y="overhead", ax=axarr1[0], hue="dimm", data=data_dep[(data_dep["one_ratio"] == 0.5)], palette="Set2", 
    )
    plt_datadep.set_ylabel("Energy Overhead")
    plt_datadep.set_xlabel("Vendor")
    axarr1[0].set_title("Data Dependency (Baseline: All 0s) (One Ratio:  50%)")
    
    plt_datadep = sns.barplot( 
        x="vendor", y="overhead", ax=axarr1[1], hue="dimm", data=data_dep[(data_dep["one_ratio"] == 1)], palette="Set2"
    )
    plt_datadep.set_ylabel("")
    plt_datadep.set_xlabel("Vendor")
    axarr1[1].set_title("Data Dependency (Baseline: All 0s) (One Ratio: 100%)")

    plt.ylim(0.97, 1.03)
    plt.savefig("datadependency.pdf")

    # Plot Column Switch 
    fig, ax2 = plt.subplots( 1, 1, sharey=True, sharex=False, figsize=(16,10))
    plt_colswdep = sns.barplot( data=colsw_dep,
        x="vendor", y="overhead", hue="dimm",  ax=ax2, palette="Set2"
    )
    plt_colswdep.set_ylabel("Energy Overhead")
    plt_colswdep.set_xlabel("Vendor")
    ax2.set_title("Column Switch (Baseline: Read 0s from the same column) (All 0s)")
    plt.ylim(0.97, 1.03)
    plt.savefig("columnswitch.pdf")
    
    # Plot Data of Interleaving 2 Banks
    fig, axarr3 = plt.subplots( 1, 2, sharey=True, sharex=False, figsize=(16,10))
    plt_colsw_data_dep = sns.barplot( data=colsw_data_dep[(colsw_data_dep["one_ratio"]==0.5)],
        x="vendor", y="overhead", hue="dimm",  ax=axarr3[0], palette="Set2"
    )
    plt_colsw_data_dep.set_ylabel("Energy Overhead")
    plt_colsw_data_dep.set_xlabel("Vendor")
    axarr3[0].set_title("Data Dep. of Interleaving 2 Banks (Baseline: Both 0s) (One Ratio:  50%)")

    plt_colsw_data_dep = sns.barplot( data=colsw_data_dep[(colsw_data_dep["one_ratio"]==1)],
        x="vendor", y="overhead", hue="dimm",  ax=axarr3[1], palette="Set2"
    )
    plt_colsw_data_dep.set_ylabel("")
    plt_colsw_data_dep.set_xlabel("Vendor")
    axarr3[1].set_title("Data Dep. of Interleaving 2 Banks (Baseline: Both 0s) (One Ratio: 100%)")
    plt.ylim(0.97, 1.03)
    plt.savefig("interleaving.pdf")

    # Per DIMM Plots
    fig, axarr = plt.subplots(3,3,sharey=True,sharex=True, figsize=(16,10))
    plt_index = 0
    for dimm , dimm_group in df.groupby("dimm"):
        
        row = plt_index / 3
        col = plt_index % 3 
        plt_index += 1

        g = sns.barplot(data=dimm_group, x="test_id", y="ave_current",  ax=axarr[row][col], palette="Set2")
        axarr[row][col].set_title(dimm)
        g.set_xlabel("")
        g.set_ylabel("")
    
    plt.ylim(20, 70)
    plt.legend(bbox_to_anchor=(1.05, 1), loc=2, borderaxespad=0.) 
    plt.savefig("TestperDimm.pdf")

def generate_cols(row):    
    # Vendor label based on dimm name
    vendor_dict = {
        "crucial": "A",
        "micron" : "A",
        "samsung": "B",
        "hynix"  : "C"
    }
    for key, value in vendor_dict.items():
        if key in row["dimm"]:
            row["vendor"] = value

    return row

def get_combined_csv():
    file_list = get_file_list()
    data_deps = []
    colsw_deps = []
    colsw_data_deps = []
    dfs = []
    for file in file_list:
        # read csv file into dataframe
        df = pd.read_csv(file)
        print file + ": "
        
        # get dimm name
        print "-DIMM"
        file_parts = file.split("_")
        beginning = file_parts[0]
        beginning_parts = beginning.split("/")
        dimm = beginning_parts[-1]
        vendor = get_vendor(dimm)
        df["dimm"] = dimm
        df["vendor"] = vendor
        df["ave_current"] = df["ave"] * 100 / COIL_TURNS
        print df
        dfs.append(df)
        
        # Calculating Dependencies
        print "-Overhead"
        averages = [0 for k in range(6)]
        for test_id , test_group in df.groupby("test_id"):
            averages[test_id] = test_group["ave_current"].mean()
        
        ###############################################################################
        # Analyze Overheads    
        ## 1. Data 
        data_dep_dict={"overhead": [], "one_ratio":[] }
        
        data_dep_dict["one_ratio"].append(0.5)
        data_dep_dict["overhead"].append(float(averages[1])/float(averages[0]))
        data_dep_dict["one_ratio"].append(1)
        data_dep_dict["overhead"].append(float(averages[2])/float(averages[0]))
        
        data_dep = pd.DataFrame(data_dep_dict)
        data_dep["vendor"] = vendor
        data_dep["dimm"]   = dimm 
        data_deps.append(data_dep)

        ## 2. Column Switching
        colsw_dep_dict={"overhead": [] }
        
        colsw_dep_dict["overhead"].append(float(averages[3])/float(averages[0]))
        
        colsw_dep = pd.DataFrame(data_dep_dict)
        colsw_dep["vendor"] = vendor
        colsw_dep["dimm"]   = dimm 
        colsw_deps.append(colsw_dep)

        ## 3. Interleave 2 Columns with Alternating Data
        colsw_data_dep_dict={"overhead": [], "one_ratio":[] }
        
        colsw_data_dep_dict["overhead"].append(float(averages[4])/float(averages[3]))
        colsw_data_dep_dict["one_ratio"].append(0.5)
        colsw_data_dep_dict["overhead"].append(float(averages[5])/float(averages[3]))
        colsw_data_dep_dict["one_ratio"].append(1)

        colsw_data_dep = pd.DataFrame(colsw_data_dep_dict)
        colsw_data_dep["vendor"] = vendor
        colsw_data_dep["dimm"]   = dimm 
        colsw_data_deps.append(colsw_data_dep)

    df              = pd.concat(dfs            , ignore_index=True)
    data_dep        = pd.concat(data_deps      , ignore_index=True)
    colsw_dep       = pd.concat(colsw_deps     , ignore_index=True)
    colsw_data_dep  = pd.concat(colsw_data_deps, ignore_index=True)

    sort_params     = ["vendor"]
    df              = df.sort_values(by=sort_params, ascending=[1]*len(sort_params))
    data_dep        = data_dep.sort_values(by=sort_params, ascending=[1]*len(sort_params))
    colsw_dep       = colsw_dep.sort_values(by=sort_params, ascending=[1]*len(sort_params))
    colsw_data_dep  = colsw_data_dep.sort_values(by=sort_params, ascending=[1]*len(sort_params))

    df.to_csv(CSV_DIRECTORY+"/../df.csv")
    data_dep.to_csv(CSV_DIRECTORY+"/../data_dep.csv")
    colsw_dep.to_csv(CSV_DIRECTORY+"/../colsw_dep.csv")
    colsw_data_dep.to_csv(CSV_DIRECTORY+"/../colsw_data_dep.csv")

    return (df, data_dep, colsw_dep, colsw_data_dep)

def get_file_list():
    output_creation_date = 0.0
    file_list = []
    for file in os.listdir(CSV_DIRECTORY):
        file_path = CSV_DIRECTORY+'/'+file
        if '.csv' in file and file not in COMB_CSV_FILE : #and "samsungo19" not in file:
            print file + " is " + str(file_age_in_seconds(file_path)) + " old."
            if file_age_in_seconds(file_path) > 20*60 or True:
                file_list.append(file_path)

    return file_list

def GenRC(font_size):
    return {'font.size': font_size, 'axes.labelsize': font_size,
          'legend.fontsize': font_size - 1, 'axes.titlesize': font_size,
          'xtick.labelsize': font_size - 1, 'ytick.labelsize': font_size - 1,
          'grid.linewidth': 0.5, 'pdf.fonttype': 42, 'ps.fonttype': 42, }

def plot_settings():
    sns.set_style("whitegrid")
    new_rc = GenRC(12)
    sns.set_context("paper", rc = new_rc)
    plt.rc('pdf', fonttype=42)
    plt.rcParams['lines.linewidth'] = 1.5
    plt.rcParams['mathtext.fontset'] = 'custom'

def file_age_in_seconds(pathname):
    return time.time() - os.stat(pathname)[stat.ST_MTIME]

def get_vendor(dimm) :
    # Vendor label based on dimm name
    vendor_dict = {
        "crucial": "A",
        "micron" : "A",
        "samsung": "B",
        "hynix"  : "C"
    }
    for key, value in vendor_dict.items():
        if key in dimm:
            return value
main()