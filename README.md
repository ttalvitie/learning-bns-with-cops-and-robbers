# Learning Bayesian networks with cops and robbers
The code for the paper "Learning Bayesian networks with cops and robbers" (Topi Talvitie, Pekka Parviainen).

Contents:

- `bnrepository` contains the networks as downloaded from the [bnlearn repository](https://www.bnlearn.com/bnrepository/)

- `bnrepository_nets` contains the network structures and CPDAGs preprocessed using `preprocess_bnrepository.py` (the script requires R and the [bnlearn](https://www.bnlearn.com/) R package)

- `tamaki2017` contains the [Tamaki-2017 treewidth solver](https://github.com/TCS-Meiji/PACE2017-TrackA)

- This directory contains our code. Usage instructions are given below.

The `Makefile` compiles all our code and the Tamaki-2017 treewidth solver. Use it by running `make`. A C++ compiler and a Java installation is required. After compiling, you can use the three resulting executables as follows:

- To test that the algorithm works, run `bayesian_test` with three arguments: minimum and maximum node counts and time limit in seconds per run. For example, to test it random instances with 0..10 nodes and time limit of 1 second per run, run
    ```
    ./bayesian_test 0 10 1
    ```
    The program will run infinitely (unless it finds an error) and print statistics every 10 minutes.

- To measure independence query count distributions of our algorithm and the PC algorithm when using the exact independence oracle, run `bnrepository_test` with two arguments: name of the preprocessed network file and time limit per algorithm in seconds. For example, for the alarm network with time limit of 10 minutes, run
    ```
    ./bnrepository_test bnrepository_nets/alarm.net 600
    ```

- To measure the SHD of the learned network from the correct one using our algorithm and the PC algorithm learned from real data, run `bnrepository_data_test` with two arguments: name of the preprocessed network file and time limit per algorithm in seconds. The program takes the data as input. To generate data from the network, you should use the `gen_data.py` script (requires R and the bnlearn package). For example, to generate 1000 data points from the alarm network and measure the SHDs of the learned networks, run
    ```
    ./gen_data.py bnrepository/alarm.bif.gz 1000 | ./bnrepository_data_test bnrepository_nets/alarm.net 600
    ```

The code has been configured with a maximum of 128 nodes. To increase this, increase WordCount in `bitset.hpp`.
