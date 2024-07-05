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
if [ ! -f snap.png ];
then
    wget https://www.freepnglogos.com/uploads/linux-png/linux-logo-logo-brands-for-0.png -O snap.png
fi
if [ ! -f imagenet_comp_graph_label_strings.txt ];
then
    wget https://raw.githubusercontent.com/martinwicke/tensorflow-tutorial/master/imagenet_comp_graph_label_strings.txt -O imagenet_comp_graph_label_strings.txt
fi
if [ ! -f eagle.jpg ];
then
    wget https://t4.ftcdn.net/jpg/00/94/69/61/240_F_94696187_7TrGej016dC4IUdiZgcRANRlp2KAR2nO.jpg -O eagle.jpg
fi

python3 -m http.server 8000