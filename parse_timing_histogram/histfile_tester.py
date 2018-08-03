#!/usr/bin/python
from histfile import Histfile

import sys

def main():
    #"out/VIRT_rcd2_rp2_ret0_temp20_volt1.500000.hist"
	file_name = sys.argv[1]
	print "Creating histfile object by using the file: " + file_name

	my_histfile = Histfile(file_name, [8,32768,128])
	my_histfile.summary()
	# my_histfile.export_csv()

main()
