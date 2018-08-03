#include "timing_histogram_mem.h"
#include "timing_histogram.h"

// Collect a histogram of error occurance in each burst for every row in the dimm
vector<uint64_t> zero_err(8,0);
vector<uint64_t> one_err(8,0);
vector<uint64_t> two_err(8,0);
vector<uint64_t> three_err(8,0);
vector<uint64_t> more_err(8,0);
// Total number of error bits in each beat
vector<uint64_t> total_beat_err_bit(8,0);
// Total number of tested beats
vector<uint64_t> total_beat_cnt(8,0);

void TallyBurstError(unsigned beat_errors[])
{
    // 8 beats per burst
    for (int i = 0; i < 8; i++)
    {
        if (beat_errors[i] == 0)
            zero_err[i]++;
        else if (beat_errors[i] == 1)
            one_err[i]++;
        else if (beat_errors[i] == 2)
            two_err[i]++;
        else if (beat_errors[i] == 3)
            three_err[i]++;
        else
            more_err[i]++;
        total_beat_err_bit[i] += beat_errors[i];
        total_beat_cnt[i]++;
    }
}

/**
 * @brief Check the correctness of a row data and record the error info
 * in that row.
 **/
int TallyCheckData(fpga_t* fpga, uint8_t pattern, RowErrInfo& row_info)
{
    assert((NUM_COLS/COLS_PER_CACHELINE) <= 128);
    bitset<128> err_cl_vec;
    int bit_err_cnt = 0;
    int err_cnt = 0;
    unsigned beat_errors[8] = {0,0,0,0,0,0,0,0};

    // Compare each cache line's result
    for (int i = 0; i < NUM_COLS; i+=COLS_PER_CACHELINE)
    {
        err_cnt = recv_compare(fpga, 0, pattern, beat_errors) ;
        debug_print("Beat 0 error: %d | col %d\n", beat_errors[0], i);
        TallyBurstError(beat_errors);
        if (err_cnt == -1) {
            detail_print("recv error: didn't receive enough data!\n");
            return -1;
        }
        else if (err_cnt > 0)
            err_cl_vec.set(i/COLS_PER_CACHELINE);
        bit_err_cnt += err_cnt;
    }

    // Fill in the row info
    row_info.err_cl_vec = err_cl_vec;
    // TODO: This line causes severe memory leak!
    /*row_info.cacheline_err_loc = err_cl_vec.to_string<char,
        std::string::traits_type,std::string::allocator_type>();*/
    row_info.total_err_bit = bit_err_cnt;
    row_info.beat0_err = beat_errors[0];
    row_info.beat1_err = beat_errors[1];
    row_info.beat2_err = beat_errors[2];
    row_info.beat3_err = beat_errors[3];
    row_info.beat4_err = beat_errors[4];
    row_info.beat5_err = beat_errors[5];
    row_info.beat6_err = beat_errors[6];
    row_info.beat7_err = beat_errors[7];
    return 0;
}

