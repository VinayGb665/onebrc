# Part 2 : 6/1/24
- If i open the files seek and keep it ready there's an improvement
- A basic pre-processor which finds the right offsets to split the file reads optimizes the shit out it !!
    - All I had to do was to iterate until I find the next \n and lulz.
- Within the thread
    - Using getline is the absolute worst, since it dynamically allocates memory and just ugh
        - ~FOREVER to process 1B rows
    - Reading char by char with getc and checking for \n to count the lines is one way of doing it but slow
        - ~75s to process 1B rows
    - fscanf is an absolute god
        - Just dont use the regex to parse your string into k,v pair
            - Compared to the previous implementation, i think this IS the MAJOR breakthrough
            - Here's the results with 256 threads and fscanf(fp, "%[^;];%lf", city, temp)
                - vinay@VINAYS-MACBOOK-AIR C % make all
                  cc -O0 main.c -o onebrc.o
                  time ./onebrc.o
                  Mate fuk is up
                  Maga So i created 255 tasks with size 8835572454
                  SMD I read 1000000001 lines 
                        239.58 real       790.04 user      1075.71 sys
            - This is just stupid and not worth it
        - ~17s to process 1B rows
        - Increasing the number of threads really didnt do much of a difference b/w 128 vs 256
            - I have 8 cores to it kinda makes sense
            - I wonder if there's a limit past which the context switching would just harm the performance
            - 128 
                - vinay@VINAYS-MACBOOK-AIR C % make all
                  cc -O0 main.c -o onebrc.o
                  time ./onebrc.o
                  Mate fuk is up
                  Maga So i created 127 tasks with size 8835572454
                  SMD I read 1000000001 lines 
                         17.01 real        97.73 user        20.96 sys
            - 256
                - vinay@VINAYS-MACBOOK-AIR C % make all
                  cc -O0 main.c -o onebrc.o
                  time ./onebrc.o
                  Mate fuk is up
                  Maga So i created 255 tasks with size 8835572454
                  SMD I read 1000000001 lines 
                         15.05 real        88.43 user        20.65 sys

        - Till this point we were just reading the line as a string, now we gotta parse it into our format 
            - format is <key:string>;<value:double>
            - Using strtok to split it into 2 strings ( without atof / atod ) is hitting a bit hard ~30s?
                - cc -O0 main.c -o onebrc.o
                  time ./onebrc.o
                  Mate fuk is up
                  Maga So i created 127 tasks with size 8835572454
                  SMD I read 1000000000 lines 
                         29.60 real       185.60 user        21.78 sys
                  make: *** [run] Error 1
            - ^ + atof conversion just makes this rat shit clocking ~180s
                - vinay@VINAYS-MACBOOK-AIR C % time ./onebrc.o
                  Mate fuk is up
                  Maga So i created 127 tasks with size 8835572454
                  SMD I read 1000000000 lines 
                  ./onebrc.o  185.05s user 21.59s system 711% cpu 29.027 total

# Part 3 :  7/1/24
- atof is not too bad when I compile it with -O3 optimizations
    - Down to almost ~32s

- Gonna use stb_ds.h to setup hashmap <char *, double>
    - stb is an overkill i feel for our tiny requirement
- We need to parse the strings on our own, and our dummy data format just has one char as the city name so should be fine
- The result structures need to be properly allocated otherwise we cannot compute it
- **** READ THE REQUIREMENTS PROPERLY ****
    - There was no need to parse a full precision double which we were trying to do till now
    - The requirements clearly say that the temperature is with 1 decimal part.
    - atof/atoi/fscanf with %f all of them are overkills for this and just not worth it
- Parsing the line into city and temperature on our own is the best bet
    - atof for the temperature is still bad, we parse the integer and decimal part separately and get a float
- Execution time down to ~20s , les fukin gooo
    - Total lines counted : 1000000000
       17.30 real       103.62 user        19.96 sys

# Part 4: 10/1/24
- Opening files and reading them in threads is a bit costly, lets use mmap
- mmap gives significant boost for reading but unfortunately this also means we have to do repeated memcpy for our lines to parse
- strncpy is worse than memcpy
- Some good learning about mmap
    - No need to spin up a separate copy of mmap in each thread, it leads to mmap error for some reason
    - One mmap pointer shared across all the threads
    - Since mmap is opened with PROT_READ , really cannot modify the stream
        - For example, if I want to parse the temperature with atoi/atof which expect a \0 terminated string I cannot do
            f[x] = '\0' -> Leads to bus error which is valid
- The CPU usage goes down drastically, without mmap, all my cores were running at 100% utilization, now it hardly shoots up to 50-60%
- To work around the null termination, we memcpy the n bytes into a char array , this is a bottleneck
- The execution times are the best till now around ~13.5s
    - Total lines counted : 1000000000
      ./onebrc.o  41.63s user 13.59s system 423% cpu 13.031 total

