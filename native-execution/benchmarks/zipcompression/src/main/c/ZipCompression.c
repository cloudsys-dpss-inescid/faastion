#include "../../../build/generated/sources/headers/java/main/ZipCompression.h"
#include <string.h>
#include <zip.h>
#include <stdio.h>
#include <dirent.h>

int recursive_compression(struct zip *zip_archive, char* init_dir, char* dir){

    DIR *directory = opendir(dir);
    if (directory == NULL) {
        zip_close(zip_archive);
        return -1;
    }
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }else{
            char file_name[PATH_MAX];
            snprintf(file_name, PATH_MAX, "%s/%s", dir, entry->d_name);
            if (entry->d_type == DT_DIR) {
                    //char* substring = file_name + strlen(init_dir);
                    if(zip_dir_add(zip_archive, file_name, ZIP_FL_ENC_UTF_8) < 0 ) {
                            printf("Error");
                            break;
                    }
                    recursive_compression(zip_archive, init_dir, file_name);
            }else{
                    struct zip_source *source = zip_source_file(zip_archive, file_name, 0, 0);
                    char *substring = strstr(file_name, init_dir);
                    substring += strlen(init_dir) + 1;
                    if(zip_file_add(zip_archive, substring, source, ZIP_FL_ENC_UTF_8) < 0){
                            printf("Error");
                            break;
                    }
                    }
                }
        }
        closedir(directory);
	return 0;
}

JNIEXPORT jint JNICALL Java_ZipCompression_compress
  (JNIEnv *env, jobject thisObj, jstring param1, jstring param2){
    
    char* input = (*env)->GetStringUTFChars(env,param1,NULL);
    char* output = (*env)->GetStringUTFChars(env,param2,NULL);
    struct zip *zip_archive = zip_open(output, ZIP_CREATE | ZIP_EXCL, NULL);
    
    if (zip_archive == NULL) {
        return -1;
    
    }
    recursive_compression(zip_archive, input,input);
    zip_close(zip_archive);
    return 0;
  }


