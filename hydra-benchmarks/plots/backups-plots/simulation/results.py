#!/usr/bin/python


def read_column(file, col):
    return [int(x.split(' ')[col]) for x in open(file).readlines()]
