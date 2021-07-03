$NDKPath = Get-Content ./ndkpath.txt

$buildScript = "$NDKPath/build/ndk-build"
if (-not ($PSVersionTable.PSEdition -eq "Core")) {
    $buildScript += ".cmd"
}

& $buildScript NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.tests.mk NDK_APPLICATION_MK=./Application.mk clean
# BUILD tests (compile warnings and errors only), always with a clean build
& $buildScript NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.tests.mk NDK_APPLICATION_MK=./Application.mk
# Cleanup built tests so we don't accidentally upload them as artifacts
& $buildScript NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.tests.mk NDK_APPLICATION_MK=./Application.mk clean
& $buildScript NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk clean
& $buildScript NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk
