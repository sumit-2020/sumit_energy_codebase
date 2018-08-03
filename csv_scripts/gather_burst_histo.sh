#!/bin/sh

timestamp=$(date +%Y_%m_%d_%T)
hostname=$(hostname)
python gather_burst_histo.py --out burst_histo.$hostname.csv --paths ../timing_histogram_mem/out
python gather_burst_histo.py --out burst_histo_fix.$hostname.csv --paths ../timing_histogram_mem/out_fix
