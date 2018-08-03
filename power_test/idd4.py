#!/usr/bin/python

from functions import *


def main():

    data_dict = {
        "test_name": [], "pattern": [],
        "raw_data": [], "stdev": [],
        "row": [], "ave_curr": []
    }

    trashed_dict = {
        "test_name": [], "pattern": [],
        "raw_data": [], "stdev": [],
        "row": [], "ave_curr": []
    }
    meter = init_meter_connection()
    args = parse_arguments(True)

    rows = [0, 128, 1280]
    output_csv = "./out/" + args.dimm + "_" + str_timestamp() + ".csv"
    trash_csv = "./out/" + args.dimm + "_" + str_timestamp() + "_trash.csv"
    tweet("I'm starting the #IDD4R and #IDD4W tests for #"+str(args.dimm))

    args.num_ops = 764
    for row in rows:
        args.row = row
        for pattern in [0x00, 0x01, 0x11, 0x13, 0x33, 0x37, 0x77, 0x7f, 0xff]:
            args.pattern = pattern
            for test_name in ["idd4w", "idd4r"]:
                args.test_name = test_name
                print args
                data_dict, trashed_dict = collect_settled_values(
                    args, meter,
                    data_dict, trashed_dict,
                    output_csv, trash_csv
                )

    tweet("Hey! I'm done with #"+str(args.dimm)+"! Please come visit me.")


# , idle_arr=[-1,-1,-1], idle_q=0):
def append_data(data_dict, test_name, pattern, data_arr, row):
    raw_data = ','.join(map(str, data_arr))
    data_dict["test_name"].append(test_name)
    data_dict["pattern"].append(pattern)
    data_dict["raw_data"].append(raw_data)
    data_dict["stdev"].append(stats.pstdev(data_arr) * 20)
    data_dict["row"].append(row)
    data_dict["ave_curr"].append(stats.mean(data_arr) * 20)
    return data_dict


def parse_arguments(print_flag=False):
    parser = argparse.ArgumentParser()
    parser.add_argument('dimm', metavar='dimm', type=str, default="SANDBOX",
                        help='DIMM Name')
    parser.add_argument('iter', metavar='iter', type=int, default=10,
                        help='# of Iterations')

    args = parser.parse_args()
    args.log = False

    if (print_flag):
        print "DIMM Name         : " + args.dimm
        print "# of Iterations   : " + str(args.iter)

    return args


def collect_settled_values(args, meter, data_dict, trashed_dict, output_csv, trash_csv):
    STD_DEV_THRESH = 1.5  # 30mA
    dataframe = pd.DataFrame(columns=data_dict.keys())
    trashframe = pd.DataFrame(columns=trashed_dict.keys())
    averages = []
    garbages = []
    attempt = 0
    remove_max = True
    while len(averages) < args.iter and attempt <= args.iter * 3:
        attempt += 1
        # Measure a new set of values
        run_binary(
            attempt, args.dimm,
            args.test_name, 0,
            args.row, args.pattern,
            args.num_ops
        )
        sleep(0.01)
        data_arr = get_measurements(meter)
        run_binary(
            attempt, args.dimm, "stoploop"
        )
        ave_val = stats.mean(data_arr)
        averages.append(ave_val)
        print "Vave: " + str(ave_val) + " ",

        # Calculate the variation
        std_dev = 0
        if len(averages) > 1:
            std_dev = stats.stdev(averages)
            print "stdev: " + str(std_dev) + " ",

        while ((std_dev > STD_DEV_THRESH) and (len(averages) > 1)):
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
    data_dict = append_data(
        data_dict, args.test_name,
        args.pattern, averages,
        args.row
    )
    dataframe = dataframe.from_dict(data_dict)
    dataframe.to_csv(output_csv)

    if len(garbages) > 0:
        trashed_dict = append_data(
            trashed_dict, args.test_name,
            args.pattern, garbages,
            args.row
        )
        trashframe = trashframe.from_dict(trashed_dict)
        trashframe.to_csv(trash_csv)

    return data_dict, trashed_dict


main()
