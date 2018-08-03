#!/usr/bin/python

from functions import *

# FPS = 40
FNULL = open(os.devnull, 'w')
STD_DEV_THRESH = 1.5 # 1.5mV -> 30mA


def main():
    args = parse_arguments(True)
    list_num_ops_b2b = [
        0, 1, 2, 3, 4, 5, 6, 10, 14, 24,
        40, 90, 250, 640, 720, 764
    ]
    list_num_ops_fix = [
        0, 10, 50, 100, 150, 200, 250, 300,
        350, 420, 440, 470, 490, 500, 510
    ]
    banks = [0]
    rows = [0x01, 0x10, 0x80]
    patterns = [0xaa]
    tests = ["fixedtime_actnrdpre"] # "actnrdpre"
    dataframe = pd.DataFrame()
    trshframe = pd.DataFrame()
    meter = init_meter_connection()
    start_time = time.time()
    num_of_finished_tests = 0
    num_of_tests = len(patterns) * len(rows) * len(banks)
    num_of_tests *= len(list_num_ops_b2b) + len(list_num_ops_fix)

    print args
    output_csv = "./evalout/"+args.dimm+"_"+str_timestamp()+".csv"
    trash_csv = "./evalout/"+args.dimm+"_"+str_timestamp()+"_trash.csv"
    for bank in banks:
        args.bank = bank
        # tweet("I'm starting the #ACT_764WR_PRE and #ACT_764RD_PRE tests on #bank"+str(bank)+" for #"+str(args.dimm))
        for row in rows :
            args.row = row
            for pattern in patterns:
                for test_name in tests:
                    args.pattern = pattern
                    args.test_name = test_name
                    list_num_ops = list_num_ops_b2b if test_name == "actnrdpre" else list_num_ops_fix
                    for num_ops in list_num_ops :
                        i = 0
                        attempt = 0
                        averages = []
                        garbages = []
                        remove_max = True
                        while len(averages) < args.iter and attempt < 3 * args.iter:
                            run_binary(attempt,args.dimm,args.test_name,args.bank,args.row,args.pattern,num_ops,is_sandbox=args.sandbox)
                            sleep(0.1)
                            data_arr = get_measurements(meter, is_sandbox=args.sandbox)
                            run_binary(attempt,args.dimm,"stoploop",is_sandbox=args.sandbox)

                            attempt = attempt + 1
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

                        data_dict = create_entry(args, i, averages, num_ops)
                        dataframe = dataframe.append(data_dict, ignore_index=True)
                        dataframe.to_csv(output_csv)

                        if (len(garbages) > 0):
                            trsh_dict = create_entry(args, i, garbages, num_ops)
                            trshframe = trshframe.append(trsh_dict, ignore_index=True)
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

    # tweet("Hey! I'm done with #"+str(args.dimm)+"! Please come visit me.")


def create_entry(args, iterindex, data_arr, num_ops):
    raw_data = ':'.join(map(str, data_arr))
    return {
        "test_name": args.test_name,
        "data_patt": args.pattern,
        "iter": iterindex,
        "raw_data": raw_data,
        "avg": stats.mean(data_arr),
        "stdev": stats.pstdev(data_arr),
        "bank": args.bank,
        "row": args.row,
        "avg_curr": stats.mean(data_arr)*20,
        "num_ops": num_ops
    }

def parse_arguments(print_flag = False):
    parser = argparse.ArgumentParser()
    parser.add_argument('dimm', metavar='dimm', type=str, default="SANDBOX",
       help='DIMM Name')
    parser.add_argument('--iter', metavar='iter', type=int, default=10,
       help='# of Iterations')
    parser.add_argument('--sandbox', default=False, action="store_true",
       help='Sandbox flag')
    args = parser.parse_args()
    if args.dimm == "sandbox":
        args.iter = 1
        args.sandbox = True

    args.log = False
    args.nocam = True

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

main()

