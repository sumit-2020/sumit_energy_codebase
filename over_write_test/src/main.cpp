/**
 * @brief Testing the activation latency effect on writes
 * @author Kevin Chang <kevincha@cmu.edu>
 **/

#include "test_routine.h"

// Test params
#define TEST_NUM_BANKS 8
//#define TEST_NUM_BANKS 1
#define TEST_NUM_ROWS 32736
//#define TEST_NUM_ROWS 4
//#define TOTAL_TEST_ITS 20
#define TOTAL_TEST_ITS 1
#define START_TRCD 4

// This basically makes the test writing/reading checkerboard patterns
#define ALT_COL_PATT true

// DIMM parameters
#define COL_GRAN 8
#define NUM_COL_ROW 1024

void verifyRead(fpga_t* fpga, const uint row, const uint bank,
        const uint8_t pattern, CmdQueue*& cq, const uint ncol, uint col, string msg)
{
    if (ncol == 0)
        return;
    int error = readAndCompareRow(fpga, row, bank, pattern, cq, ncol, col, ALT_COL_PATT);
    if (error) {
        cout << endl << msg << "-->ERROR at Row : " << row << " col " << col <<
            " count " << error << endl;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cout << "Provide the pattern to test. $ ./bin 0xaa" << endl;
        return 0;
    }

    // Begin Setup of FPGA
    fpga_info_list info;
    if (fpga_list(&info) != 0) {
        printf("Error populating fpga_info_list\n");
        exit(EXIT_FAILURE);
    }
    printf("Number of devices: %d\n", info.num_fpgas);
    for (int i = 0; i < info.num_fpgas; i++) {
        printf("%d: id:%d\n", i, info.id[i]);
        printf("%d: num_chnls:%d\n", i, info.num_chnls[i]);
        printf("%d: name:%s\n", i, info.name[i]);
        printf("%d: vendor id:%04X\n", i, info.vendor_id[i]);
        printf("%d: device id:%04X\n", i, info.device_id[i]);
    }
    fpga_t* fpga = fpga_open(0);
    if (!fpga){
        printf("Problem on opening the fpga \n");
        exit(EXIT_FAILURE);
    }
    printf("The FPGA has been opened successfully! \n");
    fpga_reset(fpga); //keep this
    //////

    CmdQueue* cq = nullptr;
    ofstream error_file;
    error_file.open("over_write_test_"+ fmtTime());

    // Worst-case pattern
    const uint8_t patt_base = (uint8_t)strtol(argv[1], NULL, 0);
    uint8_t patt_inv = (uint8_t)~patt_base;
    cout << GREEN "Patt & PattInv=" << hex << (unsigned)patt_base <<
        " " << (unsigned)patt_inv << dec << RESET << endl;
    cout << GREEN "Checkerboard? " << ALT_COL_PATT << RESET << endl;
    int error = 0;

    for (int test_it = 0; test_it < TOTAL_TEST_ITS; test_it++)
    {
        // This test sequence performs a series of writes and reads over rows
        printf(BLUE "######## Stream write tests it:%d ########\n" RESET, test_it);
        for (uint rcd = START_TRCD; rcd >= 1; rcd--) {
            cout << endl << "Test -- RCD" << rcd << endl;
            for (uint bank = 0; bank < TEST_NUM_BANKS; bank++) {
                for (uint r = 0; r < TEST_NUM_ROWS; r++) {
                    uint8_t patt = (r & (uint8_t)0x1) ? patt_base : patt_inv;
                    turnBus(fpga, BUSDIR::WRITE, cq) ;
                    writeRowFlexRCD(fpga, r, bank, patt, cq, NUM_COL_ROW, rcd,
                            0, ALT_COL_PATT);
                    turnBus(fpga, BUSDIR::READ, cq) ;
                    int error = 0 ;
                    error = readAndCompareRow(fpga, r, bank, patt, cq,
                            NUM_COL_ROW, 0, ALT_COL_PATT);
                    if (error) {
                        cout << endl << "RCD=" << rcd << "-->ERROR at Row : " <<
                            r << " count " << error << endl;
                        error_file << fmtTime() << "ROW " << rcd << " " << bank <<
                            " " << r << " " << error << endl;
                    }
                    if (r % 1000 == 0) {
                        cout << "\rBank " << bank << " | Row " << r;
                        fflush(stdout);
                    }
                }
                usleep(1000); // wait a bit
            }
        }

        // Some filler iterations until we check the time again
        printf(BLUE "\n\n######## Write column-wise tests it:%d ########\n" RESET, test_it);
        for (uint rcd = START_TRCD; rcd >= 1; rcd--) {
            cout << endl << "Test -- RCD" << rcd << endl;
            for (int bank = 0; bank < TEST_NUM_BANKS; bank++) {
                for (uint r = 0; r < TEST_NUM_ROWS; r++) {
                    uint8_t patt = (r & (uint8_t)0x1) ? patt_base : patt_inv;

                    // Fill the row with data using the default trcd
                    turnBus(fpga, BUSDIR::WRITE, cq);
                    writeRowFlexRCD(fpga, r, bank, patt, cq, NUM_COL_ROW,
                            DEF_TRCD, 0, ALT_COL_PATT);
                    turnBus(fpga, BUSDIR::READ, cq) ;
                    error = readAndCompareRow(fpga, r, bank, patt, cq,
                            NUM_COL_ROW, 0, ALT_COL_PATT);
                    if (error)
                        cout << endl << "Default rcd write ERROR " <<
                            error << " at Row : " << r << endl;

                    // Now write one col at a time to test if short trcd
                    // affects other columns that are not accessed --
                    // does it cause erroneous latching?

                    // This will over-write the first half and keeps giving error
                    // for (uint col = 0; col < NUM_COL_ROW; col+=COL_GRAN)
                    // unless we re-write that column again
                    // So let's PICK a half point.
                    uint col = NUM_COL_ROW / 2;
                    {
                        turnBus(fpga, BUSDIR::WRITE, cq);
                        writeRowFlexRCD(fpga, r, bank, (uint8_t)~patt, cq,
                                COL_GRAN, rcd, col, ALT_COL_PATT);
                        turnBus(fpga, BUSDIR::READ, cq);
                        error = readAndCompareRow(fpga, r, bank, (uint8_t)~patt,
                                cq, COL_GRAN, col, ALT_COL_PATT);
                        if (error) {
                            cout << endl << "Compare rows RCD=" << rcd <<
                                "-->ERROR at Row : " << r <<
                                " col " << col <<
                                " count " << error << endl;
                            error_file << fmtTime() << "COL " << bank <<
                                " " << r << " " << col <<
                                " " << error << endl;
                        }

                        // Read other non-accessed ones with a different pattern
                        // First half
                        if (col > 0)
                            verifyRead(fpga, r, bank, patt, cq, col, 0, "First half read cl");
                        // Second half
                        if (col < (NUM_COL_ROW-8))
                            verifyRead(fpga, r, bank, patt, cq,
                                    NUM_COL_ROW-col-COL_GRAN, col+COL_GRAN,
                                    "Second half read cl");
                    }

                    if (r % 1000 == 0) {
                        cout << "\rBank " << bank << " | Row " << r;
                        fflush(stdout);
                    }
                }
            }
        }
        cout << endl;
    }
    cout << endl;

    // DONE
    usleep(500 * 1000); // takes microseconds
    fpga_close(fpga);
}
