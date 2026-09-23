#!/bin/bash

if [ ! -e .venv ];
then
	mkdir .venv
	python3 -m venv .venv
	source .venv/bin/activate
	pip install matplotlib
else
	source .venv/bin/activate
fi
