#!/usr/bin/python

from functions import *

def get_tests(dimm):

    tests = []
    test_addr_set_pairs = [
        ("singlebanksinglecol","d00"), ("singlebanksinglecol","d01"), ("singlebanksinglecol","d02"),
        ("singlebanksinglecol","d03"), ("singlebanksinglecol","d04"), ("singlebanksinglecol","d05"),
        ("singlebanksinglecol","d06"), ("singlebanksinglecol","d07")
    ]

    for pair in test_addr_set_pairs:
        test = pair[0]
        addr_set = pair[1]

        # 0% NumberOf1s: 0
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x00 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x00 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x00 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x00, "patt2": 0x00 })
        
        # 0% NumberOf1s: 2
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x01, "patt2": 0x01 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x02, "patt2": 0x02 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x08, "patt2": 0x08 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x20, "patt2": 0x20 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x80, "patt2": 0x80 })

        # 0% NumberOf1s: 4
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x05, "patt2": 0x05 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x0a, "patt2": 0x0a })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x50, "patt2": 0x50 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xa0, "patt2": 0xa0 })

        # 0% NumberOf1s: 6
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x07, "patt2": 0x07 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x70, "patt2": 0x70 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xd0, "patt2": 0xd0 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x0d, "patt2": 0x0d })

        # 0% NumberOf1s: 8
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x5c, "patt2": 0x5c })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xb3, "patt2": 0xb3 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x9a, "patt2": 0x9a })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x2b, "patt2": 0x2b })

        # 0% NumberOf1s: 10
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x1f, "patt2": 0x1f })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xf4, "patt2": 0xf4 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xb5, "patt2": 0xb5 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xce, "patt2": 0xce })        
        
        # 0% NumberOf1s: 12
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x3f, "patt2": 0x3f })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xf3, "patt2": 0xf3 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xd7, "patt2": 0xd7 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x7d, "patt2": 0x7d })

        # 0% NumberOf1s: 14
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0x7f, "patt2": 0x7f })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xf7, "patt2": 0xf7 })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xdf, "patt2": 0xfd })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xfd, "patt2": 0xdf })

        # 0% NumberOf1s: 16
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })
        tests.append({"test_n": str(test), "addr_set": str(addr_set), "patt1": 0xff, "patt2": 0xff })

    return tests

def __main__():
    STD_DEV_THRESH = 1.5 # 1.5mV = 30mA
    args = parse_arguments(True)
    args.log = False
    tests = get_tests(args.dimm)

    print "I have " + str(len(tests)) + " tests to run! Yeayyy !!!"
    clean_start()

    data_dict = {
        "test_id"  :[],
        "test_n"   :[],
        "addr_set" :[],
        "patt1"    :[],
        "patt2"    :[],
        "raw_data" :[],
        "min"      :[],
        "max"      :[],
        "ave"      :[],
        "stdev"    :[],
        "row"      :[]
    }

    trashed_dict = {
        "test_id"  :[],
        "test_n"   :[],
        "addr_set" :[],
        "patt1"    :[],
        "patt2"    :[],
        "raw_data" :[],
        "min"      :[],
        "max"      :[],
        "ave"      :[],
        "stdev"    :[],
        "row"      :[]
    }

    meter = init_meter_connection()
    dataframe = pd.DataFrame(columns=data_dict.keys())
    trashframe= pd.DataFrame(columns=trashed_dict.keys())
    output_csv = "./out/"+args.dimm+"_"+str_timestamp()+".csv"
    trash_csv = "./out/"+args.dimm+"_"+str_timestamp()+"_trash.csv"
    
    print "["+str_timestamp()+"] Starting the experiments. Output: " + output_csv
    tweet("#"+str(args.dimm)+" starting #singlecoltest")
    for test_id, test in enumerate(tests) :
        progress = float(test_id) / float(len(tests)) * 100.0
        print "["+str_timestamp()+"] Progress: " + str(progress) + " % "

        averages = []
        garbages = []
        attempt  = 0
        remove_max = True
        while len(averages) < args.iter and attempt <= args.iter * 3:
            attempt += 1
            # Measure a new set of values
            #print test
            run_binary(args.dimm,test["test_n"],test["addr_set"],args.row,test["patt1"],test["patt2"],attempt)
            #quit()
            sleep(0.01)
            data_arr = get_measurements(meter)
            run_binary(args.dimm,"stoploop")
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
        data_dict = append_data(data_dict, test_id, test, averages, args.row)
        dataframe = dataframe.from_dict(data_dict)
        dataframe.to_csv(output_csv)

        if len(garbages) > 0 :
            trashed_dict = append_data(trashed_dict, test_id, test, garbages, args.row)
            trashframe = trashframe.from_dict(trashed_dict)
            trashframe.to_csv(trash_csv)
  
    tweet("#"+str(args.dimm)+" is done!")

def run_binary(dimm,test,addr_set="r01",row=17868,patt1=0,patt2=0, attempt=0):
    command  = "./bin/toggletest " + dimm
    command += " " + test
    command += " " + addr_set
    command += " " + str(row)
    command += " " + str(patt1)
    command += " " + str(patt2)
    # command += " " + str(patt3)
    # command += " " + str(1)

    if test != "stoploop":
        print "["+str_timestamp()+"] " + command + " (" + str(attempt).zfill(2) +") ", 

    command += "  > ./log_out/" + "_" + dimm
    command += "_" + test
    command += "_" + addr_set
    command += "_" + str(row)
    command += "_" + str(patt1)
    command += "_" + str(patt2)
    command += ".log"

    sp.call(command, shell=True)

def parse_arguments(print_flag = False):
    parser = argparse.ArgumentParser()
    parser.add_argument('dimm', metavar='dimm', type=str, default="SANDBOX",
        help='DIMM Name')
    parser.add_argument('iter', metavar='iter', type=int, default=10,
        help='# of Iterations')
    parser.add_argument('row', metavar='row', type=int, default=17868,
        help='Row number')
    parser.add_argument('--nocam', action='store_true', default=False,
        help='No Camera in the measurement part')

    args = parser.parse_args()

    if (print_flag) :
        print "DIMM Name         : " + args.dimm
        print "# of Iterations   : " + str(args.iter)
        print "Row number        : " + str(args.row)

    return args

def append_data(data_dict, test_id, test, data_arr, row):
    raw_data = ','.join(map(str, data_arr))
    data_dict["test_id"].append(test_id)
    data_dict["test_n"].append(test["test_n"])
    data_dict["addr_set"].append(test["addr_set"])
    data_dict["patt1"].append(test["patt1"])
    data_dict["patt2"].append(test["patt2"])
    data_dict["raw_data"].append(raw_data)
    data_dict["min"].append(min(data_arr))
    data_dict["max"].append(max(data_arr))
    data_dict["ave"].append(stats.mean(data_arr))
    data_dict["stdev"].append(stats.pstdev(data_arr))
    data_dict["row"].append(row)
    return data_dict

__main__()
