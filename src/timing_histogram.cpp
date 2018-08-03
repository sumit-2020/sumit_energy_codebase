#include "timing_histogram.h"

int wr_rd_2rows( fpga_t * fpga, uint bank, uint row, uint8_t pattern, CmdQueue * cq,
		const uint trcd, const uint trp, const uint ms_wait_time, string file_name)
{
    int ch = 0; //riffa channel should always be 0
    int recv_try = 0;
    bool recv_failed = false;
    int err_row = 0;
    do {
    	recv_failed = false;
        if (cq == nullptr)
            cq = new CmdQueue();
        else
            cq->size = 0;

        uint8_t pattern_inv = (uint8_t)~pattern;
        uint row_next = row + 1;

//        cout << recv_try << ". try of write and read to/from rows "<< row << "," << row_next << " of bank " << bank << endl;
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
        fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
        #endif
        //high_resolution_clock::time_point t1 = high_resolution_clock::now();

        // Wait x ms -- retention time
        if (ms_wait_time > 1)
            usleep((ms_wait_time-OVERHEAD_MS)*1000);

//        cout << "Write is finished, read is starting" << endl;
        /////////////
        // READ
        /////////////
        cq->size = 0;
        cq->insert(genBusDirCMD(BUSDIR::READ));
        cq->insert(genWaitCMD(5));
        insertFastReadCmds(cq, row, bank, pattern, trcd, trp);
        insertFastReadCmds(cq, row_next, bank, pattern_inv, trcd, trp);

//        detail_print(YELLOW "\rReading Bank: %d Row: %d " RESET, bank, row);
//        fflush(stderr);

        // START Transaction
        assert(cq->size < 1024);
        cq->insert(genStartTR());
        #ifndef SANDBOX
        fpga_send(fpga, ch, (void*)cq->cmds, CMD_SIZE*cq->size, 0, 1, 0);
        #endif
        // high_resolution_clock::time_point t2 = high_resolution_clock::now();
        // auto duration = duration_cast<microseconds>( t2 - t1 ).count();
        // cout << "Duration " << duration << endl; // Around 66us -> round to 1ms

//        cout << "Read commands are sent. Waiting for the response" << endl;
        //////////////////////
        // Check data content
        //////////////////////
        RowErrInfo row_info(-1, row, bank);
        if (checkData(fpga, pattern, row_info) == -1){
            detail_print(RED "Bank: %d Row: %d \n" RESET, bank, row);
            recv_failed = true;
            recv_try ++;
            //
        }

        // for inv pattern
        RowErrInfo row_next_info(-1, row_next, bank);
        if (checkData(fpga, pattern_inv, row_next_info) == -1){
            detail_print(RED "Bank: %d Row: %d \n" RESET, bank, row_next);
            recv_failed = true;
            recv_try ++;
        }

        #ifdef SANDBOX
        cout << "Errors are being generated in SANDBOX mode" << endl;
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
        
        if ( recv_failed ){ //Drain Buffers
        	cout << "Recv failed" << endl;
        	uint rbuf[32];
			int num_recv = 0 ;
			do {
				num_recv = fpga_recv(fpga, 0, (void*)rbuf, 32, 1000);
				usleep(50*1000);
			} while (num_recv);
        }
        else { // Update Histogram

        	err_row += update_row_in_log_file(file_name, bank, row, row_info.err_cl_vec);
        	err_row += update_row_in_log_file(file_name, bank, row_next, row_next_info.err_cl_vec);
			err_row = err_row / 2;
//			cout << "Recv successful. Average num of errors per row is " << err_row << endl;
			return err_row;
        }

    } while(recv_failed && recv_try < RECV_TRY_MAX);
    detail_print(RED "The rows are tried to be accessed for %d times. Exiting now...\n" RESET, RECV_TRY_MAX);
    exit(EXIT_FAILURE);
}

timespec start_timer(){
    timespec cgt_start_point;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cgt_start_point);
    return cgt_start_point;
}

double stop_timer(timespec cgt_start_point, bool print = true){
    timespec cgt_stop_point;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cgt_stop_point);    
    double spent_time   = (cgt_stop_point.tv_sec - cgt_start_point.tv_sec) * 1000
                        + ((double)(cgt_stop_point.tv_nsec - cgt_start_point.tv_nsec)) / 1000000.0;
    if (print)
        cout << spent_time << " ms." << endl;  
    return spent_time;
}

uint find_row_in_log(uint bank_num, uint row_num) {
	uint offset = 0;
	offset += bank_num * TEST_NUM_ROWS * TEST_NUM_CACHELINES;	// Offset of previous banks
	offset += row_num * TEST_NUM_CACHELINES; //Offset of the previous rows of the same bank
	return offset;
}

int update_row_in_log_file(string file_name, uint bank_num, uint row_num, bitset<TEST_NUM_CACHELINES> err_vec){
//	printf("Updating %d:%d\n",bank_num,row_num);
//	fflush(stdout);
//	cout << err_vec << endl;
	int num_of_errors = 0;
	FILE * hist_file;
	hist_file = fopen(file_name.c_str(), "rb+");
	if (hist_file){
		uint offset = find_row_in_log(bank_num,row_num);
		for (uint i = 0 ; i < TEST_NUM_CACHELINES ; i++) {
			int fseek_return = fseek(hist_file, offset+i, SEEK_SET);
//			cout << "fseek_return is " << fseek_return << " for byte: " << (offset + i) << " ";
			if (fseek_return == 0 ){
				char mychar = fgetc(hist_file);
				if (mychar != EOF){
//					cout << "old value: " << (uint8_t) mychar << " new value: " <<  ((uint8_t) mychar + err_vec[i]) << endl;
					fseek(hist_file, offset+i, SEEK_SET);
					fputc((char)((uint8_t)mychar + err_vec[i]) , hist_file);
					num_of_errors += err_vec[i];
				}
				else{
					detail_print(RED "Index of Bank %d / Row %d / Col %d points to the end of file \n" RESET, bank_num, row_num, i);
					exit(EXIT_FAILURE);
				}
			}
			else {
				detail_print(RED "Index of Bank %d / Row %d / Col %d does not exist in the current log file \n" RESET , bank_num, row_num, i);
				exit(EXIT_FAILURE);
			}
		}
		fclose(hist_file);
	}
	else {
		detail_print(RED "%s cannot be opened!\n" RESET , file_name.c_str() );
		exit(EXIT_FAILURE);
	}

	return num_of_errors;
}

int cp(const char *to, const char *from)
{
    int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;
    int saved_errno;

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR)
            {
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0)
    {
        if (close(fd_to) < 0)
        {
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);

        /* Success! */
        return 0;
    }

  out_error:
    saved_errno = errno;

    close(fd_from);
    if (fd_to >= 0)
        close(fd_to);

    errno = saved_errno;
    return -1;
}
