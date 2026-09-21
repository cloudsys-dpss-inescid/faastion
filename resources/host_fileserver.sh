#!/bin/bash

if [ ! -e .venv ];
then
	mkdir .venv
	python3 -m venv .venv
	source .venv/bin/activate
	pip install flask
else
	source .venv/bin/activate
fi

mkdir -p /tmp/uploads
python3 fileserver.py
