#!/usr/bin/env bash

for g in 1 2 6 24
do
  for f in paper_eval_keys/json/*
  do
    # Remove old binary and build folder
    rm reconstruct && rm -r build
    # Setup correct config
    sed -E -i "s#GAMMA = [0-9]+#GAMMA = ${g}#g" src/Constants.h
    sed -E -i "s#FileName = \".*?\"#FileName = \"${f}\"#g" src/Config.cpp
    # Build and run
    mkdir build
    pushd build
    cmake .. && make clean all
    popd
    { time ./reconstruct ; } > paper_eval_keys/reconstruction_log/$(basename ${f})_gamma_${g}.log
    cp finalCandidates.json paper_eval_keys/reconstruction_log/finalCandidates_$(basename ${f})_gamma_${g}
  done
done

