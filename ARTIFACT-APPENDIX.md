# Artifact Appendix 

Paper title: **When Drones Meet Privately: Secure Coordination with 𝑡-PSI**

Requested Badge(s):
  - [X] **Available**
  - [X] **Functional**
  - [X] **Reproduced**

## Description 

This artifact is related to the paper "When Drones Meet Privately: Secure Coordination with 𝑡-PSI"
which was accepted into PETS2026.4

This artifact contains the reference implementation of the proposed algorithm. 
It was used for the storage, bandwidth and computing time benchmarks reported inside the paper.

### Security/Privacy Issues and Ethical Concerns 

This artifact does not contain malicious code, nor pose any security or privacy risk.
It also does not include or process privacy-sensitive data. 
The code is built and run inside a dedicated Docker container with no effect on the host system.

## Basic Requirements 

### Hardware Requirements 

*Minimal Hardware Requirements.* No minimum requirement. 

All code was tested on two machines: 
an MSI GF63 laptop, 
running an Intel(R) Core(TM) i7-10750H CPU @ 2.60GHz 
with 12 logical cores and 16 GB of RAM,
and a Raspberry Pi 400, 
with a Broadcom BCM2711 quad-core Cortex-A72 (ARM v8) 64-bit SoC @ 1.8GHz 
and 4 GB of RAM.

### Software Requirements 

This code was tested on Ubuntu 24.04 as the host OS. 
It requires the following APT packages:
- `build-essential=12.10ubuntu1` 
- `cmake=3.28.3-1build7` 
- `ninja-build=1.11.1-2`


We provide a Docker image based on the `ubuntu:24.04` base image. 
Package dependencies are frozen inside the Dockerfile.
It was tested on the same host, with Docker version 28.4.0.

### Estimated Time and Storage Consumption 

Building the Docker image will require 10 minutes of compute time and 3 GB of disk space.
Running the experiments with the default values will require roughly 48 hours of compute time 
and less than 200MB of runtime memory.

## Environment 

### Accessibility 

