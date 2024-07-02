#include <jni.h>
#include <stdio.h>
#include "my_app_HelloWorld.h"

JNIEXPORT void JNICALL Java_my_app_HelloWorld_print
  (JNIEnv * env, jclass class) {
        printf("Hello world; this is C talking!\n");
  }
