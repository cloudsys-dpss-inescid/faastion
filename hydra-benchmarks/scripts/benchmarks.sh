#!/bin/bash

# Getting the local default ip used to connect to the internet.
IP=$(ip route get 8.8.8.8 | grep -oP  'src \K\S+')
PORT=8000

JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS gv_java_sleep"
JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS gv_java_native_hw" 
JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS gv_java_native_msg"
JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS gv_java_native_test"
JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS gv_java_hw"                 # 256 MB
JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS gv_java_shopcart"
JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS gv_java_filehashing"        # 256 MB
JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS gv_java_classify"           # 1024 MB
JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS gv_java_httprequest"        # 256 MB
JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS gv_java_videoprocessing"    # 1024 MB

JV_CR_BENCHMARKS="$JV_CR_BENCHMARKS cr_java_filehashing"        # 256 MB
JV_CR_BENCHMARKS="$JV_CR_BENCHMARKS cr_java_hw"                 # 256 MB
JV_CR_BENCHMARKS="$JV_CR_BENCHMARKS cr_java_httprequest"        # 256 MB
JV_CR_BENCHMARKS="$JV_CR_BENCHMARKS cr_java_classify"           # 1024 MB
JV_CR_BENCHMARKS="$JV_CR_BENCHMARKS cr_java_sleep"
JV_CR_BENCHMARKS="$JV_CR_BENCHMARKS cr_java_videoprocessing"    # 1024 MB

PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS gv_python_sleep"
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS gv_python_hw"               # 256 MB
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS gv_python_mst"              # 512 MB
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS gv_python_bfs"              # 512 MB?
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS gv_python_pagerank"         # 512 MB?
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS gv_python_dna"              # 512 MB?
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS gv_python_classify"         # 512 MB?
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS gv_python_dynamichtml"      # 256 MB
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS gv_python_compression"      # 512 MB
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS gv_python_thumbnail"        # 256 MB
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS gv_python_videoprocessing"  # 512 MB
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS gv_python_uploader"         # 256 MB

PY_CR_BENCHMARKS="$PY_CR_BENCHMARKS cr_python_thumbnail"        # 256 MB
PY_CR_BENCHMARKS="$PY_CR_BENCHMARKS cr_python_compression"      # 256 MB
PY_CR_BENCHMARKS="$PY_CR_BENCHMARKS cr_python_hw"               # 256 MB
PY_CR_BENCHMARKS="$PY_CR_BENCHMARKS cr_python_dynamichtml"      # 256 MB
PY_CR_BENCHMARKS="$PY_CR_BENCHMARKS cr_python_mst"              # 512 MB
PY_CR_BENCHMARKS="$PY_CR_BENCHMARKS cr_python_uploader"         # 256 MB
PY_CR_BENCHMARKS="$PY_CR_BENCHMARKS cr_python_sleep"
PY_CR_BENCHMARKS="$PY_CR_BENCHMARKS cr_python_videoprocessing"  # 512 MB

JS_GV_BENCHMARKS="$JS_GV_BENCHMARKS gv_javascript_sleep"
JS_GV_BENCHMARKS="$JS_GV_BENCHMARKS gv_javascript_hw"           # 256 MB
JS_GV_BENCHMARKS="$JS_GV_BENCHMARKS gv_javascript_dynamichtml"  # 256 MB
JS_GV_BENCHMARKS="$JS_GV_BENCHMARKS gv_javascript_thumbnail"    # 512 MB
JS_GV_BENCHMARKS="$JS_GV_BENCHMARKS gv_javascript_uploader"     # 256 MB

JS_CR_BENCHMARKS="$JS_CR_BENCHMARKS cr_javascript_thumbnail"    # 512 MB
JS_CR_BENCHMARKS="$JS_CR_BENCHMARKS cr_javascript_hw"           # 256 MB
JS_CR_BENCHMARKS="$JS_CR_BENCHMARKS cr_javascript_dynamichtml"  # 256 MB
JS_CR_BENCHMARKS="$JS_CR_BENCHMARKS cr_javascript_uploader"     # 256 MB
JS_CR_BENCHMARKS="$JS_CR_BENCHMARKS cr_javascript_sleep"

GV_BENCHMARKS="$JV_GV_BENCHMARKS $PY_GV_BENCHMARKS $JS_GV_BENCHMARKS"
CR_BENCHMARKS="$JV_CR_BENCHMARKS $PY_CR_BENCHMARKS $JS_CR_BENCHMARKS"

