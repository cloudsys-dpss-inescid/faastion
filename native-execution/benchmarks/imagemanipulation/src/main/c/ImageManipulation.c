#include "../../../build/generated/sources/headers/java/main/ImageManipulation.h"
#include <stdio.h>
#include <vips/vips.h>


JNIEXPORT jint JNICALL Java_ImageManipulation_manipulation
  (JNIEnv *env, jobject thisObj, jstring param0, jstring param1, jstring param2){
  
  VipsImage *in;
  VipsImage *out;
 
  char* program = (*env)->GetStringUTFChars(env,param0,NULL);
  char* input = (*env)->GetStringUTFChars(env,param1,NULL);
  char* output = (*env)->GetStringUTFChars(env,param2,NULL);

  double scale = 0.5;

  if(VIPS_INIT("libimage.so"))
        vips_error_exit( NULL );

  if(!(in = vips_image_new_from_file(input, NULL)))
        vips_error_exit( NULL );

  printf("image width before resizing: %d\n", vips_image_get_width(in));
  printf("image height before resizing: %d\n", vips_image_get_height(in));

  if(vips_resize(in, &out, scale, NULL))
        vips_error_exit("unable to resize image");

  printf("image width after resizing: %d\n", vips_image_get_width(out));
  printf("image height after resizing: %d\n", vips_image_get_height(out));

  if (vips_image_write_to_file(out, output, NULL))
        vips_error_exit("unable to save output image");

  g_object_unref(in);
  g_object_unref(out);
  vips_shutdown();

  return 0;
  }
