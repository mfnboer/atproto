To package the java code in a .aar file:

> export ANDROID_HOME=~/Android/Sdk
> ./gradlew assembleRelease

The .aar file will be in build/outputs/aar/

To add the .aar file to your app project, copy it to your android/libs

Add this to your CMake

if(ANDROID)
    set_property(TARGET yourapp APPEND PROPERTY
        QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
    )
endif()

This will add anything in android/libs to your project
