#!/usr/bin/python

from functions import *

NUM_ROWS = 32768
STEP     =    32
STRIDE   =  4096

def main():

    data_dict = {
        "test_name" :[],
        "pattern"   :[],
        "iter"      :[],
        "raw_data"  :[],
        "min"       :[],
        "max"       :[],
        "ave"       :[],
        "stdev"     :[],
        "quality"   :[],
        "bank"      :[],
        "row"       :[],
        "ave_curr"  :[],
        "idle_raw"  :[],
        "idle_ave"  :[],
        "idle_stdev":[],
        "idle_q"    :[]
    }

    args = parse_arguments(True)
    banks = [0,3,4,7]
    rows = generate_list_of_rows()
    patterns = [0,170,255]
    dataframe = pd.DataFrame(columns=data_dict.keys())
    print args
    output_csv = "./out/"+args.dimm+"_"+str_timestamp()+"_bank_row_sweep_idd0.csv"
    for bank in banks:
      tweet("I'm starting the row-sweeping #IDD0 test for #bank"+str(bank)+" of #"+str(args.dimm))
      for row in rows:
        for pattern in patterns :
          i = 0
          attempt = 0
          while i < args.iter and attempt < 3 * args.iter:
            attempt = attempt + 1
            clean_start("./img_out","*")
            run_binary(i,args.dimm,"idd0",bank,row,pattern)
            sleep(0.01)
            data_arr, data_q = measure_loop(args)
            run_binary(i,args.dimm,"stoploop")

            if (data_q > args.qt):
              i = i + 1 
              idle_arr, idle_q = read_meter(args)
              if (idle_q > args.qt) :
                data_dict = append_data(data_dict, "idd0", pattern, i, data_arr, data_q, bank, row, idle_arr, idle_q)
                dataframe = dataframe.from_dict(data_dict)
                dataframe.to_csv(output_csv)
              
            else:
              print "["+str_timestamp()+"] ERROR: Low measurement quality: " + str(data_q)

<<<<<<< HEAD
=======
            idle_arr, idle_q = read_meter(args)
            if (idle_q > args.qt) :
              idle_dict = append_idle(idle_dict, "idd0_idle", pattern, i, idle_arr, idle_q, bank, row)
              idle_df   = idle_df.from_dict(idle_dict)
              idle_df.to_csv(idle_csv)            
>>>>>>> 396970b7d3eec7be3d144583e02f1c0fa3fd953e
    tweet("I'm done with #"+str(args.dimm)+"! Please somebody help me!")



def append_data(data_dict, test_name, pattern, i, data_arr, quality,bank,row,idle_arr,idle_q):
  raw_data = ','.join(map(str, data_arr))
  idle_raw = ','.join(map(str, idle_arr))
  data_dict["test_name"].append(test_name)
  data_dict["pattern"].append(pattern)
  data_dict["iter"].append(i)
  data_dict["raw_data"].append(raw_data)
  data_dict["min"].append(min(data_arr))
  data_dict["max"].append(max(data_arr))
  data_dict["ave"].append(stats.mean(data_arr))
  data_dict["stdev"].append(stats.pstdev(data_arr))
  data_dict["quality"].append(quality)
  data_dict["bank"].append(bank)
  data_dict["row"].append(row)
  data_dict["ave_curr"].append(stats.mean(data_arr)*20)
  data_dict["idle_raw"].append(idle_raw)
  data_dict["idle_ave"].append(stats.mean(idle_arr))
  data_dict["idle_stdev"].append(stats.pstdev(idle_arr))
  data_dict["idle_q"].append(idle_q)

  return data_dict

def append_idle(idle_dict, test_name, pattern, i, idle_arr, quality, bank, row):
  raw_data = ','.join(map(str, idle_arr))
  idle_dict["test_name"].append(test_name)
  idle_dict["pattern"].append(pattern)
  idle_dict["iter"].append(i)
  idle_dict["raw_data"].append(raw_data)
  idle_dict["ave"].append(stats.mean(idle_arr))
  idle_dict["stdev"].append(stats.pstdev(idle_arr))
  idle_dict["quality"].append(quality)
  idle_dict["bank"].append(bank)
  idle_dict["row"].append(row)
  idle_dict["ave_curr"].append(stats.mean(idle_arr)*20)
  return idle_dict

def parse_arguments(print_flag = False):
  parser = argparse.ArgumentParser()
  parser.add_argument('dimm', metavar='dimm', type=str, default="SANDBOX",
       help='DIMM Name')
  parser.add_argument('iter', metavar='iter', type=int, default=10,
       help='# of Iterations')
  parser.add_argument('--nocam', action='store_true', default=False,
       help='No Camera in the measurement part')
  parser.add_argument('--qt', metavar='qt', type=float, default=0.6,
       help='Quality Threshold of Vision: [# of Samples] / [# of Reads]')
  parser.add_argument('--nos', metavar='nos', type=int, default=30,
       help='# of Samples per Read')

  args = parser.parse_args()
  args.log = False

  if (print_flag) :
    print "DIMM Name         : " + args.dimm
    print "# of Iterations   : " + str(args.iter)
    print "Quality           : " + str(args.qt)
    print "Number of Samples : " + str(args.nos)

  return args

def generate_list_of_rows():
  k = 0
  r = 0
  rows = []
  while r < NUM_ROWS:
   rows.append(r)
   if r < STRIDE * k + 512 :
    r += STEP
   else :
    r += STRIDE - 512
    k += 1
  return rows

main()
