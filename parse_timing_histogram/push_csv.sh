#!/bin/bash
h=$(hostname)i
rsync combined_summary_cachelines_with_error.csv_* dram@dramg2.andrew.cmu.edu:/home/dram/parse_hist/combined_cls_w_err/$h/
rsync combined_summary_hist_num_errors.csv_* dram@dramg2.andrew.cmu.edu:/home/dram/parse_hist/combined_hist_num_errs/$h/
