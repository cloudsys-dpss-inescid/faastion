#!/bin/bash

set -xe;

timestamp=$(date +%s)
filename="plots_$timestamp.7z"
7z a $filename plots/
