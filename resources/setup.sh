#!/bin/bash

if [ ! -f tensorflow_inception_graph.pb ];
then
    wget https://github.com/martinwicke/tensorflow-tutorial/raw/master/tensorflow_inception_graph.pb
fi

if [ ! -f bacillus_subtilis.fasta ];
then
    wget https://github.com/spcl/serverless-benchmarks-data/raw/6a17a460f289e166abb47ea6298fb939e80e8beb/500.scientific/504.dna-visualisation/bacillus_subtilis.fasta
fi

if [ ! -f pebbles.jpg ];
then
    wget -O pebbles.jpg https://github.com/spcl/serverless-benchmarks-data/blob/6a17a460f289e166abb47ea6298fb939e80e8beb/200.multimedia/210.thumbnailer/9_cobblestone-granite-pebbles-1029604.jpg?raw=true
fi

