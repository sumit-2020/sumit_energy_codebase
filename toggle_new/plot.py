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
import statistics as stats

COIL_TURNS = 5

def calc_percentage(row) :
    row["rd_percentage"] = "{:2.2f}".format(float(row["num_reads"] * 400) / (row["num_reads"] * 4 + 18))
    return row

def main():
    plot_settings()

    df = get_combined_csv("./out","combined.csv")
    summarize(df)

    for v in ["A","B","C"] :
        # Plot Single Bank Multiple Columns
        df_single = df[(df.test_n == "singlebankmulticol") & (df.vendor == v)]
        fig, ax = plt.subplots(figsize=(12.8,9.6))
        g = sns.barplot(
            data=df_single,
            x="addr_set_text",
            y="ave_current",
            ax=ax,
            hue="pattern"
        ) 
        # for item in g.get_xticklabels():
        #     item.set_rotation(75)
        g.set_title('Single Bank Multiple Cols Test for Vendor '+v)
        g.set_ylabel("Ave Current (mA)")
        g.set_xlabel("Access Pattern")

        fig.savefig("plot_out/patteffect_singlebank_vendor"+v+".pdf")

        # Plot Multiple Banks Multiple Columns
        df_multi = df[(df.test_n == "multibankmulticol") & (df.vendor == v)]
        fig, ax = plt.subplots(figsize=(12.8,9.6))
        g = sns.barplot(
            data=df_multi,
            x="addr_set_text",
            y="ave_current",
            ax=ax,
            hue="pattern"
        ) 

        # for item in g.get_xticklabels():
        #     item.set_rotation(75)
        g.set_title('Multiple Banks Multiple Cols Test for Vendor '+v)
        g.set_ylabel("Ave Current (mA)")
        g.set_xlabel("Access Pattern")

        fig.savefig("plot_out/patteffect_multibank_vendor"+v+".pdf")

    plt.show()

def summarize(df) :
    group_keys = ["addr_set","row","test_n","dimm","vendor","pattern"]
    new_dict = {
        "test_type"       : [],
        "access_pattern"  : [],
        "average"         : [],
        "data_pattern"    : [],
        "row"             : [],
        "stdev"           : [],
        "dimm"            : [],
        "vendor"          : []
    }
    for test, test_group in df.groupby(group_keys) :
        average = new_dict["average"]
        ave_ave = stats.mean(test_group["ave_current"])
        average.append(ave_ave)
        new_dict["access_pattern"].append(test_group.iloc[0]["addr_set_text"])
        new_dict["data_pattern"].append(test_group.iloc[0]["pattern"])
        new_dict["vendor"].append(test_group.iloc[0]["vendor"])
        new_dict["dimm"].append(test_group.iloc[0]["dimm"])
        new_dict["row"].append(test_group.iloc[0]["row"])
        new_dict["test_type"].append(test_group.iloc[0]["test_n"])
        new_dict["stdev"].append(stats.pstdev(test_group["ave_current"]))

    new_df = pd.DataFrame(columns=new_dict.keys())
    new_df = new_df.from_dict(new_dict)
    new_df.to_csv("summary.csv")


def generate_pattern_str(row):
    row["patt"] = '{0:08b}'.format(row["pattern"])
    return row

def plot_read_effect(df,plot_name):
    df = df[(df.vendor=="A")]

    fig, ax = plt.subplots(figsize=(6.4,4.8))

    g = sns.pointplot(
                    data=df,
                    x="test_name",
                    y="ave_current",
                    hue="patt",
                    size=5,
                    legend=True,
                    ax=ax
                    # palette=color_pal
                ) 

