#!/bin/bash

if [ ! -f tensorflow_inception_graph.pb ];
then
    wget https://github.com/martinwicke/tensorflow-tutorial/raw/master/tensorflow_inception_graph.pb
fi
if [ ! -f ffmpeg ];
then
    wget https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz
    tar -xf ffmpeg-release-amd64-static.tar.xz
    mv ffmpeg-*-amd64-static/ffmpeg .
    rm -r ffmpeg-*-amd64-static
    rm ffmpeg-release-amd64-static.tar.xz
fi

python3 -m http.server 8000