# OpticalFlowStats (UNFINISHED demo)


The objective is calculation of dense optical flow of a given video and then its statistics. 
The idea came up from my master thesis which objective was to check some posibilities
of differentiating turbulent flows basing on relatively simple statistics. Example of such flows
are ones obtained from videos from combustion chamber interiors, where combution processes with
different air-fuel ratio can be observed.

Some statistics have been already defined. Once a new statitic is defined 
it's easy to add it to the Collector class object. Collector creates collections automatically
which can be then saved to files.

Statistics to be defined (among others):
velocity variance, turbulence intensity, curl statistics, max velocity, max divergence, max curl,
mean number of divergence/curl sign changes in vertical and horizontal direction,
histogram of angles with magnitude(velocity - taken as the length of a vector),
histogram of velocity.

Configurations for statistics to be created and calculated are provided to Collector in config.json
# TODOs:

Add documentation!
Use streams to enable simultaneous collecting and printing/writing results to files

The rest of TODOs are in todo-tree.txt

# Build options:
  ## -DUSE_GPU
      Use cuda-defined statistics equivalents
  ## -DBUILD_TESTING
      Build unit tests

# Testing
    make test -C /path/to/build/directory
