#!/usr/bin/python

from functions import *

STD_DEV_THRESH = 1.5

def get_tests():
    tests=[]
    
    test_names = [
        # Read Tests
        "actwaitpre", 
        "actprewait", 
        "actrdprewait", 
        "actrdwaitpre",
        "actwrprewait", 
        "actwrwaitpre"  
    ]
    
    access_patterns = []
    for bank in [0,1,2,3,4,5,7]:
      for col in [0,16]:
        access_patterns.append(str(bank)+"_"+str(col))

    for bank_pair in [(0,0),(1,1),(2,2),(3,3),(4,4),(5,5),(7,7),(0,1),(1,2),(2,3),(5,1),(7,3),(3,4)]:
      for col_pair in [(0,16)]:
        access_pattern  = str(bank_pair[0])+"_"+str(col_pair[0])+"-"
        access_pattern += str(bank_pair[1])+"_"+str(col_pair[0])+"-"
        access_pattern += str(bank_pair[0])+"_"+str(col_pair[1])+"-"
        access_pattern += str(bank_pair[1])+"_"+str(col_pair[1])

        access_patterns.append(access_pattern)

    patterns = [ 
        (0x00, 0x00), (0x00, 0x00), (0x00, 0x00), (0x00, 0x00), #   0.0% Toggle  0 Ones 
        (0x01, 0x01), (0x08, 0x08), (0x10, 0x10), (0x80, 0x80), #   0.0% Toggle  2 Ones
        (0x03, 0x03), (0x0c, 0x0c), (0xc0, 0xc0), (0x30, 0x30), #   0.0% Toggle  4 Ones 
        (0x07, 0x07), (0xe0, 0xe0), (0x0e, 0x0e), (0x70, 0x70), #   0.0% Toggle  6 Ones
        (0x0f, 0x0f), (0xf0, 0xf0), (0xaa, 0xaa), (0x55, 0x55), #   0.0% Toggle  8 Ones 
        (0xf8, 0xf8), (0x1f, 0x1f), (0xd9, 0xd9), (0x9d, 0x9d), #   0.0% Toggle 10 Ones
        (0xfc, 0xfc), (0x3f, 0x3f), (0xdb, 0xdb), (0xbd, 0xbd), #   0.0% Toggle 12 Ones 
        (0x7f, 0x7f), (0xfe, 0xfe), (0xef, 0xef), (0xf7, 0xf7), #   0.0% Toggle 14 Ones
        (0xff, 0xff), (0xff, 0xff), (0xff, 0xff), (0xff, 0xff), #   0.0% Toggle 16 Ones 
        (0x00, 0x01), (0x01, 0x00), (0x00, 0x80), (0x80, 0x00), #  12.5% Toggle  1 Ones
        (0x00, 0x0a), (0x0a, 0x00), (0x00, 0xa0), (0xa0, 0x00), #  25.0% Toggle  2 Ones
        (0x00, 0x07), (0x07, 0x00), (0x00, 0x70), (0x70, 0x00), #  37.5% Toggle  3 Ones
        (0x00, 0x0f), (0x0f, 0x00), (0x00, 0xaa), (0xaa, 0x00), #  50.0% Toggle  4 Ones
        (0x00, 0x1f), (0x1f, 0x00), (0x00, 0xab), (0xab, 0x00), #  62.5% Toggle  5 Ones
        (0x00, 0x3f), (0x3f, 0x00), (0x00, 0xbb), (0xbb, 0x00), #  75.0% Toggle  6 Ones
        (0x00, 0x7f), (0x7f, 0x00), (0x00, 0xef), (0xef, 0x00), #  87.5% Toggle  7 Ones
        (0x00, 0xff), (0xff, 0x00), (0x00, 0xff), (0xff, 0x00), # 100.0% Toggle  8 Ones
    ]
   
    unique_patterns = []
    for patt1, patt2 in patterns:
        if patt1 not in unique_patterns:
          unique_patterns.append(patt1)
        if patt2 not in unique_patterns:
          unique_patterns.append(patt2)

    test_id = 1
    for test_name in test_names:
        for addr in access_patterns:
            addr_parts = addr.split("-")
            if len(addr_parts) > 1:
                for (pattern1, pattern2) in patterns: 
                    tests.append({
                        "test_id"  : test_id,
                        "test_name": test_name,
                        "addr_set" : addr,
                        "pattern1" : pattern1,
                        "pattern2" : pattern2
                    })
                    test_id += 1

            else:
                for pattern in unique_patterns:
                    tests.append({
                        "test_id"  : test_id,
                        "test_name": test_name,
                        "addr_set" : addr,
                        "pattern1" : pattern,
                        "pattern2" : pattern
                    })
                    test_id += 1

    return tests

