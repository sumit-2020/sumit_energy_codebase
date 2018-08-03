#!/usr/bin/python

import pandas as pd
import argparse
import os

parser = argparse.ArgumentParser(description='Process csv files.')
parser.add_argument('--paths', metavar='N', type=str, nargs='+', required=True,
        help='paths of output files. Ex: timing_histogram_mem/out')
parser.add_argument('--out', metavar='F', type=str, required=True,
        help='output file name')
args = parser.parse_args()

processed_files = 0

def walk_dir(dir_path, csv_list):
    # Find all readable csv files in the directory
    for root, dirs, files in os.walk(dir_path):
        for f in files:
            if 'csv' == f[-3:]:
                csv_f = os.path.join(root, f)
                if os.path.getsize(csv_f)>0 and os.access(csv_f, os.R_OK):
                    csv_list.append(csv_f)

def create_pd_frame(central_df, csv):
    df = pd.read_csv(csv)
    df_len = len(df.index)

    # Check if the file is currently being updated or corrupted
    if df_len < 8:
        print 'Unfilled csv %s' % csv
        return

    # Add other attributes
    fname = os.path.basename(csv)
    tokens = fname.split('_')
    if len(tokens) > 9 and not tokens[-1]=='fix.csv':
        print 'Unmatched csv %s' % csv
        return
    df['DIMM'] = pd.Series([tokens[0] for i in xrange(df_len)])
    df['tRCD'] = pd.Series([tokens[1][len('rcd'):] for i in xrange(df_len)])
    df['tRP'] = pd.Series([tokens[2][len('rp'):] for i in xrange(df_len)])
    df['Temperature'] = pd.Series([tokens[4][len('temp'):] for i in xrange(df_len)])
    df['Vdd'] = pd.Series([tokens[5][len('volt'):] for i in xrange(df_len)])
    df['Pattern'] = pd.Series([tokens[6][len('patt0x'):] for i in xrange(df_len)])
    df['Date'] = pd.Series([tokens[7] for i in xrange(df_len)])

    global processed_files
    processed_files += 1
    return pd.concat([df, central_df], ignore_index=True)

def main():
    csv_list = []
    for p in args.paths:
        walk_dir(p, csv_list)

    central_df = None
    for csv in csv_list:
        central_df = create_pd_frame(central_df, csv)

    central_df.to_csv(args.out, index=False)
    print 'Processed %d files || output is %s' % (processed_files, args.out)

if __name__ == '__main__':
    main()
