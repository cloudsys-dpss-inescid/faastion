#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

function enable_turbo_boost {
	if [ -f "/sys/devices/system/cpu/intel_pstate/no_turbo" ]; then
		echo "0" | tee /sys/devices/system/cpu/intel_pstate/no_turbo
		echo "Enabled turbo boost."
	else
		echo "Warning: failed to enable turbo boost."
	fi
}

function disable_turbo_boost {
	if [ -f "/sys/devices/system/cpu/intel_pstate/no_turbo" ]; then
		echo "1" | tee /sys/devices/system/cpu/intel_pstate/no_turbo
		echo "Disabled turbo boost."
	else
		echo "Warning: failed to disable turbo boost."
	fi
}


#enable_turbo_boost
#echo on | tee /sys/devices/system/cpu/smt/control # Enable hyperthreading.

disable_turbo_boost
echo off | tee /sys/devices/system/cpu/smt/control # Disable hyperthreading.
