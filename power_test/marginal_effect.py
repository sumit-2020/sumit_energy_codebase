#!/usr/bin/python

from functions import *

def get_test_args():
    test_args = {
        "test_name" : ["fixedtime_actnrdpre","fixedtime_actnwrpre"],
        "row"       : [0],
        "n"         : [0,2,4,8,16],
        "pattern"   : [0x00, 0x33, 0xFF],
        "bank"      : [0,7,2,4]
    }
    num_of_tests = len(test_args["test_name"]) * len(test_args["row"])
    num_of_tests = num_of_tests * len(test_args["n"]) * len(test_args["pattern"])
    num_of_tests = num_of_tests * len(test_args["bank"])

    return test_args, num_of_tests

def __main__():
    STD_DEV_THRESH = 1.5 # 1.5mV = 30mA
    args = parse_arguments(True)
    args.log = False
    test_args, num_of_tests = get_test_args()

    data_dict = {key:[] for key in test_args}
    trsh_dict = {key:[] for key in test_args}

    data_dict["raw_data"] = []
    trsh_dict["raw_data"] = []
    data_dict["avg_curr"] = []
    trsh_dict["avg_curr"] = []

    meter = init_meter_connection()
    dataframe = pd.DataFrame(columns=data_dict.keys())
    trshframe = pd.DataFrame(columns=trsh_dict.keys())
    data_csv  = "./out/"+args.dimm+"_"+str_timestamp()+"_me.csv"
    trsh_csv  = "./out/"+args.dimm+"_"+str_timestamp()+"_me_trash.csv"

    print "["+str_timestamp()+"] Starting the experiments. Output: " + data_csv
    print "["+str_timestamp()+"] There are " + str(num_of_tests) + " tests to run."

    start_time = time.time()
    num_of_finished_tests = 0
    for t in test_args["test_name"] :
        tweet("#"+str(args.dimm)+" is starting #MarginalEffectTest for #"+t)
        for b in test_args["bank"] :
            for r in test_args["row"]:
                for p in test_args["pattern"] :
                    for n in test_args["n"]:
                        test = {"test_name":t, "bank":b, "row":r, "pattern": p, "n":n}
                        averages = []
                        garbages = []
                        attempt  = 0
                        remove_max = True
                        log = "Running "+t+" on "+args.dimm+" for bank "+str(b)
                        log += " row " + str(r) + " pattern {:02X}".format(p)
                        log += ". (N="+str(n)+")"
                        print log

                        while len(averages) < args.iter and attempt <= args.iter * 3:
                            attempt += 1
                            # Measure a new set of values
                            run_binary(attempt, args.dimm, t, bank=b, row=r, pattern=p, num_ops=n)
                            sleep(0.01)
                            data_arr = get_measurements(meter)
                            run_binary(attempt, args.dimm, "stoploop")
                            avg_val = stats.mean(data_arr)
                            averages.append(avg_val)
                            print "Vavg: " + str(avg_val) + " ",

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
                        dataframe = dataframe.from_dict(data_dict)
                        dataframe.to_csv(data_csv)

                        if len(garbages) > 0 :
                            trsh_dict = append_data(trsh_dict, test, garbages)
                            trshframe = trshframe.from_dict(trsh_dict)
                            trshframe.to_csv(trsh_csv)

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

    tweet("#"+str(args.dimm)+" is done!")

def append_data(data_dict, test, data_arr):
    raw_data = ','.join(map(str, data_arr))
    data_dict["test_name"].append(test["test_name"])
    data_dict["bank"].append(test["bank"])
    data_dict["row"].append(test["row"])
    data_dict["pattern"].append(test["pattern"])
    data_dict["n"].append(test["n"])
    data_dict["raw_data"].append(raw_data)
    data_dict["avg_curr"].append(stats.mean(data_arr))
    return data_dict

def run_binary(i, dimm, test_name, bank=0, row=0, pattern=0x00, num_ops=0):
    args = [dimm,test_name,str(pattern),str(bank),str(row),str(num_ops)]
    command  = "./bin/iddtest " + " ".join(args)
    command += "  > ./log_out/" + "_".join(args) + ".log"
    #if test_name != "stoploop":
    #    print "["+str_timestamp()+"] Exp. "+str(i+1)+": ", command
    sp.call(command, shell=True)

def str_timestamp():
    d = datetime.fromtimestamp(time.time())
    return d.strftime("%d.%m.%y_%I:%M%p")

def parse_arguments(print_flag = False):
    parser = argparse.ArgumentParser()
    parser.add_argument('dimm', metavar='dimm', type=str, default="SANDBOX",
        help='DIMM Name')
    parser.add_argument('iter', metavar='iter', type=int, default=10,
        help='# of Iterations')

    args = parser.parse_args()

    if (print_flag) :
        print "DIMM Name         : " + args.dimm
        print "# of Iterations   : " + str(args.iter)

    return args

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

__main__()
