#!/bin/bash

OVERLAYNAME=$1
OVERLAY=$2

sudo losetup --detach $(cat $OVERLAYNAME.loop)
sudo dmsetup remove $OVERLAYNAME
rm $OVERLAYNAME.loop
rm $OVERLAY
