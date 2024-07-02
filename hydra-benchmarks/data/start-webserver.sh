#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

if [ ! -f ffmpeg ];
then
    wget https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz
    tar -xf ffmpeg-release-amd64-static.tar.xz
    mv ffmpeg-*-amd64-static/ffmpeg .
    rm -r ffmpeg-*-amd64-static
    rm ffmpeg-release-amd64-static.tar.xz
fi

if [ ! -f tensorflow_inception_graph.pb ];
then
    wget https://github.com/martinwicke/tensorflow-tutorial/raw/master/tensorflow_inception_graph.pb
fi

if [ ! -f resnet50-19c8e357.pth ];
then
    wget https://github.com/spcl/serverless-benchmarks-data/raw/6a17a460f289e166abb47ea6298fb939e80e8beb/400.inference/411.image-recognition/model/resnet50-19c8e357.pth
fi

if [ ! -f bacillus_subtilis.fasta ];
then
    wget https://github.com/spcl/serverless-benchmarks-data/raw/6a17a460f289e166abb47ea6298fb939e80e8beb/500.scientific/504.dna-visualisation/bacillus_subtilis.fasta
fi


docker run -d -p 8000:80 --rm -v $DIR:/usr/share/nginx/html --name web nginx