def main():
    start_time = time.time()
    args = parse_arguments(True)
    args.log = False
    tests = get_tests()
    print "I have " + str(len(tests)) + " tests to run! Yeayyy !!!"

    data_dict = {
        "test_name":[],
        "addr_set" :[],
        "pattern1" :[],
        "pattern2" :[],
        "ave_curr" :[],
        "row"      :[]
    }
    
    summ_dict = {
        "test_name":[],
        "addr_set" :[],
        "pattern1" :[],
        "pattern2" :[],
        "ave_curr" :[],
        "std_dev"  :[],
        "row"      :[]
    }

    if not args.sandbox:
        meter = init_meter_connection()
    dataframe = pd.DataFrame(columns=data_dict.keys())
    summframe = pd.DataFrame(columns=summ_dict.keys())
    output_csv  = "./out/"+args.dimm+"_"+str_timestamp()+".csv"
    summary_csv = "./out/"+args.dimm+"_"+str_timestamp()+"_summary.csv"
    print "["+str_timestamp()+"] Starting the experiments. Output: " + output_csv
    tweet("#"+str(args.dimm)+" starting the single read idle tests")
    stopped = True
    for test in tests:

        averages = []
        garbages = []

        attempt  = 0
        remove_max = True
        while len(averages) < args.iter and attempt <= args.iter * 3:
            attempt += 1
            # Starting the test here
            command = run_binary(
                args.dimm, test["test_name"], test["addr_set"], args.row, 
                test["pattern1"], test["pattern2"], sandbox=args.sandbox
            )
            print "["+str_timestamp()+"] " + command + " (" + str(attempt).zfill(2) +") " ,
            stopped = False
            sleep(0.01)
            data_arr = get_measurements(meter, args.sandbox)
            while not stopped: 
                run_binary(args.dimm, "stoploop", sandbox=args.sandbox)
                idle_arr = get_measurements(meter, args.sandbox)
                stopped = (abs(stats.mean(data_arr) - stats.mean(idle_arr)) > 0.5)
            
            # Looping is stopped
            ave_val = stats.mean(data_arr)
            averages.append(ave_val)
            print "Vave: " + str(ave_val) + " ",

            # Calculate the variation
            std_dev = 0
            if len(averages) > 1 :
                std_dev = stats.stdev(averages)
                print "stdev: "+str(std_dev)+" ",

            while ((std_dev > STD_DEV_THRESH) and (len(averages) > 1)):
                if remove_max :
                    max_ave = max(averages)
                    garbages.append(max_ave)
                    averages.remove(max_ave)
                    remove_max = False
                    print "rm_max: " + str(max_ave),
                else :
                    min_ave = min(averages)
                    garbages.append(min_ave)
                    averages.remove(min_ave)
                    remove_max = True
                    print "rm_min: " + str(min_ave),

                if len(averages) > 1:
                    std_dev = stats.stdev(averages)
            print ""

        data_dict, summ_dict = append_data(data_dict, summ_dict, args, test, averages + garbages)
        dataframe = dataframe.from_dict(data_dict)
        dataframe.to_csv(output_csv)
        summframe = summframe.from_dict(summ_dict)
        summframe.to_csv(summary_csv)
        
        # Calculate the progress and estimate the remaining time
        test_id = test["test_id"]
        progress = float(test_id) / float(len(tests)) 
        elapsed_time = float(int(time.time()) - int(start_time))
        average_time = elapsed_time / float(test_id)
        remain_time  = elapsed_time * (1.0 - progress) / progress
        print "["+str_timestamp()+"] Finished test "+str(test_id)+" ",
        print "Progress: "  + "{0:.2f}".format(progress*100.0) + "% ",
        print "Elapsed: "   + stringify_time(elapsed_time),
        print "Remaining: " + stringify_time(remain_time),
        print "Pace: "      + stringify_time(average_time) + "per test."
    tweet("#"+str(args.dimm)+" is done with #iccad tests!")

def run_binary(dimm, test_name, addr_set="0_0", row=0, patt1=0, patt2=0, sandbox=False):
    command  = "./bin/idletest " + dimm
    command += " " + test_name
    command += " " + addr_set
    command += " " + str(row)
    command += " " + str(patt1)
    command += " " + str(patt2)
    if sandbox:
        command += " 1"
    sp.call(command, shell=True, stdout=FNULL, stderr=sp.STDOUT)
    return command

def parse_arguments(print_flag = False):
    parser = argparse.ArgumentParser()
    parser.add_argument('dimm', metavar='dimm', type=str, default="SANDBOX",
       help='DIMM Name')
    parser.add_argument('iter', metavar='iter', type=int, default=10,
       help='# of Iterations')
    parser.add_argument('--sandbox', action='store_true', default=False,
       help='Run in Sandbox Mode')
    args=parser.parse_args()
    args.row=17868
    
    print "DIMM Name         : " + args.dimm
    print "# of Iterations   : " + str(args.iter)

    return args

def append_data(data_dict, summ_dict, args, test, data_arr):
    std_dev = stats.pstdev(data_arr)
    remove_max = True
    print "["+str_timestamp()+"] Recording test...",
    while ((std_dev > STD_DEV_THRESH) and (len(data_arr) > 1)):
        if remove_max :
            max_ave = max(data_arr)
            data_arr.remove(max_ave)
            remove_max = False
            print "rm_max: " + str(max_ave),
        else :
            min_ave = min(data_arr)
            data_arr.remove(min_ave)
            remove_max = True
            print "rm_min: " + str(min_ave),
        std_dev = stats.pstdev(data_arr) 
    print ""

    for data_point in data_arr:
        data_dict["test_name"].append(test["test_name"])
        data_dict["addr_set"].append(test["addr_set"])
        data_dict["pattern1"].append(test["pattern1"])
        data_dict["pattern2"].append(test["pattern2"])
        data_dict["row"].append(args.row)
        data_dict["ave_curr"].append(data_point * 20)

    summ_dict["test_name"].append(test["test_name"])
    summ_dict["addr_set"].append(test["addr_set"])
    summ_dict["pattern1"].append(test["pattern1"])
    summ_dict["pattern2"].append(test["pattern2"])
    summ_dict["row"].append(args.row)
    summ_dict["ave_curr"].append(stats.mean(data_arr) * 20)
    summ_dict["std_dev"].append(stats.pstdev(data_arr) * 20)

    return data_dict, summ_dict

main()