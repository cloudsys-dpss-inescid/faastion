function gv_java_native_factors {
    APP_LANG=java
    APP_NAME=gv-native-factorization
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libfactors.so
    APP_MAIN=com.jni.Factorization
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8080/register?name=factors\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"factors","async":"false","cached":"true","arguments":""}' > /tmp/payload1.post
}

function gv_java_native_matmul {
    APP_LANG=java
    APP_NAME=gv-native-matmul
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libmatmul.so
    APP_MAIN=com.jni.MatrixMultiplication
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8080/register?name=matmul\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"matmul","async":"false","cached":"true","arguments":""}' > /tmp/payload2.post
}

function gv_java_httprequest {
    APP_LANG=java
    APP_NAME=gv-httprequest
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libhttprequest.so
    APP_MAIN=com.httprequest.HttpRequest
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8080/register?name=httprequest\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"httprequest","async":"false","cached":"true","arguments":""}' > /tmp/payload3.post
}

function gv_java_sleep {
    APP_LANG=java
    APP_NAME=gv-sleep
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libsleep.so
    APP_MAIN=com.sleep.Sleep
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8080/register?name=sleep\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"sleep","async":"false","cached":"true","arguments":""}' > /tmp/payload4.post
}

function gv_java_native_hw {
    APP_LANG=java
    APP_NAME=gv-native-hw
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libnativehw.so
    APP_MAIN=com.jni.HelloJNI
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8080/register?name=nativehw\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"nativehw","async":"false","cached":"true","arguments":""}' > /tmp/payload5.post
}

function gv_java_hw {
    APP_LANG=java
    APP_NAME=gv-hello-world
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libhelloworld.so
    APP_MAIN=com.hello_world.HelloWorld
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8080/register?name=hw\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"hw","async":"false","cached":"true","arguments":""}' > /tmp/payload6.post
}

export SANDBOX=isolate

gv_java_native_factors
gv_java_native_matmul
gv_java_httprequest
gv_java_sleep
gv_java_native_hw
gv_java_hw

wrk -t$1 -c$1 -d30s -s native.lua http://127.0.0.1:8080
