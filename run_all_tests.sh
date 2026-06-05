#!/bin/bash
echo "Building Docker image...";
./build_docker.sh;
echo "Running Test 1 (Init)...";
./run_docker_init.sh;
echo "Running Test 2 (Enc/Dec)";
./run_docker_encdec.sh;
echo "Running Test 3 (Reconstruction) - up to t=5";
./run_docker_full_lowt.sh;
echo "Running Test 3 (Reconstruction) - t=7";
./run_docker_full_t7.sh;
echo "All done.";