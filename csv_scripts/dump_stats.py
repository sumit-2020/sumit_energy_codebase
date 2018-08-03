#!/usr/bin/python

import sys
import pandas as pd
import argparse

# Arguments
parser = argparse.ArgumentParser()
parser.add_argument('csv_file', metavar='fname', type=str, help='File name')
parser.add_argument('-l', required=False, action='store_true',
        help='List the failure row info.')
parser.add_argument('-file_list', metavar='file_list', type=str, nargs = '+',
        help='File name')
parser.add_argument('-out_csv', metavar='out_csv', type=str, help='File name')
args = parser.parse_args()

results = {'trcd':[], 'trp':[], 'vdd':[], 'ACT':[], 'ErrorACT':[],
'ErrorACTFrac':[], 'ErrBitPerErrACT':[], 'Temperature':[]}

csv_file = args.csv_file

def ParseTokenValue(tokens, name, convert):
    idx = -1
    for i in xrange(len(tokens)):
        if name in tokens[i]:
            idx = i
    return convert(tokens[idx].lstrip(name))

def ParseError(csv_file, args, results):
    print csv_file

    # Panda parse
    df = pd.read_csv(csv_file)
    df_err = df[df['TotalErrorBits'] > 0]
    #err_bits = df['TotalErrorBits'].sum()
    err_bits = df_err['TotalErrorBits'].sum()
    num_act = len(df.index)
    num_failed_act = len(df_err.index)
    print 'Err Act: %d (ACT Failure %8.7f) -> Sum err: %d | Num Act:%d' % (num_failed_act,
            float(num_failed_act)/num_act, err_bits, num_act)
    if num_failed_act > 0:
        print 'ErrBit/failed ACT %f' % (float(err_bits) / num_failed_act)

    # Parse out test configurations
    if args.file_list:
        name_tokens = csv_file.strip().split('_')
        results['trcd'].append(ParseTokenValue(name_tokens, 'rcd', int))
        results['trp'].append(ParseTokenValue(name_tokens, 'rp', int))
        results['vdd'].append(ParseTokenValue(name_tokens, 'volt', float))
        results['ACT'].append(num_act)
        results['ErrorACT'].append(num_failed_act)
        results['ErrorACTFrac'].append(float(num_failed_act)/num_act)
        if num_failed_act > 0:
            results['ErrBitPerErrACT'].append(float(err_bits) / num_failed_act)
        else:
            results['ErrBitPerErrACT'].append(0)
        results['Temperature'].append(ParseTokenValue(name_tokens, 'temp', float))

    # List those rows that have errors
    if args.l:
        pd.set_option('display.max_rows', len(df_err))
        print df_err
############################################################################

# Parse through a list of files
if args.file_list:
    print args.file_list
    for csv_file in args.file_list:
        ParseError(csv_file, args, results)
    df = pd.DataFrame(results)
    df.to_csv(args.out_csv)
else:
    ParseError(csv_file, args, results)
