#!/bin/bash

function run_java {
	$JAVA_HOME/bin/java \
		-cp build/libs/demo-polyglot-engine-sharing-1.0-all.jar \
		com.demo_polyglot.DemoPolyglot \
		true \
		10 \
		python \
		../../src/python/gv-thumbnail/main.py \
		http://localhost:8000/snap.png | tee sharing.log

	$JAVA_HOME/bin/java \
		-cp build/libs/demo-polyglot-engine-sharing-1.0-all.jar \
		com.demo_polyglot.DemoPolyglot \
		false \
		10 \
		python \
		../../src/python/gv-thumbnail/main.py \
		http://localhost:8000/snap.png | tee nosharing.log

}


function run_ni {
	build/demopolyglot \
		true \
		10 \
		python \
		../../src/python/gv-thumbnail/main.py \
		http://localhost:8000/snap.png | tee sharing.log

	build/demopolyglot \
		false \
		10 \
		python \
		../../src/python/gv-thumbnail/main.py \
		http://localhost:8000/snap.png | tee nosharing.log
}

#run_java
run_ni

cat nosharing.log | grep MBs | awk '{print $9}' > mem_nosharing.dat
cat sharing.log   | grep MBs | awk '{print $9}' > mem_sharing.dat
cat nosharing.log | grep MBs | awk '{print $7}' > lat_nosharing.dat
cat sharing.log   | grep MBs | awk '{print $7}' > lat_sharing.dat
cat nosharing.log | grep took | grep -v MBs | awk '{print $5}' > req_nosharing.dat
cat sharing.log   | grep took | grep -v MBs | awk '{print $5}' > req_sharing.dat
