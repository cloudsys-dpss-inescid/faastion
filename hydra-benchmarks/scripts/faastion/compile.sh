#!/bin/bash

base_dir="$ARGO_HOME/benchmarks/src/java"
dir_prefix="gv-native-hw-"

# Iterate over directories
for dir in "${base_dir}/${dir_prefix}"*; do
    if [ -d "$dir" ]; then
        echo "Compiling benchmark: $dir"
        
        cd "$dir" || exit
        
        ./build_script_proc_iso.sh >/dev/null 2>&1

        cd - || exit

        echo "------------------------"
    fi
done

echo "All scripts executed successfully."
