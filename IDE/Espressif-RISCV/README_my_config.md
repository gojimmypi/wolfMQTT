
Ensure you have a private secrets configuration file located (by default) in
 `\workspace\my_private_config.h`.

Place `my_config.h` in the `./main/` directory.

Include the file in source code where secrets are needed:
```C
/* include private my_config.h first  */
#include "my_config.h"
```

Add this text to CMakeLists.txt just before the `include($ENV{IDF_PATH}/tools/cmake/project.cmake)`

```cmake
# BEG"IN my_private_config
#
# we'll look for a my_private_config.h in various environments
# we also assume that the file is added to the local .gitignore
# to ensure it is never inadvertently shared

if(EXISTS "/c/workspace/my_private_config.h")
   message(STATUS "found SYSPROGS_MY_PRIVATE_CONFIG")
   add_definitions( -DSYSPROGS_MY_PRIVATE_CONFIG="/c/workspace/my_private_config.h" )
endif()

if(EXISTS "/workspace/my_private_config.h")
   message(STATUS "found WINDOWS_MY_PRIVATE_CONFIG")
   add_definitions( -DWINDOWS_MY_PRIVATE_CONFIG="/workspace/my_private_config.h" )
endif()

if(EXISTS "/mnt/c/workspace/my_private_config.h")
   message(STATUS "found WSL_MY_PRIVATE_CONFIG")
   add_definitions( -DWSL_MY_PRIVATE_CONFIG="/mnt/c/workspace/my_private_config.h" )
endif()

if(EXISTS "(~/my_private_config.h")
   message(STATUS "found LINUX_MY_PRIVATE_CONFIG")
   add_definitions( -DWSL_MY_PRIVATE_CONFIG="~/my_private_config.h" )
endif()
# END my_private_config
```

When used, the CMake should report something like:

```
[2/7] Building C object esp-idf/main/CMakeFiles/__idf_main.dir/main.c.obj
In file included from ../../../main/main.c:11:
../../../main/my_config.h:100:13: note: #pragma message: Found WINDOWS_MY_PRIVATE_CONFIG !
     #pragma message ( "Found WINDOWS_MY_PRIVATE_CONFIG !" )
             ^~~~~~~
../../../main/my_config.h:101:13: note: #pragma message: "/workspace/my_private_config.h"
     #pragma message ( XSTR(WINDOWS_MY_PRIVATE_CONFIG) )
             ^~~~~~~
```

There may also be messages like this:

```
../../../main/main.c:31: warning: "EXAMPLE_ESP_WIFI_SSID" redefined
 #define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
 
In file included from ../../../main/my_config.h:102,
                 from ../../../main/main.c:11:
/workspace/my_private_config.h:18: note: this is the location of the previous definition
```

In the case of conflicts, consider using names not found in the default `sdkconfig` file.