def plot_ones_distribution(df,plot_name):
    one_ratio_list = [25.0,50.0,75.0]
    test_list      = ["idd0","idd1","actrdrdpre"]
    fig, axarr = plt.subplots(
                len(one_ratio_list),len(test_list), 
                sharey=True, sharex=True, 
                figsize=(16,10)
            )
    # ax = fig.add_subplot(111)
    for x,one_r in enumerate(one_ratio_list):
        for y,t in enumerate(test_list):
            title = "One Ratio " + str(one_r) + " Test " + t + " "
            subdf = df[ (df.one_ratio == one_r) & (df.test_name == t) ]
            sort_params = ["row_bin","row"]
            subdf = subdf.sort_values(by=sort_params, ascending=[1]*len(sort_params))
            subdf = subdf.apply(lambda row: generate_pattern_str(row), axis=1)
            print subdf
            # subdf["hue_val"] = "V: " + subdf["vendor"].map(str) 
            # subdf["hue_val"]+= " P: "  + subdf["patt"].map(str) 
            color_pal = generate_color_palette(subdf[["row","row_bin"]])
            # print color_pal
            if not subdf.empty :
                g = sns.pointplot(
                    data=subdf,
                    x="row",
                    y="ave_current",
                    hue="patt",
                    size=5,
                    legend=True,
                    ax=axarr[x,y]
                    # palette=color_pal
                ) 
                axarr[x,y].set_title(title)
                
                for item in g.get_xticklabels():
                    item.set_rotation(45)
                
                g.set_ylabel("Ave Current (mA)")
                g.set_xlabel("Row Number")

                handles, labels = g.get_legend_handles_labels()
                legend = g.legend(loc="upper right", frameon=True)
                legend.get_frame().set_facecolor('#FFFFFF')

    plt.savefig(plot_name)

def plot_pattern_current(df,plot_name):
    vendor_list = ["A","B","C"]
    test_list   = ["idd0","idd1","actrdrdpre"]

    if max(len(vendor_list),len(test_list)) > 1:
        fig, axarr = plt.subplots(
                len(vendor_list),len(test_list), 
                sharey=True, sharex=True, 
                figsize=(16,10)
            )

        for x,v in enumerate(vendor_list):
            for y,t in enumerate(test_list):
                title = "Vendor " + v + " Test " + t + " "
                subdf = df[ (df.vendor == v) & (df.test_name == t) ]
                sort_params = ["row_bin","row"]
                subdf = subdf.sort_values(by=sort_params, ascending=[1]*len(sort_params))
                color_pal = generate_color_palette(subdf[["row","row_bin"]])
                # print color_pal
                g = sns.pointplot(
                    data=subdf,
                    x="one_ratio",
                    y="ave_current",
                    hue="row",
                    size=5,
                    legend=False,
                    ax=axarr[x,y],
                    palette=color_pal
                ) 

                g.set_ylabel("Ave Current (mA)")
                g.set_xlabel("Row Number")
                axarr[x,y].set_title(title)
                handles,labels = generate_legend(axarr[x,y])
                legend = axarr[x,y].legend(handles, labels, loc='upper left', frameon=True)
                legend.set_zorder(20000)
                legend.get_frame().set_facecolor('#FFFFFF')
                # break
            # break
    else :
        fig, axarr = plt.subplots(figsize=(64,28))
        ax = draw_pointplot(df,vendor_list[0],test_list[0])


        handles,labels = generate_legend(ax)
        legend = ax.legend(handles, labels, loc='upper right', frameon=True)
        legend.set_zorder(20000)
        legend.get_frame().set_facecolor('#FFFFFF')

    plt.savefig(plot_name)

def plot_row_current(df,plot_name):
    df["hue_val"] = df["one_ratio"].map(str) + "%" 

    # X labels
    x = df["row"].unique()

    fig, ax = plt.subplots(figsize=(6.4,2.8))

    g = sns.FacetGrid(df, col="test_name", row="vendor",hue="hue_val"
                        , col_order=["idd1"]
                        , row_order=["B"]
                        , margin_titles=True
                        , legend_out=True
    )
    g = (g.map(plt.scatter, "row", "ave_current", s = 5 ).add_legend())

    g.set_xlabels("Row Number")
    g.set_xticklabels(rotation=90)
    g.set_ylabels("Current Consumption (mA)")
    # plt.legend(bbox_to_anchor=(1.05, 1), loc=2, borderaxespad=0.)
    plt.savefig(plot_name)

def generate_legend(ax) :
    row_types = ["High Energy Rows (512x)","Low Energy Rows"]

    handles, labels = ax.get_legend_handles_labels()
    old_h = [0 , 0]
    new_handles = []
    for h in handles :
        # print h
        # pprint(vars(h))
        # print h._edgecolors
        # quit()
        if (h._edgecolors[0][0] != old_h[0]) :
            new_handles.append(h)
            old_h = h._edgecolors[0]

    return new_handles, row_types

