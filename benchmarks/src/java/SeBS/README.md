# Building an app in Faastion

This guide provides step-by-step instructions on how to build a new Java app for Faastion.

## Setting Up Application Folders

1. Create a new folder, following the structure of the existing applications (in this guide, we will use `gv-native-bfs` as an example).

2. Inside the newly created folder, you should have the following files (we will use gradle to build your application):
    - [build.gradle](gv-native-bfs/build.gradle)
    - [gradle](gv-native-bfs/gradle/)
    - [gradlew](gv-native-bfs/gradlew)
    - [settings.gradle](gv-native-bfs/settings.gradle) \
    You should also have a build script and Makefile to help automate the build process:
    - [build_script.sh](gv-native-bfs/build_script.sh)
    - [Makefile](gv-native-bfs/Makefile)

3. Create a `src` directory to package your source code, and develop your application. Following the same structure as `gv-native-bfs`, you can have a `src/main/java` directory for Java code and `src/main/c` for C code.

4. Update `build.gradle`, change the "Main-Class" qualified package name and dependencies if necessary. \
Update `settings.gradle`  if you wish to change the name of the final jar file. \
Update `build_script.sh`. Make sure to change the `build_native_library` function to compile your C library with the required dependencies and flags. Make sure to change `build_ni` and `build_native_binary` functions if you changed the qualified package name and the name of the final jar file.

> [!NOTE]
> Because we use the docker image to build the application, make sure that any C libraries and dependencies are available in the container.

## Compilation

To compile your applications, use the docker image created during setup. Execute [launch_container_image.sh](../../../../images/faastion/launch_container_image.sh) and move to the application's folder you wish to build. 
```bash
cd src/java/SeBS/gv-native-bfs
./build_script.sh
```
Once finished, you can terminate the container. The build process has generated 2 zip files: `libbfs.zip` and `libbfs-plugin.zip`. The former contains the original jar file, and the latter contains a modified jar file (with trampoline calls to the native functions).

If you run the build script inside the docker image, then you will notice that the `Makefile` also takes care of compiling the wrapper libraries (using the modified libc).

After you build your application, you can run the following commands to register the function in Faastion:
```bash
# register original function (no bytecode transformation)
curl -s -X POST "127.0.0.1:8080/register?"\
"entryPoint=com.jni.BFS"\
"&language=java"\
"&name=bfs0"\
"&sandbox=isolate"\
"&url=http://127.0.0.1:8000/apps/libbfs.zip"

# register modified function (with wrapper libraries)
curl -s -X POST "127.0.0.1:8080/register?"\
"entryPoint=com.jni.BFS"\
"&language=java"\
"&name=bfs1"\
"&sandbox=pku"\
"&url=http://127.0.0.1:8000/apps/libbfs-plugin.zip"
```

Notice that the original function uses an `isolate` sandbox type, and the modified function uses a `pku` (hybrid) sandbox. The original function provides the url for `libbfs.zip`, while the modified function provides the url for `libbfs-plugin.zip`.