std::pair<row_hist,row_hist> wr_rd_2rows(
    fpga_t * fpga, std::pair<row_hist,row_hist> row_hists, uint row, uint bank, uint8_t pattern,
    CmdQueue * cq, const uint trcd, const uint trp, const uint ms_wait_time)
{
    int ch = 0; //riffa channel should always be 0
    int recv_try = 0;
    bool recv_failed = false;
    int send_words = 0;
    do {
        recv_failed = false;
        if (cq == nullptr) {
            detail_print("NULL CMD queue allocation!\n");
            cq = new CmdQueue();
        }
        else
            cq->size = 0;

        uint8_t pattern_inv = (uint8_t)~pattern;
        uint row_next = row + 1;

        /////////////
        // WRITE
        /////////////
        cq->insert(genBusDirCMD(BUSDIR::WRITE));
        cq->insert(genWaitCMD(5));
        insertWriteRowCmds(cq, row, bank, pattern);
        insertWriteRowCmds(cq, row_next, bank, pattern_inv);

        // Insert now other wise the queue will overflow
        cq->insert(genStartTR());
        #ifndef SANDBOX
        send_words = fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 1000);
        if (send_words <= 0) {
            detail_print("send writes timeout %d!\n", send_words);
            recv_failed = true;
        }
        #endif

        // Wait x ms -- retention time
        if (ms_wait_time > 1)
            usleep((ms_wait_time-OVERHEAD_MS)*1000);

        /////////////
        // READ
        /////////////
        cq->size = 0;
        cq->insert(genBusDirCMD(BUSDIR::READ));
        cq->insert(genWaitCMD(5));
        insertFastReadCmds(cq, row, bank, pattern, trcd, trp);
        insertFastReadCmds(cq, row_next, bank, pattern_inv, trcd, trp);

        // START Transaction
        assert(cq->size < 1024);
        cq->insert(genStartTR());
        #ifndef SANDBOX
        send_words = fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 1000);
        if (send_words <= 0) {
            detail_print("send reads timeout %d!\n", send_words);
            recv_failed = true;
        }
        #endif

        //////////////////////
        // Check data content
        //////////////////////
        RowErrInfo row_info(-1, row, bank);
        if (TallyCheckData(fpga, pattern, row_info) == -1 && !recv_failed) {
            detail_print(RED "Bank: %d Row: %d \n" RESET, bank, row);
            recv_failed = true;
            recv_try ++;
        }

        // for inv pattern
        RowErrInfo row_next_info(-1, row_next, bank);
        if (TallyCheckData(fpga, pattern_inv, row_next_info) == -1 &&
            !recv_failed) {
            detail_print(RED "Bank: %d Row: %d \n" RESET, bank, row_next);
            recv_failed = true;
            recv_try ++;
        }

        ///////////////////////////////////////
        #ifdef SANDBOX
        detail_print("Errors are being generated in SANDBOX mode\n");
        // Generate random errors if the FPGA is not connected until the desired latency values.
        uint desired_trcd = 4;
        uint desired_trp = 7;
        if (trcd < desired_trcd) {
            row_info.err_cl_vec[rand() % TEST_NUM_CACHELINES] = 1;
            row_next_info.err_cl_vec[rand() % TEST_NUM_CACHELINES] = 1;
        }
        if (trp < desired_trp) {
            for (int i = 0 ; i < TEST_NUM_CACHELINES ; i += rand()%2){
                row_info.err_cl_vec[i] = 1;
                row_next_info.err_cl_vec[i+2] = 1;
            }
        }
        #endif
        ///////////////////////////////////////

        //Drain Buffers on a recv failure
        if (recv_failed) {
            uint rbuf[32];
            int num_recv = 0;
            detail_print("drain buffers!\n");
            do {
                num_recv = fpga_recv(fpga, 0, (void *)rbuf, 32, 1000);
                usleep(50 * 1000);
            } while (num_recv);
            detail_print("drain buffers done!\n");
        }
        else {
            // Update histogram
            row_hists.first  = update_hists(bank, row, row_hists.first , row_info.err_cl_vec);
            row_hists.second = update_hists(bank, row_next, row_hists.second , row_next_info.err_cl_vec);
            return row_hists;
        }
    } while(recv_failed && recv_try < RECV_TRY_MAX);
    detail_print(RED "The rows are tried to be accessed for %d times. Exiting now...\n" RESET, RECV_TRY_MAX);
    exit(EXIT_FAILURE);
}

row_hist update_hists(uint bank, uint row, row_hist row_h , bitset<TEST_NUM_CACHELINES> err_vec){
    uint i = 0;
    uint num_of_faulty_cachelines = 0;
    // cout << "Err Vector: ";
    for (row_hist::iterator cl_iter = row_h.begin() ; cl_iter != row_h.end() ; cl_iter++ ){
    	// cout << err_vec[i] << " ";
        num_of_faulty_cachelines += err_vec[i];
        (*cl_iter) += err_vec[i];
        i++;
    }
    // cout << endl;

    // New approach says that if there is errors on more than 2 cachelines
    // tRP is too low. If the errors occur in up to 2 cachelines, the tRCD
    // value should be increased
    if (num_of_faulty_cachelines > 2)
        err_cntrs[2]++;
    else if (num_of_faulty_cachelines > 0)
        err_cntrs[0]++;

    return row_h;
}