def generate_color_palette(subdf):
    col0 = sns.color_palette('YlOrRd', 1)
    col1 = sns.color_palette('YlGn'  , 1)

    return_palette = []    
    for (row_bin, row_number), row_group in subdf.groupby(["row_bin","row"]):
        # print "Row: " + str(row_number) + " Bin: " +str(row_bin)
        if row_number % 512 == 0:
            for t in col0:
                return_palette.append(tuple(i for i in t))
        else :
            for t in col1:
                return_palette.append(tuple(i for i in t))

    return return_palette

def generate_row_bin(row) :
    # col2 = sns.color_palette('YlGnBu', 1)

    row["row_bin"] = 1

    if (row["row"] % 512  == 0) :
        row["row_bin"] = 0

    return row

def get_file_list(directory, output_file):
    output_creation_date = 0.0

    file_list = []
    # for file in os.listdir(directory):
    #     file_path = directory+"/"+output_file
    #     if '.csv' in file and file in output_file:
    #         print file + " is " + str(file_age_in_seconds(file_path)) + " old."
    #         output_creation_date = os.path.getctime(file_path)
    #         file_list.append(file_path)

    for file in os.listdir(directory):
        file_path = directory+'/'+file
        if '.csv' in file and file not in output_file:
            print file + " is " + str(file_age_in_seconds(file_path)) + " old."
            if file_age_in_seconds(file_path) > 20*60 or True:
                print file + " is included in the list"
                file_list.append(file_path)

    
    return file_list

def get_combined_csv(directory, output_file):
    file_list = get_file_list(directory,output_file)
    dfs = []
    for file in file_list:
        # read csv file into dataframe
        df = pd.read_csv(file)

        print file + ": "
        # add dimm name
        if "dimm" not in df :
            print "-DIMM"
            file_parts = file.split("_")
            beginning = file_parts[0]
            beginning_parts = beginning.split("/")
            df["dimm"] = beginning_parts[-1]

        if "vendor" not in df :
            print "-Vendor"
            # get ratio of ones
            df = df.apply(lambda row: generate_cols(row), axis=1)

        if "ave_current" not in df:
            print "-AveCurrent"
            # convert voltage to current
            df["ave_current"] = df["ave"] * 100 / COIL_TURNS

        if "row_bin" not in df:
            print "-RowBin"
            df = df.apply(lambda row: generate_row_bin(row), axis=1)

        cols = [    "ave","ave_current","bank","dimm","iter","max","min",
                    "pattern","quality","raw_data","row","stdev",
                    "test_name","vendor","one_ratio","row_bin"
                ]
        df = df[cols]

        dfs.append(df)


    df = pd.concat(dfs, ignore_index=True)
    sort_params = ["row_bin"]
    df = df.sort_values(by=sort_params, ascending=[0]*len(sort_params))
    df.to_csv(directory+"/"+output_file)

    return df

def duplicated_varnames(df):
    """Return a dict of all variable names that 
    are duplicated in a given dataframe."""
    repeat_dict = {}
    var_list = list(df) # list of varnames as strings
    for varname in var_list:
        # make a list of all instances of that varname
        test_list = [v for v in var_list if v == varname] 
        # if more than one instance, report duplications in repeat_dict
        if len(test_list) > 1: 
            repeat_dict[varname] = len(test_list)
    return repeat_dict

def generate_cols(row):
    # Concat patterns 
    row["pattern"] = hex(row["patt1"]) + "_" + hex(row["patt2"]) + "_" + hex(row["patt3"])
    row["addr_set_text"] = addr_set_text(row)
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

    row["hue_val"] = "Vendor " + row["vendor"] + ":" + row["pattern"]
    return row

def GenRC(font_size):
    return {'font.size': font_size, 'axes.labelsize': font_size,
          'legend.fontsize': font_size - 1, 'axes.titlesize': font_size,
          'xtick.labelsize': font_size - 1, 'ytick.labelsize': font_size - 1,
          'grid.linewidth': 0.5, 'pdf.fonttype': 42, 'ps.fonttype': 42, }

def plot_settings():
    sns.set_style("whitegrid")
    sns.despine()
    new_rc = GenRC(12)
    sns.set_context("paper", rc = new_rc)
    plt.rc('pdf', fonttype=42)
    plt.rcParams['lines.linewidth'] = 1.5
    plt.rcParams['mathtext.fontset'] = 'custom'

# def calculate_datasheet(dimm, test_name):

def file_age_in_seconds(pathname):
    return time.time() - os.stat(pathname)[stat.ST_MTIME]

