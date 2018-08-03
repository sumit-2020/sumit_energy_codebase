#!/usr/bin/python

from functions import *

STD_DEV_THRESH = 1.5  # 30mA


def __main__():
    args = parse_arguments()
    print "Starting bus direction test for " + args.dimm + "."
    clean_start()
    data_dict = {
        "test_id": [],
        "test_n": [],
        "direction": [],
        "raw_data": [],
        "ave": [],
        "stdev": []
    }
    trashed_dict = {
        "test_id": [],
        "test_n": [],
        "direction": [],
        "raw_data": [],
        "ave": [],
        "stdev": []
    }

    meter = init_meter_connection()
    dataframe = pd.DataFrame(columns=data_dict.keys())
    trashframe = pd.DataFrame(columns=trashed_dict.keys())
    output_csv = "./out/" + args.dimm + "_" + str_timestamp() + ".csv"
    trash_csv = "./out/" + args.dimm + "_" + str_timestamp() + "_trash.csv"
    print "[" + str_timestamp() + "] Starting the experiments. Output: " + output_csv

    for direction in [0, 1]:
        averages = []
        garbages = []
        attempt = 0
        remove_max = True
        while len(averages) < args.iter and attempt <= args.iter * 3:
            attempt += 1
            # Measure a new set of values
            if direction == 0 :
                pre_test = "actnrdpre"
            else :
                pre_test = "actnwrpre"
            run_binary(attempt, args.dimm, pre_test)
            run_binary(attempt, args.dimm, "stop")
            sleep(0.01)
            data_arr = get_measurements(meter)
            ave_val = stats.mean(data_arr)
            averages.append(ave_val)
            print "Vave: " + str(ave_val) + " ",

            std_dev = 0
            if len(averages) > 1:
                std_dev = stats.stdev(averages)
                print "stdev: " + str(std_dev),

                while (std_dev > STD_DEV_THRESH) and (len(averages) > 1):
                    if remove_max:
                        max_ave = max(averages)
                        garbages.append(max_ave)
                        averages.remove(max_ave)
                        remove_max = False
                        print " rm_max: " + str(max_ave),
                    else:
                        min_ave = min(averages)
                        garbages.append(min_ave)
                        averages.remove(min_ave)
                        remove_max = True
                        print " rm_min: " + str(min_ave),

                    if len(averages) > 1:
                        std_dev = stats.stdev(averages)

                print ""

        data_dict = append_data(
            data_dict,
            attempt,
            "busdir",
            direction,
            averages
        )
        dataframe = dataframe.from_dict(data_dict)
        dataframe.to_csv(output_csv)
        if len(garbages) > 0:
            trashed_dict = append_data(
                trashed_dict,
                attempt,
                "busdir",
                direction,
                garbages
            )
            trashframe = trashframe.from_dict(trashed_dict)
            trashframe.to_csv(trash_csv)


def append_data(data_dict, test_id, test, direction, data_arr):
    """Appends the data to dictionary"""
    raw_data = ','.join(map(str, data_arr))
    data_dict["test_id"].append(test_id)
    data_dict["test_n"].append(test)
    data_dict["direction"].append(direction)
    data_dict["raw_data"].append(raw_data)
    data_dict["ave"].append(stats.mean(data_arr))
    data_dict["stdev"].append(stats.pstdev(data_arr))
    return data_dict

def parse_arguments(print_flag=False):
    """Parse command line arguments"""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        'dimm',
        metavar='dimm',
        type=str,
        default="SANDBOX",
        help='DIMM Name'
    )
    parser.add_argument(
        'iter',
        metavar='iter',
        type=int,
        default=10,
        help='# of Iterations'
    )
    parser.add_argument(
        '--num_ops',
        metavar='num_ops',
        type=int,
        default=1,
        help='Number of Operations'
    )
    args = parser.parse_args()
    args.log = False

    if print_flag:
        print "DIMM Name         : " + args.dimm
        print "# of Iterations   : " + str(args.iter)

    return args

__main__()