dimm_hist initialize_histogram(string file_name){
    dimm_hist hist;
    timespec start = start_timer();
    string log;
    fstream hist_file(file_name, fstream::in);
    if (hist_file.is_open()){
        char c;
        for (uint bank = 0; bank < TEST_NUM_BANKS ; bank ++){
            bank_hist temp_bank;
            for (uint row = 0 ; row < TEST_NUM_ROWS ; row ++){
                row_hist temp_row (TEST_NUM_CACHELINES);
                for (uint cl = 0 ; cl < TEST_NUM_CACHELINES ; cl++ ){
                    hist_file.get(c);
                    temp_row[cl]=c;
                }
                temp_bank.push_back(temp_row);
            }
            hist.push_back(temp_bank);
        }
        detail_print("Histogram array has been initialized with old file in ");
    }
    else {  //TODO: Use fill instead of this loop
        for (uint bank = 0; bank < TEST_NUM_BANKS ; bank ++){
            bank_hist temp_bank;
            for (uint row = 0 ; row < TEST_NUM_ROWS ; row ++){
                row_hist temp_row (TEST_NUM_CACHELINES); //Fills with zero
                temp_bank.push_back(temp_row);
            }
            hist.push_back(temp_bank);
        }
        detail_print("Histogram array has been initialized with zeros in ");
    }
    stop_timer(start, true);
    return hist;
}

void dump_to_file(dimm_hist hist, string file_name) {
    ofstream hist_file (file_name);
    if (hist_file.is_open()) {
        for ( dimm_hist::iterator bank_iter = hist.begin() ; bank_iter != hist.end() ; bank_iter++ ){
            for ( bank_hist::iterator row_iter = (*bank_iter).begin() ; row_iter != (*bank_iter).end() ; row_iter++ ){
                for (row_hist::iterator cl_iter = (*row_iter).begin() ; cl_iter != (*row_iter).end() ; cl_iter++ ){
                    hist_file << *cl_iter;
                }
            }
        }
        detail_print(GREEN "Array has been stored\n" RESET);
    }
    else {
        detail_print(RED "The output file cannot be opened\n\n" RESET);
    }
}

void dump_beat_histo(ofstream& f)
{
    f << "Beat,zero,one,two,three,abovethree,totalbeaterror,totalbeatcount" << endl;
    for (int i = 0; i < 8; i++)
    {
        f << i << "," <<  zero_err[i] << "," << one_err[i] <<
            "," << two_err[i] << "," << three_err[i] <<
            "," << more_err[i] << "," << total_beat_err_bit[i] <<
            "," << total_beat_cnt[i] << endl;
    }
}

void reset_cntrs(){
    memset(err_cntrs, 0, sizeof(err_cntrs));
}

uint * read_cntrs(){
    return err_cntrs;
}

