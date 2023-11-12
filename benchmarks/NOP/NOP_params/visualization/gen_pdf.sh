#!/bin/bash

jupyter nbconvert --TemplateExporter.exclude_input=True --to pdf NOOP_visualization.ipynb
