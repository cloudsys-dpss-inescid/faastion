#!/bin/bash

cat main.py | jq -sR  '{value: {name: "python-sleep",  main: "main", binary: false, code: .}}' > init.json
