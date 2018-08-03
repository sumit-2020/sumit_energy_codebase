#!/usr/bin/python

from functions import *

def get_tests(dimm):
  test_names = ["coltest"]
  banks = range(1) # which bank is active
  patterns = [
    0xFF, # 1111 1111
    0xAA, # 1010 1010
    0x00  # 0000 0000
  ]
  return test_names, banks, patterns

def __main__():
  STD_DEV_THRESH = 1.5 # 1.5mV == 30mA
  args = parse_arguments(True)
  args.log = False
  test_names, banks, patterns = get_tests(args.dimm)

  clean_start()

  columns = ["test_name", "bank", "row", "col", "pattern", "raw_data", "avg_voltage", "temp"]
  data_dict = dict((el,[]) for el in columns)
  trsh_dict = dict((el,[]) for el in columns)

  meter = init_meter_connection(args.sandbox)
  output_csv = "./out/" + args.dimm + "_" + str_timestamp() + ".csv"
  trash_csv = "./out/" + args.dimm + "_" + str_timestamp() + "_trash.csv"

  print "[" + str_timestamp() + "] Starting the experiments. Output: " + output_csv

  start_time = time.time()
  num_of_tests = 8 * 3 * 128 * len(patterns) # bank * rows * columns * patterns
  num_of_finished_tests = 0
  for t in test_names:
    tweet("@liuwilliam47 #" + str(args.dimm) + " is starting #ColumnTest")
    for b in banks:
      rows = get_rows_by_dimm(args.dimm,b)
      for r in rows: # 3 rows
        for column in range(128): # 128 columns
          c = column * 8 # due to the difference of definition in dram_codebase
          for p in patterns:
            test = {"test_name":t, "bank":b, "row":r, "col":c, "pattern":p, "temp":args.temp}
            averages = []
            garbages = []
            attempt  = 0
            remove_max = True
            while (len(averages) < args.iter and attempt <= args.iter * 3):
              attempt += 1
              run_binary(args.dimm, t, b, r, c, p, attempt, args.sandbox)
              sleep(0.01)
              data_arr = get_measurements(meter, args.sandbox)
              run_binary(args.dimm, "stop")
              avg_val = stats.mean(data_arr)
              averages.append(avg_val)
              print "Vavg: " + str(avg_val) + " ",

              # Calculuate the variation
              std_dev = 0
      	      if (len(averages) > 1):
       	        std_dev = stats.stdev(averages)
                print "stdev: " + str(std_dev) + " ",
                while ((std_dev > STD_DEV_THRESH) and (len(averages) > 1)):
                  if remove_max :
                    max_avg = max(averages)
                    garbages.append(max_avg)
                    averages.remove(max_avg)
                    remove_max = False
                    print "rm_max: " + str(max_avg),
                  else :
                    min_avg = min(averages)
                    garbages.append(min_avg)
                    averages.remove(min_avg)
                    remove_max = True
                    print "rm_min: " + str(min_avg),

                  if len(averages) > 1:
                    std_dev = stats.stdev(averages)

              print ""

            data_dict = append_data(data_dict, test, averages)
            dataframe = pd.DataFrame.from_dict(data_dict)
            dataframe.to_csv(output_csv)

            if (len(garbages) > 0):
              trsh_dict = append_data(trsh_dict, test, garbages)
              trshframe = pd.DataFrame.from_dict(trsh_dict)
              trshframe.to_csv(trash_csv)

            now = time.time()
            num_of_finished_tests += 1
            progress = float(num_of_finished_tests) / float(num_of_tests)
            elapsed_time = float(int(now) - int(start_time))
            average_time = elapsed_time / float(num_of_finished_tests)
            remain_time  = elapsed_time * (1.0 - progress) / progress
            print "["+str_timestamp()+"] Finished test "+str(num_of_finished_tests)+" ",
            print "Progress: "  + "{0:.2f}".format(progress*100.0) + "% ",
            print "Elapsed: "   + stringify_time(elapsed_time),
            print "Remaining: " + stringify_time(remain_time),
            print "Pace: "      + stringify_time(average_time) + "per test."

  tweet("Hey @liuwilliam47! #" + str(args.dimm) + " is done! Come check on it!")

def run_binary(dimm,test,bank=0,row=0,column=0,pattern=0,attempt=0, sandbox=False):
  command  = "./bin/col_test"
  command += " " + dimm
  command += " " + test
  command += " " + str(pattern)
  command += " " + str(bank)
  command += " " + str(row)
  command += " " + str(column)

  if sandbox:
    command += " 1"

  if not "stop" in test:
      print "["+str_timestamp()+"] " + command,
      print " (" + str(attempt).zfill(2) +") ",

  command += "  > ./log_out/" + "_" + dimm
  command += "_" + test
  command += "_" + str(bank)
  command += "_" + str(row)
  command += "_" + str(column)
  command += "_" + str(pattern)
  command += ".log"

  sp.call(command, shell=True)

def parse_arguments(print_flag = False):
  parser = argparse.ArgumentParser()
  parser.add_argument('dimm', metavar='dimm', type=str, default="SANDBOX",
    help='DIMM Name')
  parser.add_argument('iter', metavar='iter', type=int, default=10,
    help='# of Iterations')
  parser.add_argument('--temp', metavar='temp', type=int, default=20,
    help='Temperature in C')
  parser.add_argument('--sandbox', action='store_true', default=False,
    help='If set, nothing will be sent to FPGA and meter')

  args = parser.parse_args()
  if (print_flag) :
    print "DIMM Name         : " + args.dimm
    print "# of Iterations   : " + str(args.iter)
    print "Temperature       : " + str(args.temp)
    if args.sandbox:
      print "Sandbox mode is active.\n - Nothing will be sent to FPGA.\n - Script mimics meter."

  return args

def append_data(data_dict, test, data_arr):
  raw_data = ','.join(map(str, data_arr))
  for key in test:
    data_dict[key].append(test[key])
  data_dict["raw_data"].append(raw_data)
  data_dict["avg_voltage"].append(stats.mean(data_arr))
  return data_dict

def get_rows_by_dimm(dimm, bank):
  rows_df = pd.read_csv("rows_avg.csv")
  rows_df = rows_df[rows_df.dimm == dimm]

  # see if the bank we have right now is here
  rows_df = rows_df[rows_df.bank == bank]
  rows_df = list(rows_df.row)
  if (rows_df == []): # the bank we're looking at is not here
    # go back and default to row 0
    rows_df = pd.read_csv("rows.csv")
    rows_df = rows_df[rows_df.dimm == dimm]
    rows_df = rows_df[rows_df.bank == 0]
    rows_df = list(rows_df.row)

  # turn everything in list to ints
  return map(lambda x: int(x), rows_df) # just in case of implicit casting errors

def stringify_time(t_sec):
    t_min = int(t_sec) / 60
    t_hrs = int(t_min) / 60
    t_day = int(t_hrs) / 24

    t_hrs = int(t_hrs) % 24
    t_min = int(t_min) % 60
    t_sec = int(t_sec) % 60

    t_str = ""
    if t_day > 0:
        t_str += str(t_day) + "d "
    if t_hrs > 0:
        t_str += str(t_hrs) + "h "
    if t_min > 0:
        t_str += str(t_min) + "m "
    if t_sec > 0:
        t_str += str(t_sec) + "s "

    return t_str

__main__() # gotta actually run the function
