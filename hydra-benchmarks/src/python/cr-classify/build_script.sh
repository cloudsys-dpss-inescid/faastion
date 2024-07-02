#!/bin/bash

# To prepare the openwhisk package:
# pip install virtualenv
# virtualenv virtualenv
# (in bash) source virtualenv/bin/activate
# pip install pillow

zip -r action.zip __main__.py virtualenv
base64 --wrap=0 action.zip > action.zip.base64
cat action.zip.base64 | jq -sR  '{value: {main: "main", binary: true, code: .}}' > init.json