PH_BENCHMARKS="$PH_BENCHMARKS gv_java_hw"                 # 256 MB
PH_BENCHMARKS="$PH_BENCHMARKS gv_java_filehashing"        # 256 MB
PH_BENCHMARKS="$PH_BENCHMARKS gv_java_httprequest"        # 256 MB
PH_BENCHMARKS="$PH_BENCHMARKS gv_java_videoprocessing"    # 1024 MB
PH_BENCHMARKS="$PH_BENCHMARKS gv_java_classify"           # 1024 MB

function gv_java_native_aes {
    APP_LANG=java
    APP_NAME=gv-native-aes
    APP_SO=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/build/libaes.so
    APP_MAIN=com.jni.AESEncryption
    curl -s -X POST $ip:8080/register?name=aes\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"aes","async":"false","cached":"true","arguments":""}' > $APP_POST
}

function gv_java_native_factors {
    APP_LANG=java
    APP_NAME=gv-native-factorization
    APP_SO=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/build/libfactors.so
    APP_MAIN=com.jni.Factorization
    curl -s -X POST $ip:8080/register?name=factors\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"factors","async":"false","cached":"true","arguments":""}' > $APP_POST
}

function gv_java_native_matrix {
    APP_LANG=java
    APP_NAME=gv-native-matrixmul
    APP_SO=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/build/libmatrixmul.so
    APP_MAIN=com.jni.MatrixMultiplication
    curl -s -X POST $ip:8080/register?name=matrixmul\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"matrixmul","async":"false","cached":"true","arguments":""}' > $APP_POST
}

function gv_java_native_hw {
    APP_LANG=java
    APP_NAME=gv-native-hw-1
    APP_SO=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/build/libnativehw1.so
    APP_MAIN=com.jni.HelloJNI
    curl -s -X POST $ip:8080/register?name=nativehw\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"nativehw","async":"false","cached":"true","arguments":""}' > $APP_POST
}

function gv_java_hw {
    APP_LANG=java
    APP_NAME=gv-hello-world
    APP_SO=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/build/libhelloworld.so
    APP_MAIN=com.hello_world.HelloWorld
    curl -s -X POST $ip:8080/register?name=hw\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"hw","async":"false","cached":"true","arguments":""}' > $APP_POST
}

function gv_java_shopcart {
    APP_LANG=java
    APP_NAME=gv-shopcart
    APP_SO=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/target/libshopcart.so
    APP_MAIN=micronaut.benchmark.shopcart.Application
    curl -s -X POST $ip:8080/register?name=shopcart\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"shopcart","async":"false","cached":"true","arguments":""}' > $APP_POST
}

function cr_java_hw {
    IMG=docker.io/openwhisk/java8action:latest
    APP_LANG=java
    APP_NAME=cr-hello-world
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
}

function ph_java_hw {
    IMG=docker.io/sergiyivan/large-scale-experiment:photons
    APP_LANG=java
    APP_NAME=cr-hello-world
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    APP_NAME=ph-hello-world
}

# Builtin means that we are using Truffle's builtin capability in graalvisor.
function gv_javascript_hw_builtin {
    APP_LANG=javascript
    APP_NAME=gv-hello-world
    APP_MAIN=main
    APP_SCRIPT=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/src/main/javascript/main.js
    curl -s -X POST $ip:8080/register?name=hw\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SCRIPT
    echo '{"name":"hw","async":"false","arguments":""}' > $APP_POST
}

function gv_javascript_hw {
    APP_LANG=java
    APP_NAME=gv-js-hello-world
    APP_MAIN=com.helloworld.HelloWorld
    APP_SCRIPT=$BENCHMARKS_HOME/src/javascript/gv-hello-world/build/libhelloworld.so
    curl -s -X POST $ip:8080/register?name=hw\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SCRIPT
    echo '{"name":"hw","async":"false","arguments":""}' > $APP_POST
}

function cr_javascript_hw {
    IMG=docker.io/openwhisk/action-nodejs-v12:latest
    APP_LANG=javascript
    APP_NAME=cr-hello-world
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run2.json
}

