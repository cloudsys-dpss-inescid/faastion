#!/bin/bash

cat main.js | jq -sR  '{value: {main: "main", code: .}}' > init.json
