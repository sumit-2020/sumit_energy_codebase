#!/usr/bin/python

from functions import *

def get_tests(dimm):

    tests = []
    test_types    = ["idler"]
    active_banks  = [
        # 0 banks are active
        0x00, 0x00, 0x00, 0x00, # 0000_0000
        0x00, 0x00, 0x00, 0x00, # 0000_0000

        # 1 bank is active
        0x01, 0x02, 0x04, 0x08, # 0000_0001 0000_0010 0000_0100 0000_1000
        0x10, 0x20, 0x40, 0x80, # 0001_0000 0010_0000 0100_0000 1000_0000

        # 2 banks are active
        0x03, 0x05, 0x09, 0x11, # 0000_0011 0000_0101 0000_1001 0001_0001
        0x21, 0x41, 0x81,       # 0010_0001 0100_0001 1000_0001
        0x18,                   # 0001_1000

        # 3 banks are active
        0x07, 0x0B, 0x13, 0x23, # 0000_0111 0000_1011 0001_0011 0010_0011
        0x43, 0x83,             # 0100_0011 1000_0011
        0x15, 0xA8,             # 0001_0101 1010_1000

        # 4 Banks are active
        0x0F, 0x17, 0x27, 0x47, # 0000_1111 0001_0111 0010_0111 0100_0111
        0x87,                   # 1000_0111
        0xAA, 0x55, 0xC3,       # 1010_1010 0101_0101 1100_0011

        # 5 banks are active
        0x1F, 0x2F, 0x4F, 0x8F, # 0001_1111 0010_1111 0100_1111 1000_1111
        0x37, 0x57, 0x97, 0xC7, # 0011_0111 0101_0111 1001_0111 1100_0111

        # 6 banks are active
        0x3F, 0x5F, 0x9F, 0xE7, # 0011_1111 0101_1111 1001_1111 1110_0111
        0xCF, 0xAF, 0xB7, 0xD7, # 1100_1111 1010_1111 1011_0111 1101_0111

        # 7 banks are active
        0x7F, 0xBF, 0xDF, 0xEF, # 0111_1111 1011_1111 1101_1111 1110_1111
        0xF7, 0xFB, 0xFD, 0xFE, # 1111_0111 1111_1011 1111_1101 1111_1110

        # 8 banks are active
        0xFF, 0xFF, 0xFF, 0xFF, # 1111_1111
        0xFF, 0xFF, 0xFF, 0xFF  # 1111_1111
    ]

    data_patterns = [
        0x00, # 0 ones
        0x01, # 1 ones
        0x11, # 2 ones
        0x2a, # 3 ones
        0xaa, # 4 ones
        0xad, # 5 ones
        0x3f, # 6 ones
        0xfe, # 7 ones
        0xff, # 8 ones
    ]

    return test_types, active_banks, data_patterns

def __main__():
    STD_DEV_THRESH = 1.5 # 1.5mV = 30mA
    args = parse_arguments(True)
    args.log = False
    (test_types, active_banks, data_patterns) = get_tests(args.dimm)
    clean_start()

    data_dict = {
        "test_n"        :[],
        "active_banks"  :[],
        "pattern"       :[],
        "raw_data"      :[],
        "avg_voltage"   :[],
        "temp"          :[]
    }

    trashed_dict = {
        "test_n"        :[],
        "active_banks"  :[],
        "pattern"       :[],
        "raw_data"      :[],
        "avg_voltage"   :[],
        "temp"          :[]
    }

    meter = init_meter_connection()
    output_csv = "./out/"+args.dimm+"_"+str_timestamp()+".csv"
    trash_csv = "./out/"+args.dimm+"_"+str_timestamp()+"_trash.csv"

    print "["+str_timestamp()+"] Starting the experiments. Output: " + output_csv

    for t in test_types :
        tweet("#"+str(args.dimm)+" is starting #idletest")
        for b in active_banks :
            for p in data_patterns :
                test = {"test_n":t, "active_banks":b, "pattern": p, "temp": args.temp}
                averages = []
                garbages = []
                attempt  = 0
                remove_max = True
                while len(averages) < args.iter and attempt <= args.iter * 3:
                    attempt += 1
                    # Measure a new set of values
                    run_binary(args.dimm, t, b, p, attempt)
                    sleep(0.01)
                    data_arr = get_measurements(meter)
                    run_binary(args.dimm, "stoploop")
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
                data_dict = append_data(data_dict, test, averages)
                dataframe = pd.DataFrame.from_dict(data_dict)
                dataframe.to_csv(output_csv)

                if len(garbages) > 0 :
                    trashed_dict = append_data(trashed_dict, test, garbages)
                    trashframe = pd.DataFrame.from_dict(trashed_dict)
                    trashframe.to_csv(trash_csv)

    tweet("#"+str(args.dimm)+": Hey @liuwilliam47, I'm done!")

def run_binary(dimm,test,active_banks="0",pattern=0, attempt=0):
    command  = "./bin/idletest"
    command += " " + test
    command += " " + str(active_banks)
    command += " " + str(pattern)

    if test != "stoploop":
        print "["+str_timestamp()+"] " + command + " (" + str(attempt).zfill(2) +") ",

    command += "  > ./log_out/" + "_" + dimm
    command += "_" + test
    command += "_" + str(active_banks)
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

    args = parser.parse_args()

    if (print_flag) :
        print "DIMM Name         : " + args.dimm
        print "# of Iterations   : " + str(args.iter)

    return args

def append_data(data_dict, test, data_arr):
    raw_data = ','.join(map(str, data_arr))
    data_dict["test_n"].append(test["test_n"])
    data_dict["active_banks"].append(test["active_banks"])
    data_dict["pattern"].append(test["pattern"])
    data_dict["temp"].append(test["temp"])
    data_dict["raw_data"].append(raw_data)
    data_dict["avg_voltage"].append(stats.mean(data_arr))
    return data_dict

__main__()