# Builtin means that we are using Truffle's builtin capability in graalvisor.
function gv_python_hw_builtin {
    APP_LANG=python
    APP_NAME=gv-hello-world
    APP_MAIN=main
    APP_SCRIPT=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/src/main/python/main.py
    curl -s -X POST $ip:8080/register?name=hw\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SCRIPT
    echo '{"name":"hw","async":"false","arguments":""}' > $APP_POST
}

function gv_python_hw {
    APP_LANG=java
    APP_NAME=gv-py-hello-world
    APP_MAIN=com.helloworld.HelloWorld
    APP_SCRIPT=$BENCHMARKS_HOME/src/python/gv-hello-world/build/libhelloworld.so
    curl -s -X POST $ip:8080/register?name=hw\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SCRIPT
    echo '{"name":"hw","async":"false","arguments":""}' > $APP_POST
}

function cr_python_hw {
    IMG=docker.io/openwhisk/action-python-v3.9:latest
    APP_LANG=python
    APP_NAME=cr-hello-world
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run2.json
}

function gv_python_thumbnail {
    APP_LANG=java
    APP_NAME=gv-py-thumbnail
    APP_MAIN=com.thumbnail.Thumbnail
    APP_SO=$BENCHMARKS_HOME/src/python/gv-thumbnail/build/libthumbnail.so
    curl -s -X POST $ip:8080/register?name=thumbnail\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"thumbnail","async":"false","arguments":"{\"url\":\"http://'$IP':'$PORT'/snap.png\"}"}' > $APP_POST
}

function cr_python_thumbnail {
    IMG=docker.io/openwhisk/action-python-v3.9:latest
    APP_LANG=python
    APP_NAME=cr-thumbnail
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "url": "http://'$IP':'$PORT'/snap.png" } }' > $RUN_POST
}

function gv_javascript_thumbnail {
    APP_LANG=java
    APP_NAME=gv-js-thumbnail
    APP_MAIN=com.thumbnail.Thumbnail
    APP_SO=$BENCHMARKS_HOME/src/javascript/gv-thumbnail/build/libthumbnail.so
    curl -s -X POST $ip:8080/register?name=thumbnail\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"thumbnail","async":"false","arguments":"{\"url\":\"http://'$IP':'$PORT'/snap.png\"}"}' > $APP_POST
}

function cr_javascript_thumbnail {
    IMG=docker.io/openwhisk/action-nodejs-v12:latest
    APP_LANG=javascript
    APP_NAME=cr-thumbnail
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "url": "http://'$IP':'$PORT'/snap.png" } }' > $RUN_POST
}

function gv_java_genericapp {
    APP_LANG=java
    APP_NAME=gv-genericapp
    APP_MAIN=com.genericapp.GenericApp
    APP_SO=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/build/libgenericapp.so
    curl -s -X POST $ip:8080/register?name=genericapp\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"genericapp","async":"false","arguments":"{\"memory\":\"4000000\",\"duration\":\"1000\"}"}' > $APP_POST
}

function gv_java_sleep {
    APP_LANG=java
    APP_NAME=gv-sleep
    APP_MAIN=com.sleep.Sleep
    APP_SO=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/build/libsleep.so
    curl -s -X POST $ip:8080/register?name=sleep\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"sleep","async":"false","arguments":"{\"memory\":\"128\",\"sleep\":\"1000\"}"}' > $APP_POST
}

function cr_java_sleep {
    IMG=docker.io/openwhisk/java8action:latest
    APP_LANG=java
    APP_NAME=cr-sleep
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
}

function gv_python_sleep {
    APP_LANG=java
    APP_NAME=gv-py-sleep
    APP_MAIN=com.sleep.Sleep
    APP_SCRIPT=$BENCHMARKS_HOME/src/python/gv-sleep/build/libsleep.so
    curl -s -X POST $ip:8080/register?name=sleep\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SCRIPT
    echo '{"name":"sleep","async":"false","arguments":"1"}' > $APP_POST
}

function cr_python_sleep {
    IMG=docker.io/openwhisk/action-python-v3.9:latest
    APP_LANG=python
    APP_NAME=cr-sleep
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
}

function gv_javascript_sleep {
    APP_LANG=java
    APP_NAME=gv-js-sleep
    APP_MAIN=com.sleep.Sleep
    APP_SCRIPT=$BENCHMARKS_HOME/src/javascript/gv-sleep/build/libsleep.so
    curl -s -X POST $ip:8080/register?name=sleep\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SCRIPT
    echo '{"name":"sleep","async":"false","arguments":"{\"time\":\"1\"}"}' > $APP_POST
}

