#!/usr/bin/python

from functions import *
import time
STD_DEV_THRESH = 1.5
columns = ["test_name","test_curr"]

def __main__():
    """Runs IDD tests as defined in datasheets."""
    args = parse_arguments(True)
    idd_tests = ["idd0"]
    data_dict = dict( (el,[]) for el in columns)
    trsh_dict = dict( (el,[]) for el in columns)

    meter = init_meter_connection(args.sandbox)

    output_csv = "./out/"+args.dimm+"_"+str_timestamp()+"_"+str(args.time)+"_minutes.csv"
    trash_csv = "./out/"+args.dimm+"_"+str_timestamp()+"_"+str(args.time)+"_time_var_trash.csv"

    tweet("#"+str(args.dimm)+" is starting #datasheet measurements.")

    print "Measuring the base current consumption... "
    init_base_curr = 0
    for i in range(10):
        run_binary(args.dimm, "test", verbose=True)
        run_binary(args.dimm, "stop")
        base_arr = get_measurements(meter, args.sandbox)
        init_base_curr += stats.mean(base_arr)
    init_base_curr = init_base_curr * 2
    print "["+str_timestamp()+"] IDD_base = " + str(init_base_curr) + " mA"
    print "["+str_timestamp()+"] Starting the experiments. Results will be logged in " + output_csv
    for idd_test in idd_tests:
        print "["+str_timestamp()+"] Starting the " + idd_test + " test for " + args.dimm + "."
        averages = []
        garbages = []
        attempt  = 0
        remove_max = True
        time_start = time.time()
        while(time.time() - time_start < args.time * 60):
            attempt += 1
            # Measure a new set of values
            run_binary(args.dimm, idd_test, attempt)
            sleep(0.01)
            test_arr = get_measurements(meter, args.sandbox)
            run_binary(args.dimm, "stop")
            base_arr = get_measurements(meter, args.sandbox)

            test_curr = stats.mean(test_arr) * 20
            base_curr = stats.mean(base_arr) * 20
            print "IDD_test: " + str(test_curr) + " ",
            print "IDD_base: " + str(base_curr) + " ",
            test_curr += base_curr - init_base_curr
            print "IDD_norm: " + str(test_curr) + " ",

            averages.append(test_curr)


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

            print "Time remaining: " + str(args.time*60 - (time.time() - time_start)) + " seconds"
            print ""
        data_dict = append_data(data_dict, args, idd_test, averages)
        dataframe = pd.DataFrame.from_dict(data_dict)
        dataframe["dimm"] = args.dimm
        dataframe["temp"] = args.temp
        dataframe["time"] = time.time()
        dataframe.to_csv(output_csv)

        if len(garbages) > 0 :
            trsh_dict = append_data(trsh_dict, args, idd_test, garbages)
            trashframe = pd.DataFrame.from_dict(trsh_dict)
            trashframe["dimm"] = args.dimm
            trashframe["temp"] = args.temp
            trashframe["time"] = time.time()
            trashframe.to_csv(trash_csv)

def append_data(my_dict, args, test_name, measurements):
    for m in measurements:
        my_dict["test_name"].append(test_name)
        my_dict["test_curr"].append(m)
    return my_dict

def run_binary(dimm,test_name,attempt=0,verbose=True):
    command  = "./bin/iddtest"
    command += " " + dimm
    command += " " + test_name
    cmd_log = "["+str_timestamp()+"] " + command + " (" + str(attempt).zfill(2) +") "
    log_file= "./log_out/" + dimm + "_" + test_name + ".log"
    command += " > " + log_file

    # Run the binary file
    sp.call(command, shell=True)

    # If there is FPGA send timeout, recognize the connection fault and kill the process.
    pci_try = 0
    while 'FPGA send has timed out!' in open(log_file).read():
        if pci_try >= 10:
            print "\n["+str_timestamp()+"] PCI-e connection error occured!"
            tweet("PCI-e connection error occured. Test is aborted.")
            return False

        sp.call(command, shell=True)
        pci_try += 1

    if test_name != "stop" and verbose == True:
        print cmd_log,

    return True

def parse_arguments(print_flag = False):
    parser = argparse.ArgumentParser()
    parser.add_argument('dimm', metavar='dimm', type=str, default="SANDBOX",
        help='DIMM Name')
    parser.add_argument('iter', metavar='iter', type=int, default=10,
        help='# of Iterations')
    parser.add_argument('time', metavar='time', type=int, default=1,
        help='Time to run for')
    parser.add_argument('--temp', metavar='temp', type=int, default=20,
        help='Temperature in C')

    args = parser.parse_args()

    if (print_flag) :
        print "DIMM Name         : " + args.dimm
        print "# of Iterations   : " + str(args.iter)
        print "Time to run for   : " + str(args.time)
        print "Temperature       : " + str(args.temp)

    args.sandbox = False
    if "sandbox" in args.dimm:
        args.sandbox = True
        print "~Running in sandbox mode~"

    return args

__main__ ()
