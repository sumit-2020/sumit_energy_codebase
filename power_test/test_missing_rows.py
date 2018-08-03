#!/usr/bin/python

from functions import *

STD_DEV_THRESH = 1.5

def main():
    data_dict = {
        "test_name": [], "pattern": [],
        "raw_data": [],  "bank": [],
        "row": [], "ave_curr": [], "temp": []
    }

    trashed_dict = {
        "test_name": [], "pattern": [],
        "raw_data": [], "bank": [],
        "row": [], "ave_curr": [], "temp": []
    }

    args = parse_arguments(True)
    print args

    meter = init_meter_connection()

    missing_rows = pd.read_csv("missing_points.csv")
    missing_rows = missing_rows[(missing_rows.dimm == args.dimm) & (missing_rows.temp == args.temp)]

    dataframe = pd.DataFrame(columns=data_dict.keys())
    trashframe = pd.DataFrame(columns=trashed_dict.keys())
    output_csv = "./out/" + args.dimm + "_" + str_timestamp() + ".csv"
    trash_csv = "./out/" + args.dimm + "_" + str_timestamp() + "_trash.csv"
    print "[" + str_timestamp() + "] Starting the experiments. Output: " + output_csv

    tweet(" Row-level structural variation test starts for #" + str(args.dimm))    
    for [test_name, bank, row, pattern], test_df in missing_rows.groupby(["test_name","bank","row","data_pattern"]):
        args.bank = bank
        args.row = row
        args.pattern = pattern
        args.test_name = test_name
        averages = []
        garbages = []
        attempt = 0
        remove_max = True
        while len(averages) < args.iter and attempt <= args.iter * 3:
            run_binary(attempt, args.dimm, test_name, args.bank,
                       args.row, pattern, 765)
            sleep(0.01)
            data_arr = get_measurements(meter)
            run_binary(attempt, args.dimm, "stoploop")
            ave_val = stats.mean(data_arr)
            averages.append(ave_val)
            print "Vave: " + str(ave_val) + " ",

            std_dev = 0
            if len(averages) > 1:
                std_dev = stats.stdev(averages)
                print "stdev: " + str(std_dev) + " ",

            while (std_dev > STD_DEV_THRESH) and (len(averages) > 1):
                if remove_max:
                    max_ave = max(averages)
                    garbages.append(max_ave)
                    averages.remove(max_ave)
                    remove_max = False
                    print "rm_max: " + str(max_ave),
                else:
                    min_ave = min(averages)
                    garbages.append(min_ave)
                    averages.remove(min_ave)
                    remove_max = True
                    print "rm_min: " + str(min_ave),

                if len(averages) > 1:
                    std_dev = stats.stdev(averages)
            print ""
            attempt += 1
        data_dict = append_data(data_dict, test_name, pattern, averages, args.bank, args.row, args.temp)
        dataframe = dataframe.from_dict(data_dict)
        dataframe.to_csv(output_csv)
        if len(garbages) > 0:
            trashed_dict = append_data(trashed_dict, test_name, pattern, garbages, args.bank, args.row, args.temp)
            trashframe = trashframe.from_dict(trashed_dict)
            trashframe.to_csv(trash_csv)

    tweet("Hey! I'm done with #" + str(args.dimm) + "! Please come visit me.")

# , idle_arr=[-1,-1,-1], idle_q=0):
def append_data(data_dict, test_name, pattern, data_arr, bank, row, temp=20):
    raw_data = ','.join(map(str, data_arr))
    # raw_idle = ','.join(map(str, idle_arr))
    data_dict["test_name"].append(test_name)
    data_dict["pattern"].append(pattern)
    data_dict["raw_data"].append(raw_data)
    data_dict["bank"].append(bank)
    data_dict["row"].append(row)
    data_dict["temp"].append(temp)
    data_dict["ave_curr"].append(stats.mean(data_arr) * 20)
    return data_dict

def parse_arguments(print_flag=False):
    parser = argparse.ArgumentParser()
    parser.add_argument('dimm', metavar='dimm', type=str, default="SANDBOX",
                        help='DIMM Name')
    parser.add_argument('iter', metavar='iter', type=int, default=10,
                        help='# of Iterations')
    parser.add_argument('--temp', metavar='temp', type=int, default=20,
            help='Temperature in C (default: 20)')

    args = parser.parse_args()
    args.log = False

    if (print_flag):
        print "DIMM Name         : " + args.dimm
        print "# of Iterations   : " + str(args.iter)
        print "Temperature (C)   : " + str(args.temp)

    return args

def get_sample_rows(dimm, bank):
    sample_rows_df = pd.read_csv("sample_rows.csv")
    sample_rows_df = sample_rows_df[(sample_rows_df.dimm == dimm) & (sample_rows_df.bank == bank)]
    return sample_rows_df.row.tolist()

main()