function cr_javascript_sleep {
    IMG=docker.io/openwhisk/action-nodejs-v12:latest
    APP_LANG=javascript
    APP_NAME=cr-sleep
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
}

function gv_java_filehashing {
    APP_LANG=java
    APP_NAME=gv-file-hashing
    APP_MAIN=com.filehashing.FileHashing
    APP_SO=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/build/libfilehashing.so
    curl -s -X POST $ip:8080/register?name=filehashing\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"filehashing","async":"false","arguments":"{\"url\":\"http://'$IP':'$PORT'/snap.png\"}"}' > $APP_POST
}

function cr_java_filehashing {
    IMG=docker.io/openwhisk/java8action:latest
    APP_LANG=java
    APP_NAME=cr-file-hashing
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "url": "http://'$IP':'$PORT'/snap.png" } }' > $RUN_POST
}

function ph_java_filehashing {
    IMG=docker.io/sergiyivan/large-scale-experiment:photons
    APP_LANG=java
    APP_NAME=cr-file-hashing
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "url": "http://'$IP':'$PORT'/snap.png" } }' > $RUN_POST
    APP_NAME=ph-file-hashing
}

function gv_java_httprequest {
    APP_LANG=java
    APP_NAME=gv-httprequest
    APP_MAIN=com.httprequest.HttpRequest
    APP_SO=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/build/libhttprequest.so
    curl -s -X POST $ip:8080/register?name=httprequest\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"httprequest","async":"false","arguments":"{\"url\":\"http://'$IP':'$PORT'/snap.png\"}"}' > $APP_POST
}

function cr_java_httprequest {
    IMG=docker.io/openwhisk/java8action:latest
    APP_LANG=java
    APP_NAME=cr-httprequest
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "url": "http://'$IP':'$PORT'/snap.png" } }' > $RUN_POST
}

function ph_java_httprequest {
    IMG=docker.io/sergiyivan/large-scale-experiment:photons
    APP_LANG=java
    APP_NAME=cr-httprequest
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "url": "http://'$IP':'$PORT'/snap.png" } }' > $RUN_POST
    APP_NAME=ph-httprequest
}

function gv_java_videoprocessing {
    APP_LANG=java
    APP_NAME=gv-video-processing
    APP_MAIN=com.videoprocessing.VideoProcessing
    APP_SO=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/build/libvideoprocessing.so
    curl -s -X POST $ip:8080/register?name=videoprocessing\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"videoprocessing","async":"false","arguments":"{\"video\":\"http://'$IP':'$PORT'/video.mp4\",\"ffmpeg\":\"http://'$IP':'$PORT'/ffmpeg\"}"}' > $APP_POST
}

function cr_java_videoprocessing {
    IMG=docker.io/openwhisk/java8action:latest
    APP_LANG=java
    APP_NAME=cr-video-processing
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "ffmpeg_url": "http://'$IP':'$PORT'/ffmpeg", "video_url": "http://'$IP':'$PORT'/video.mp4" } }' > $RUN_POST
}

function ph_java_videoprocessing {
    IMG=docker.io/sergiyivan/large-scale-experiment:photons
    APP_LANG=java
    APP_NAME=cr-video-processing
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "ffmpeg_url": "http://'$IP':'$PORT'/ffmpeg", "video_url": "http://'$IP':'$PORT'/video.mp4" } }' > $RUN_POST
    APP_NAME=ph-video-processing
}

function gv_python_videoprocessing {
    APP_LANG=java
    APP_NAME=gv-py-video-processing
    APP_MAIN=com.videoprocessing.VideoProcessing
    APP_SO=$BENCHMARKS_HOME/src/python/gv-video-processing/build/libvideoprocessing.so
    curl -s -X POST $ip:8080/register?name=videoprocessing\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"videoprocessing","async":"false","arguments":"{\"video\":\"http://'$IP':'$PORT'/video.mp4\",\"ffmpeg\":\"http://'$IP':'$PORT'/ffmpeg\"}"}' > $APP_POST
}

function cr_python_videoprocessing {
    IMG=docker.io/openwhisk/action-python-v3.9:latest
    APP_LANG=python
    APP_NAME=cr-video-processing
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "ffmpeg_url": "http://'$IP':'$PORT'/ffmpeg", "video_url": "http://'$IP':'$PORT'/video.mp4" } }' > $RUN_POST
}

