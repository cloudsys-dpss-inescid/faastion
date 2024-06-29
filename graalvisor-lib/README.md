# GraalVisor API library

GraalVisor API library, containing APIs for guest and host applications.

## Project Structure

[GraalVisor](src/main/java/com/oracle/svm/graalvisor/GraalVisor.java): Definition of Java Access methods to the underlying C struct.

[GraalVisorImpl](src/main/java/com/oracle/svm/graalvisor/GraalVisorImpl.java): Implementations of functions that are instrumented into GraalVisor struct. The functions pointers are assigned here and later exposed to guest Isolate.

[GraalVisor API](src/main/java/com/oracle/svm/graalvisor/api/) APIs used for host applications. Using GraalVisor API, the host application can load dynamically-linked library into memory. After that, the host application can manage guest isolates and call invocation into them. More details please refer to [GraalVisorAPI](src/main/java/com/oracle/svm/graalvisor/api/GraalVisorAPI.java)

[GraalVisor Guest API](src/main/java/com/oracle/svm/graalvisor/guestapi/) APIs used for guest applications. Through the APIs defined in [GuestAPI](src/main/java/com/oracle/svm/graalvisor/guestapi/GuestAPI.java) guest application can delegate different function call to the host isolate, including string copying and file manipulation.

[GraalVisor Isolate Types](src/main/java/com/oracle/svm/graalvisor/types/) Types with empty extension to distinguish host isolates and guest isolates.

[utils](src/main/java/com/oracle/svm/graalvisor/utils/) Utilities including String retrieving, build options checking and file access mode.

[resources](src/main/resources/com.oracle.svm.graalvisor.headers) Contains header files that describe isolate native api and GraalVisor C struct.

## Native Image Builder Options

`-DGraalVisorHost=true` Telling native-image builder that current application is going to build as a GraalVisor host application. In this case `com.oracle.svm.graalvisor-1.0-host.jar` should be used as dependency.

`-DGraalVisorGuest=true` Telling native-image builder that current application is going to build as a GraalVisor guest application. In this case `com.oracle.svm.graalvisor-1.0-guest.jar` should be used as dependency.

***One and only one of the `-DGraalVisorGuest` and `-DGraalVisorGuest` must be set as true.***

`-Dcom.oracle.svm.graalvisor.libraryPath=PATH_TO_HEADERS` Telling the native-image builder the position of the header files `graal_visor.h` and `graal_isolate.h`

## How to build

run `./build.sh` would create GraalVisor guest library, host library and commons library under `build/libs/*.jar`