def get_combined_csv(directory, output_file):
    file_list = get_file_list(directory,output_file)
    dfs = []
    for file in file_list:
        # read csv file into dataframe
        df = pd.read_csv(file)

        print file + ": "
        # add dimm name
        if "dimm" not in df :
            print "-DIMM"
            file_parts = file.split("_")
            beginning = file_parts[0]
            beginning_parts = beginning.split("/")
            df["dimm"] = beginning_parts[-1]

        if "vendor" not in df :
            print "-Vendor"
            # get ratio of ones
            df = df.apply(lambda row: generate_cols(row), axis=1)

        if "ave_current" not in df:
            print "-AveCurrent"
            # convert voltage to current
            df["ave_current"] = df["ave"] * 100 / COIL_TURNS

        # if "pattern" not in df:
        #     print "-Pattern"
        #     df = df.apply(lambda row: generate_pattern(row), axis=1)
            

        # if "row_bin" not in df:
        #     print "-RowBin"
        #     df = df.apply(lambda row: generate_row_bin(row), axis=1)

        cols = [    "addr_set","ave","iter","max","min","patt1","patt2","patt3",
                    "quality","raw_data","row","stdev","test_id","test_n",
                    "dimm","vendor","pattern","ave_current","hue_val","addr_set_text"
                ]
        df = df[cols]

        df = df[(df.ave < 20)]
        # df.to_csv(directory+"/test.csv")

        dfs.append(df)


    df = pd.concat(dfs, ignore_index=True)
    sort_params = ["addr_set","hue_val"]
    df = df.sort_values(by=sort_params, ascending=[1]*len(sort_params))
    df.to_csv(directory+"/"+output_file)

    return df

def addr_set_text(row) :
    if row["test_n"] == "singlebankmulticol":
        addr_set_dict = {
            "r01" : "Bank 0 Cols 0,1,2",
            "r02" : "Bank 0 Cols 0,2,4",
            "r03" : "Bank 0 Cols 0,16,32",
            "r04" : "Bank 0 Cols 0,29,51",
            "r17" : "Bank 0 Cols 0,1,2",
            "r18" : "Bank 0 Cols 0,2,4",
            "r19" : "Bank 0 Cols 0,16,32",
            "r20" : "Bank 0 Cols 0,29,51"
        }

        return addr_set_dict[row["addr_set"]]

    elif row["test_n"] == "multibankmulticol":
        delimiter = "_"
        addr_set_dict = {
            "r01" : "B0C00"+delimiter+"B1C01"+delimiter+"B2C02",
            "r02" : "B0C00"+delimiter+"B1C02"+delimiter+"B2C04",
            "r03" : "B0C00"+delimiter+"B1C16"+delimiter+"B2C32",
            "r04" : "B0C00"+delimiter+"B1C29"+delimiter+"B2C51",
            "r05" : "B0C00"+delimiter+"B2C01"+delimiter+"B4C02",
            "r06" : "B0C00"+delimiter+"B2C02"+delimiter+"B4C04",
            "r07" : "B0C00"+delimiter+"B2C16"+delimiter+"B4C32",
            "r08" : "B0C00"+delimiter+"B2C29"+delimiter+"B4C51",
            "r09" : "B0C00"+delimiter+"B3C01"+delimiter+"B6C02",
            "r10" : "B0C00"+delimiter+"B3C02"+delimiter+"B6C04",
            "r11" : "B0C00"+delimiter+"B3C16"+delimiter+"B6C32",
            "r12" : "B0C00"+delimiter+"B3C29"+delimiter+"B6C51",
            "r13" : "B1C00"+delimiter+"B3C01"+delimiter+"B5C02",
            "r14" : "B1C00"+delimiter+"B3C02"+delimiter+"B5C04",
            "r15" : "B1C00"+delimiter+"B3C16"+delimiter+"B5C32",
            "r16" : "B1C00"+delimiter+"B3C29"+delimiter+"B5C51",
            "r17" : "B5C00"+delimiter+"B6C01"+delimiter+"B7C02",
            "r18" : "B5C00"+delimiter+"B6C02"+delimiter+"B7C04",
            "r19" : "B5C00"+delimiter+"B6C16"+delimiter+"B7C32",
            "r20" : "B5C00"+delimiter+"B6C29"+delimiter+"B7C51"
        }

        return addr_set_dict[row["addr_set"]]
    else :
        return ""

main()