function gv_java_classify {
    APP_LANG=java
    APP_NAME=gv-classify
    APP_MAIN=com.classify.Classify
    APP_SO=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/build/libclassify.so
    curl -s -X POST $ip:8080/register?name=classify\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"classify","async":"false","arguments":"{\"model_url\":\"http://'$IP':'$PORT'/tensorflow_inception_graph.pb\",\"labels_url\":\"http://'$IP':'$PORT'/imagenet_comp_graph_label_strings.txt\",\"image_url\":\"http://'$IP':'$PORT'/eagle.jpg\"}"}' > $APP_POST
}

function cr_java_classify {
    IMG=docker.io/openwhisk/java8action:latest
    APP_LANG=java
    APP_NAME=cr-classify
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "model_url": "http://'$IP':'$PORT'/tensorflow_inception_graph.pb", "labels_url": "http://'$IP':'$PORT'/imagenet_comp_graph_label_strings.txt", "image_url": "http://'$IP':'$PORT'/eagle.jpg" } }' > $RUN_POST
}

function ph_java_classify {
    IMG=docker.io/sergiyivan/large-scale-experiment:photons
    APP_LANG=java
    APP_NAME=cr-classify
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "model_url": "http://'$IP':'$PORT'/tensorflow_inception_graph.pb", "labels_url": "http://'$IP':'$PORT'/imagenet_comp_graph_label_strings.txt", "image_url": "http://'$IP':'$PORT'/eagle.jpg" } }' > $RUN_POST
    APP_NAME=ph-classify
}

function gv_python_compression {
    APP_LANG=java
    APP_NAME=gv-py-compression
    APP_MAIN=com.compression.Compression
    APP_SO=$BENCHMARKS_HOME/src/python/gv-compression/build/libcompression.so
    curl -s -X POST $ip:8080/register?name=compression\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"compression","async":"false","arguments":"{\"url\":\"http://'$IP':'$PORT'/video.mp4\"}"}' > $APP_POST
}

function cr_python_compression {
    IMG=docker.io/openwhisk/action-python-v3.9:latest
    APP_LANG=python
    APP_NAME=cr-compression
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "url": "http://'$IP':'$PORT'/video.mp4" } }' > $RUN_POST
}

function gv_python_mst {
    APP_LANG=java
    APP_NAME=gv-py-mst
    APP_MAIN=com.mst.MST
    APP_SO=$BENCHMARKS_HOME/src/python/gv-mst/build/libmst.so
    curl -s -X POST $ip:8080/register?name=mst\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"mst","async":"false","arguments":"{\"size\":\"100\"}"}' > $APP_POST
}

function gv_python_bfs {
    APP_LANG=java
    APP_NAME=gv-py-bfs
    APP_MAIN=com.bfs.BFS
    APP_SO=$BENCHMARKS_HOME/src/python/gv-bfs/build/libbfs.so
    curl -s -X POST $ip:8080/register?name=bfs\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"bfs","async":"false","arguments":"{\"size\":\"100\"}"}' > $APP_POST
}

function gv_python_pagerank {
    APP_LANG=java
    APP_NAME=gv-py-pagerank
    APP_MAIN=com.pr.PageRank
    APP_SO=$BENCHMARKS_HOME/src/python/gv-pagerank/build/libpr.so
    curl -s -X POST $ip:8080/register?name=pagerank\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"pagerank","async":"false","arguments":"{\"size\":\"10\"}"}' > $APP_POST
}

function gv_python_dna {
    APP_LANG=java
    APP_NAME=gv-py-dna
    APP_MAIN=com.dna.DNA
    APP_SO=$BENCHMARKS_HOME/src/python/gv-dna/build/libdna.so
    curl -s -X POST $ip:8080/register?name=dna\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"dna","async":"false","arguments":"{\"fasta_url\":\"http://'$IP':'$PORT'/bacillus_subtilis.fasta\"}"}' > $APP_POST
}

function gv_python_classify {
    APP_LANG=java
    APP_NAME=gv-py-classify
    APP_MAIN=com.classify.Classify
    APP_SO=$BENCHMARKS_HOME/src/python/gv-classify/build/libclassify.so
    curl -s -X POST $ip:8080/register?name=classify\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"classify","async":"false","arguments":"{\"restnet_url\":\"http://'$IP':'$PORT'/resnet50-19c8e357.pth\",\"img_url\":\"http://'$IP':'$PORT'/snap.png\"}"}' > $APP_POST
}