The artifact repository is available on GitHub at [https://github.com/zoopr/dt_psi](https://github.com/zoopr/dt_psi).

Clone this repository with 

```
git clone --recursive https://github.com/zoopr/dt_psi
```

to properly initialize the imported submodules. The script `./build_docker.sh` should also be able to correct this if they are missing.

### Set up the environment 

We provide the script `build_docker.sh` to automate the building process for the provided Docker image.

The code can also built on a host with any C++ compiler, cmake and ninja, following the commands in the Dockerfile.

### Testing the Environment

We recommend simply running the first test via the included `run_docker_init.sh` script. 
This will run the initialization test without the need for human interaction, terminating within 2 minutes.

## Artifact Evaluation 

### Main Results and Claims

We provide this artifact as a sample implementation on worst-case runtime performance of our algorithm.
We separate timing tests in 4 main areas: parameter initialization, row encryption, row decryption, and reconstruction.
All test will provide average, variance, and standard deviation, both in absolute values and percentage of average runtime.

The `main()` function `in src/main.cc` handles the precise parametrization of each experiment in a nested for loop.
Default parameter ranges are set via the macros `MIN_*` and `MAX_*` in the same source file. Custom parameter ranges can be set via positional arguments. 

You can run all tests required for our results sequentially by executing

```
./run_all_tests.sh
```

We also provide a set of `run_docker_*.sh` scripts.
These allow you to to produce the data required for each individual figure in the paper.

#### Main Result 1: Parameter initialization

Parameter initialization focuses on the scaling between threshold, number of participants, and computing time.
We will describe the parametrization in [Experiment 1](#experiment-1-init). 
Testing only the parameter initialization should require less than 10 minutes. 
Runtime increases linearly over number of participants, scaling slightly in base value depending on threshold size.
Internally, the code runs the initialization phase 100 times for each parameter combination.
The expected timing results are presented in "Figure 1" of our paper.

#### Main Result 2: Row Encryption and Decryption

Row encryption and decryption focuses on the scaling between threshold, number of coordinates, and computing time.
We will describe the parametrization in [Experiment 2](#experiment-2-encdec). 
There should be no effect of threshold on final computing time.
Time should scale linearly with number of coordinates. 
"Figure 2" in the paper contains the timing results for both tests in its two subfigures.
Row encryption was also tested on the Raspberry Pi 400. Timing results in "Figure 2a" of our paper refer to this separate testing hardware.

#### Main Result 3: Reconstruction

Reconstruction tests demonstrate the scaling between threshold, number of participants, and computing time.
Computing time scales linearly with number of coordinates and exponentially with threshold. 
We recommend a reduced testing set for the highest threshold setting considered. 
We will describe the exact parametrization in [Experiment 3](#experiment-3-reconstruction). 
"Figure 3" in the paper contains the timing results per coordinate.

### Experiments

To build and run all experiments sequentially, run 

```
./run_all_tests.sh
```

We also provide a number of scripts for testing each element separately. 
These scripts all depend on the same docker image built by `build_docker.sh`. 
Each experiment is given one or more scripts automatically passing the correct parameters to the test environment.

For custom parameters, run `run_docker_generic.sh` passing the desired ranges. 

The full parameters list is the following:
```
[DEBUG_REC=1] [PLOT_HELPER=1-4] ./run_docker_generic.sh [min_t max_t min_coords max_coords min_participants max_participants shortcut]
```

- All `min_` and `max_` variables control the minimum and maximum value range tested by the script for each variable parameter (threshold, coordinates, participants).
- `shortcut` is a value from 0 to 2 determining whether the script runs only initialization (`1`), only up to encryption and decryption (`2`), or fully, including reconstruction (`0`).
- `DEBUG_REC` prints a line each time a coordinate is confirmed, if set. May be useful for debugging at very high threshold values.
- `PLOT_HELPER` can be set to only produce the output relevant to each of the 4 graph templates. See [Limitations](#limitations).


Also note that in order to pass the correct environmental variables, `run_docker_generic.sh` runs without sudo, so you will need to be able to run docker without superuser privileges. 

#### Experiment 1: Init

Runtime: 2 compute minutes

You can run Experiment 1 by running

```
./run_docker_init.sh
```

Text output directly on terminal should report average, variance, and standard deviation, including percentage of the average run.
We manually input these results onto a `tikz` image into LaTeX.

#### Experiment 2: Enc/Dec

Runtime: 10 compute minutes

You can run Experiment 2 by running

```
./run_docker_encdec.sh
```

As before, terminal output should report average, variance, and standard deviation, including percentage of the average run.
Note that this will also output zero-length Reconstruction data, which should be ignored.
We manually input these results onto a `tikz` image into LaTeX. 

To replicate the numbers in figure 2a, these tests should be run on the Raspberry Pi 400. 
They should run about 7x slower. Expected runtime is slightly over 1 hour.

#### Experiment 3: Reconstruction

Runtime: 44 compute hours total

We split this experiment in two batches to simplify result extraction for different complexities. 

```
./run_docker_full_lowt.sh
```
Threshold up to 5, <2 hours

```
./run_docker_full_t7.sh
```
Threshold 7, ~ 42 hours

Results are aggregated per coordinate processed. Timing stability in both sets at 127 participants should be below 2%.

This set may run for prolonged time without external feedback. To avoid this, the `DEBUG_REC` environmental variable is set inside `./run_docker_full_t7.sh`, printing each time a coordinate test is completed. Please note that each coordinate may still require several hours.

Terminal output should report average, variance, and standard deviation, including percentage of the average run.
We manually input these results onto a `tikz` image into LaTeX. 


## Limitations 

Generating the graphs still requires manual input into a `tikz` template. 
We provide templates for each figure in the repository in the `figures/` folder.

We offer the `PLOT_HELPER` environmental variable to directly produce the test values in the proper formatting. In other words, this will package the mean and standard deviation of each experiment in a single line, already in the format required by the line graphs, with no additional text output.

This environmental variable can be set to values from 1 to 4, corresponding to `init`, `enc`, `dec`, and `rec` respectively. The text output will directly produce the inner tables to be copy/pasted into the corresponding template. 

Note that this does not automatically adjust the experiment parameters. These should still be passed according to the tests. 
If in doubt, copy and paste the contents of the corresponding script.

For instance, after adding `-e PLOT_HELPER=1` to the contents of `./run_docker_init.sh`:
```
docker run --rm -e PLOT_HELPER=1 dtpsi 2 7 1000 1000 7 128 1
```

will print out:
```
(4,0.000488896) +- (0,7.71649e-05) % 15.7835
(4,0.000479578) +- (0,9.46242e-05) % 19.7307
T=2
(8,0.00449236) +- (0,0.000211564) % 4.70941
(16,0.00500953) +- (0,0.000150343) % 3.00115
(32,0.00627994) +- (0,0.000219344) % 3.49277
(64,0.00895791) +- (0,0.000315433) % 3.52128
(128,0.0140047) +- (0,0.000367383) % 2.62327
T=3
(8,0.00494074) +- (0,0.000328892) % 6.65673
(16,0.00589893) +- (0,0.000177706) % 3.01251
(32,0.00792619) +- (0,0.000301734) % 3.8068
(64,0.0122606) +- (0,0.000502296) % 4.09683
(128,0.0204633) +- (0,0.000528419) % 2.58228
T=4
(8,0.005331) +- (0,0.000174506) % 3.27341
(16,0.00702788) +- (0,0.00057648) % 8.20275
...
```
The first two rows are a byproduct of initialization and should be ignored. Following them is a header indicating the current threshold value, and a sequence of lines, one for each individual experiment at that threshold. 

Paste each per-threshold subset in the correct place in the corresponding LaTeX file. For instance, the lines between `T=2` and `T=3` should be pasted in the following section of `init_tikz.tex`:
```
...

% Threshold 2
\addplot+[error bars/.cd,
        y dir=both,
        y explicit] coordinates {

<<< here >>>

};
\addlegendentry{2}

...
```

Note that the two subfigures in Figure 2 (corresponding to `enc` and `dec`) are scaled to the millisecond within the paper. The numerical values should be manually edited to be 1000 times higher.


## Notes on Reusability 

The code can be included as a library in other projects as-is
or can be further expanded and modified to be integrated into 
practical applications or further experiments.
Individual modules and other elements of the proposed protocol 
can be adapted to process different data types, 
or interface with different hardware devices' network stacks.