std::pair<row_hist,row_hist> wr_rd_2rows_fix(
    fpga_t * fpga, std::pair<row_hist,row_hist> row_hists, uint row, uint bank, uint8_t pattern,
    CmdQueue * cq, const uint trcd, const uint trp, const uint ms_wait_time)
{
    int ch = 0; //riffa channel should always be 0
    int recv_try = 0;
    bool recv_failed = false;
    bool send_failed = false;
    int send_words = 0;
    do {
        recv_failed = false;
        send_failed = false;
        if (cq == nullptr) {
            detail_print("NULL CMD queue allocation!\n");
            cq = new CmdQueue();
        }
        else
            cq->size = 0;

        uint8_t pattern_inv = (uint8_t)~pattern;
        uint row_next = row + 1;

        /////////////
        // WRITE
        /////////////
        cq->insert(genBusDirCMD(BUSDIR::WRITE));
        cq->insert(genWaitCMD(5));
        insertWriteRowCmds(cq, row, bank, pattern);
        insertWriteRowCmds(cq, row_next, bank, pattern_inv);

        // Wait x ms -- retention time
        if (ms_wait_time > 1)
            usleep((ms_wait_time-OVERHEAD_MS)*1000);

        /////////////
        // READ
        /////////////
        cq->insert(genBusDirCMD(BUSDIR::READ));
        cq->insert(genWaitCMD(5));

        // ACT - PRE pair to test low tRP effect on the first row
        cq->insert(genRowCMD(row_next, bank, MC_CMD::ACT)); //Activate target row
        cq->insert(genWaitCMD(15)); // TRAS
        cq->insert(genRowCMD(PRE_BANK, bank, MC_CMD::PRE)); //Precharge target bank
        if (trp > 1)
            cq->insert(genWaitCMD(trp-1)); // Wait for user-defined tRP

        // Read 2 different rows
        insertFastReadCmds(cq, row, bank, pattern, trcd, trp);
        insertFastReadCmds(cq, row_next, bank, pattern_inv, trcd, trp);

        // START Transaction
        assert(cq->size < 2048);
        cq->insert(genStartTR());
        #ifndef SANDBOX
        send_words = fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 10);
        if (send_words != CMD_SIZE*cq->size) {
            detail_print("Not enough commands sent %d out of %d!\n", send_words, CMD_SIZE*cq->size);
            send_failed = true;
        }
        #endif

        //////////////////////
        // Check data content
        //////////////////////
        RowErrInfo row_info(-1, row, bank);
        RowErrInfo row_next_info(-1, row_next, bank);
        if (!send_failed) {
            // Row 1 with Pattern
            if (TallyCheckData(fpga, pattern, row_info) == -1) {
                detail_print(RED "Bank: %d Row: %d \n" RESET, bank, row);
                recv_failed = true;
            }
            // Row 2 with inv pattern
            if (TallyCheckData(fpga, pattern_inv, row_next_info) == -1) {
                detail_print(RED "Bank: %d Row: %d \n" RESET, bank, row_next);
                recv_failed = true;
            }
        }

        ///////////////////////////////////////
        #ifdef SANDBOX
        detail_print("Errors are being generated in SANDBOX mode\n");
        // Generate random errors if the FPGA is not connected until the desired latency values.
        uint desired_trcd = 4;
        uint desired_trp = 7;
        if (trcd < desired_trcd) {
            row_info.err_cl_vec[rand() % TEST_NUM_CACHELINES] = 1;
            row_next_info.err_cl_vec[rand() % TEST_NUM_CACHELINES] = 1;
        }
        if (trp < desired_trp) {
            for (int i = 0 ; i < TEST_NUM_CACHELINES ; i += rand()%2){
                row_info.err_cl_vec[i] = 1;
                row_next_info.err_cl_vec[i+2] = 1;
            }
        }
        #endif
        ///////////////////////////////////////

        //Drain Buffers on a recv failure
        if (recv_failed || send_failed) {
            uint rbuf[32];
            int num_recv = 0;
            detail_print("drain buffers!\n");
            do {
                num_recv = fpga_recv(fpga, 0, (void *)rbuf, 32, 1000);
                usleep(50 * 1000);
            } while (num_recv);
            detail_print("drain buffers done!\n");
            recv_try++;
        }
        else {
            // Update histogram
            row_hists.first  = update_hists(bank, row, row_hists.first , row_info.err_cl_vec);
            row_hists.second = update_hists(bank, row_next, row_hists.second , row_next_info.err_cl_vec);
            return row_hists;
        }
    } while((recv_failed || send_failed) && recv_try < RECV_TRY_MAX);
    detail_print(RED "The rows are tried to be accessed for %d times. Exiting now...\n" RESET, RECV_TRY_MAX);
    exit(EXIT_FAILURE);
}

struct modified_line
{
    string value;
    operator string() const { return value; }
};

string log_key;
bool found_log_key = false;
int log_value;

istream& operator>>(istream& a_in, modified_line& a_line)
{
    string local_line;
    if (getline(a_in, local_line))
    {
        if (local_line.find(log_key) == 0)
        {
            int val_pos = local_line.find(",") + 1;
            int val = atoi(local_line.substr(val_pos).c_str());
            found_log_key = true;
            val += log_value;
            local_line.replace(val_pos, -1, to_string(val));
        }
        a_line.value = local_line;
    }
    return a_in;
}

int log_iteration_count(string log_file_name, string key, int value)
{
    ifstream in(log_file_name);
    vector<string> modified_lines;
    if (in.is_open())
    {
        log_value = value;
        log_key = key;

        // Load into a vector, modifying as they are read.
        copy(istream_iterator<modified_line>(in),
             istream_iterator<modified_line>(),
             back_inserter(modified_lines));
        in.close();
    }

    // Insert a new log key and value
    if (!found_log_key)
    {
        string new_key_val = key + "," + to_string(value);
        modified_lines.push_back(new_key_val);
    }

    // Overwrite.
    ofstream out(log_file_name);
    if (out.is_open())
    {
        copy(modified_lines.begin(), modified_lines.end(),
                    ostream_iterator<string>(out, "\n"));
    }
    return 0;
}
