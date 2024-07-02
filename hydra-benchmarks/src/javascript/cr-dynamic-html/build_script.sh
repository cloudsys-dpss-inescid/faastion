#!/bin/bash

zip -r action.zip main.js package.json node_modules
base64 --wrap=0 action.zip > action.zip.base64
cat action.zip.base64 | jq -sR  '{value: {main: "main", binary: "True", code: .}}' > init.json
