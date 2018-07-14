compass-unpack

A program to unpack raw ROOT or binary files written by CAEN CoMPASS software
reading V1730/V1725 digitizers.  Performs timestamp-based matching of coincidence
events and can either write the resulting events to a plain ROOT file, or to an
NPTOOL ROOT file (if the digitizer is being used w/ the MDM Oxford detector).

Note - NPTOOL is not a necessary prereq; if it's not installed on your system
the nptool parts of the code will be disabled automatically.

To compile do the following:

    mkdir build
    cd build
    cmake ../
    make

To run the program, the only argument is the path of a json config file. See
the json files in this directory for examples.

    ./compass_unpack ../config.json