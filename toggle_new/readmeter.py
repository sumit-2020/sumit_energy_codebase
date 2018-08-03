import sys
import fnmatch
import serial
sys.path.insert(0, '../src/')
import usbtmc as tmc
import time

conn  = serial.Serial('/dev/ttyS0', 460800, timeout=0.1)
meter = tmc.instrument(tmc.AGILENT_34461A)
meter.write('*RST')
for i in range(10):
  print float(meter.ask('MEAS:VOLT:DC? AUTO'))*1000
  
