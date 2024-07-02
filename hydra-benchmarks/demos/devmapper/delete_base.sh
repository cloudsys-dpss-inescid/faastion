#!/bin/bash

BASENAME=$1

sudo losetup --detach $(cat $BASENAME.loop)
sudo dmsetup remove $BASENAME
rm $BASENAME.loop
