#!/usr/bin/python

from functions import *

STD_DEV_THRESH = 1.5

def main():
    calibration_period = 16
    calibration_step = 1.0 / calibration_period
    data_dict  = {"bank":[], "row": [], "iter": [], "idd": [], "iddfix": []}
    calib_dict = {"bank":[], "row": [], "iter": [], "idd": [], "iddfix": []}
    args = parse_arguments(True)
    meter = init_meter_connection()

    bank = 0
    test_name = "idd0"
    pattern = 0xff
    temp = 20

    args.bank = bank
    args.test_name = test_name
    args.pattern = pattern
    args.temp = temp


    print calibration_step
    output_csv = "./out/" + args.dimm + "_" + str_timestamp() + "_row_meas_new.csv"
    calib_csv  = "./out/" + args.dimm + "_" + str_timestamp() + "_row_calib_new.csv"
    print "[" + str_timestamp() + "] Starting the experiments. Output: " + output_csv

    # rows = range(0x0000,0x01f8,0x0008)
    # subarrays = {
    #     "A": 0x0000,
    #     "B": 0x1D00,
    #     "C": 0x2500,
    #     "D": 0x3000,
    #     "E": 0x4000,
    #     "F": 0x5D00,
    #     "G": 0x6E00,
    #     "H": 0x7E00
    # }
    row_sets = [
        [
            0x0000, 0x0800, 0x1800, 0x1300, 
            0x2700, 0x2f00, 0x3f00, 0x3760, 
            0x4667, 0x47f1, 0x57f1, 0x55f7, 
            0x67f7, 0x6f7f, 0x7f7f, 0x7fff
        ],
        [
            0x0000, 0x0100, 0x1100, 0x1c00, 
            0x2e00, 0x20f0, 0x30f0, 0x3e60, 
            0x4c37, 0x4ef1, 0x5ef1, 0x5af7, 
            0x6ef7, 0x67ff, 0x77ff, 0x7fff
        ],
    ]
    tweet(" Row-level structural variation test starts on #bank" +
              str(bank) + " for #" + str(args.dimm))

    idd_buffer = []
    for i in range(20):
        run_binary(i, args.dimm, test_name, bank,
                   0, pattern, 765)
        sleep(0.01)
        data_arr = get_measurements(meter)
        run_binary(i, args.dimm, "stoploop")
        print ""
        idd_buffer.append(stats.mean(data_arr))

    idd_r0_init = stats.mean(idd_buffer)
    idd_r0_begin = idd_r0_init
    idd_buffer = []
    row_buffer = []
    
    for _iter in range(5):
        args.iter = _iter
        tweet("#IDD0 row variation test starts to "+str(_iter+1)+". iteration for #" + str(args.dimm))
        for row_set in row_sets:
            for bank in [0,2,4,6,1,3,5,7]:
                for row in row_set:
                    args.row = row
                    args.bank = bank
                    # Measure current consumption and append it to the buffer
                    run_binary(args.iter, args.dimm, test_name, args.bank,
                                   args.row, pattern, 765)

                    sleep(0.01)
                    data_arr = get_measurements(meter)
                    run_binary(args.iter, args.dimm, "stoploop")
                    idd_buffer.append(stats.mean(data_arr))
                    row_buffer.append(args.row)

                    # If the buffer is full, then it is time to get new calibration point
                    if len(idd_buffer) == calibration_period :

                        # Measure row 0 again and get new calibration point
                        print ""
                        run_binary(args.iter, args.dimm, test_name, args.bank,
                                   0, pattern, 765)
                        print ""
                        sleep(0.01)
                        data_arr = get_measurements(meter)
                        run_binary(args.iter, args.dimm, "stoploop")
                        idd_r0_end = stats.mean(data_arr)

                        # Find the delta from very beginning to the beginning of the buffer
                        delta_begin = idd_r0_begin - idd_r0_init

                        # Find relative delta inside the buffer by using the calibration points at both ends
                        delta_step  = calibration_step * (idd_r0_end - idd_r0_begin)
                        delta_buff  = [i * delta_step for i in range(calibration_period)]

                        # Print out the deltas (print out as mA, but all calculations continue in mV)
                        print "Deltas: " + str(delta_begin*20) + " + [ ",
                        for d in delta_buff:
                            print str(d*20) + " ",
                        print"]"

                        # Set row to the beginning of the buffer and append the measurements along with the fixed values
                        for i, idd in enumerate(idd_buffer) :
                            rec_row = row_buffer[i]
                            idd_fix = idd - delta_begin - delta_buff[i]
                            data_dict = append_data(data_dict, args.iter, bank, rec_row, idd, idd_fix)

                        idd_buffer = []
                        row_buffer = []
                        idd_r0_begin = idd_r0_end
                        calib_dict = append_data(calib_dict, args.iter, bank, args.row, idd_r0_end)

                        # Create a dataframe from all the measurements so far and save it as a csv file
                        dataframe = pd.DataFrame.from_dict(data_dict)
                        calibframe = pd.DataFrame.from_dict(calib_dict)

                        dataframe["dimm"] = args.dimm
                        dataframe["test_name"] = test_name
                        dataframe["pattern"] = pattern
                        dataframe["temp"] = temp
                        # dataframe["subarr"] = subarray_name
                        dataframe.to_csv(output_csv)

                        calibframe["dimm"] = args.dimm
                        calibframe["test_name"] = test_name
                        calibframe["pattern"] = pattern
                        calibframe["temp"] = temp
                        # calibframe["subarr"] = subarray_name
                        calibframe.to_csv(calib_csv)

                    # Print the endl for the log of run_binary
                    print ""

    tweet("Hey! I'm done with #" + str(args.dimm) + "! Please come visit me.")

def append_data(data_dict, rec_iter, rec_bank, rec_row, idd, idd_fix = False):
    data_dict["bank"].append(rec_bank)
    data_dict["row"].append(rec_row)
    data_dict["iter"].append(rec_iter)
    data_dict["idd"].append(idd * 20)

    if idd_fix:
        data_dict["iddfix"].append(idd_fix * 20)
        print "Iter: " + str(rec_iter) + " Row: " + str(rec_row) + " IDD: " + str(idd*20) + " IDDfix: " + str(idd_fix*20)
    else:
        data_dict["iddfix"].append(idd * 20)
        print "Measuring row "+str(rec_row)+" for calibration: " + str(idd*20)
    return data_dict

def parse_arguments(print_flag=False):
    parser = argparse.ArgumentParser()
    parser.add_argument('dimm', metavar='dimm', type=str, default="SANDBOX",
                        help='DIMM Name')
    args = parser.parse_args()

    return args

main()