function cr_python_mst {
    IMG=docker.io/openwhisk/action-python-v3.9:latest
    APP_LANG=python
    APP_NAME=cr-mst
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "size": "100" } }' > $RUN_POST
}

function gv_javascript_dynamichtml {
    APP_LANG=java
    APP_NAME=gv-js-dynamic-html
    APP_MAIN=com.dynamichtml.DynamicHTML
    APP_SO=$BENCHMARKS_HOME/src/javascript/gv-dynamic-html/build/libdynamichtml.so
    curl -s -X POST $ip:8080/register?name=dynamichtml\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"dynamichtml","async":"false","arguments":"{\"url\":\"http://'$IP':'$PORT'/template.html\",\"username\":\"rbruno\",\"nsize\":\"10\"}"}' > $APP_POST
}

function cr_javascript_dynamichtml {
    IMG=docker.io/openwhisk/action-nodejs-v12:latest
    APP_LANG=javascript
    APP_NAME=cr-dynamic-html
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "url": "http://'$IP':'$PORT'/template.html", "username": "rbruno", "nsize": "10" } }' > $RUN_POST
}

function gv_python_dynamichtml {
    APP_LANG=java
    APP_NAME=gv-py-dynamic-html
    APP_MAIN=com.dynamichtml.DynamicHTML
    APP_SO=$BENCHMARKS_HOME/src/python/gv-dynamic-html/build/libdynamichtml.so
    curl -s -X POST $ip:8080/register?name=dynamichtml\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"dynamichtml","async":"false","arguments":"{\"url\":\"http://'$IP':'$PORT'/template.html\",\"username\":\"rbruno\",\"nsize\":\"10\"}"}' > $APP_POST
}

function cr_python_dynamichtml {
    IMG=docker.io/openwhisk/action-python-v3.9:latest
    APP_LANG=python
    APP_NAME=cr-dynamic-html
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "url": "http://'$IP':'$PORT'/template.html", "username": "rbruno", "nsize": "10" } }' > $RUN_POST
}

function gv_python_uploader {
    APP_LANG=java
    APP_NAME=gv-py-uploader
    APP_MAIN=com.uploader.Uploader
    APP_SCRIPT=$BENCHMARKS_HOME/src/python/gv-uploader/build/libuploader.so
    curl -s -X POST $ip:8080/register?name=uploader\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SCRIPT
    echo '{"name":"uploader","async":"false","arguments":"{\"url\":\"http://'$IP':'$PORT'/snap.png\"}"}' > $APP_POST
}

function cr_python_uploader {
    IMG=docker.io/openwhisk/action-python-v3.9:latest
    APP_LANG=python
    APP_NAME=cr-uploader
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "url": "http://'$IP':'$PORT'/snap.png" } }' > $RUN_POST
}

function gv_javascript_uploader {
    APP_LANG=java
    APP_NAME=gv-js-uploader
    APP_MAIN=com.uploader.Uploader
    APP_SCRIPT=$BENCHMARKS_HOME/src/javascript/gv-uploader/build/libuploader.so
    curl -s -X POST $ip:8080/register?name=uploader\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SCRIPT
    echo '{"name":"uploader","async":"false","arguments":"{\"url\":\"http://'$IP':'$PORT'/snap.png\"}"}' > $APP_POST
}

function cr_javascript_uploader {
    IMG=docker.io/openwhisk/action-nodejs-v12:latest
    APP_LANG=javascript
    APP_NAME=cr-uploader
    INIT_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/init.json
    RUN_POST=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/run.json
    echo '{ "value": { "url": "http://'$IP':'$PORT'/snap.png" } }' > $RUN_POST
}

function gv_python_warble {
    APP_LANG=python
    APP_NAME=gv-warble
    APP_MAIN=main
    APP_SCRIPT=$BENCHMARKS_HOME/src/$APP_LANG/$APP_NAME/main.py
    curl -s -X POST $ip:8080/register?name=warble\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SCRIPT
    echo $'{"name":"warble","async":"false","arguments":"[\'-v\',\'{PRINT(\\"test\\")}\']"}' > $APP_POST
}
