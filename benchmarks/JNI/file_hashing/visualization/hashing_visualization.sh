#!/bin/bash

jupyter nbconvert --TemplateExporter.exclude_input=True --to pdf graphing.ipynb
