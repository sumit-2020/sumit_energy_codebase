# Running Combined Toggle Tests
The experiments on the 6 previously tested DIMMs show that bit toggle contributes to the overall power consumption as shown in [this technical report](https://github.com/agyaglikci/fastforward2/blob/master/month16.pdf). So we extend our tests as they will include more DIMMs. Also, we observe the toggle sensitivity of the write operations on power consumption with this set of tests. 

## Which tests to run?
A complete list of the selected DIMMs and tests are located in the following [Google Sheet](https://docs.google.com/spreadsheets/d/1M5tVKpJsatm4sijZtbPQarNHoccMcLN4nNpZRMtPEC4/edit?usp=sharing)

## How to track if a test is done or not?
Currently Y22, B5, and Jerboa are assigned for the toggle tests. All 3 machines tweet in the beginning and at the end of the test. You can see the activity by simply following [@safari_cage](https://twitter.com/safari_cage). A typical tweet looks like the following:
> "#jerboa: #micronb53 starting the tests for #iccad"

## Start a Test
Please refer to the following procedure when starting a new test on any of these machines:

1. Double check the test is completed on the active byobu session. 
2. Select a DIMM among the ones with a green selected column at [Google Sheet](https://docs.google.com/spreadsheets/d/1M5tVKpJsatm4sijZtbPQarNHoccMcLN4nNpZRMtPEC4/edit?usp=sharing)
3. Power off the board.
4. Replace the DIMM with the one you selected. 
5. Hit the ```shift``` button on the meter and read the value. Since the power is off we expect 0, however it is never 0 because of the noise in the environment. Verify that the value is oscilating between positive and negative values. If not, calibrate by rotating the "zero adjust knob" until the oscilation covers some positive and negative values.
6. Turn the board on
7. ssh to the host machine and reboot. You can copy and paste the following into your ```~/.ssh/config``` file to access the machines by using their name.
    
        Host cage
            HostName 128.237.153.90
            User dram
            Compression yes
            ForwardX11 yes
            Ciphers blowfish-cbc,arcfour

        Host jerboa
          HostName 128.237.152.196
          User dram
          Compression yes
          ForwardX11 yes
        
        Host b5
          HostName safari_b5.andrew.cmu.edu
          User dram
          Compression yes
          ForwardX11 yes
> Note that Y22 is not accessible from out of the cage. So you need to access it in two steps: (1) ```ssh cage```, (2) ```ssh y22```.

8. After reboot, ssh to the host machine again: ```ssh jerboa```
9. Open a byobu session: ```byobu```
10. Go to the toggle test directory, and start the test. You can copy the command with proper arguments from the [spreadsheet]((https://docs.google.com/spreadsheets/d/1M5tVKpJsatm4sijZtbPQarNHoccMcLN4nNpZRMtPEC4/edit?usp=sharing)) ("Command to Run" column) **Please be sure that you put the correct host name before copying the command.**

        > cd energy_codebase/toggle_new
        > ./run_iccad.py -h
        usage: run_iccad.py [-h] [--sandbox] [--notests NOTESTS] dimm iter

        positional arguments:
        dimm               DIMM Name
        iter               # of Iterations

        optional arguments:
        -h, --help         show this help message and exit
        --sandbox          Run in Sandbox Mode
        --notests NOTESTS  Provide tests to skip. Ex: sbscrd-dbdcwr skips the 
                           SingleBankSingleColReaD and DoubleBankDoubleColumnWRite 
                           tests
                            
        >  ./run_iccad micronb53 5 --notests sbscrd-sbdcrd

    **IMPORTANT!** B5 has a driver permission issue. We need to run tests on b5 in super user mode. If you provide the host name, spreadsheet puts ```sudo``` in the beginning of the command when necessary.

11. Update the google spreadsheet. So that, everything will be synced. 

  - Update Columns H-O of the previous test and mark them as "done"
  
  - Put the host name of the new test to Column G
  
  - New command should pop up in column AA and the previous one should turn to dark green.
  
  