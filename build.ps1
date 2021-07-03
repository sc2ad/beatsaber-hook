$NDKPath = Get-Content $PSScriptRoot/ndkpath.txt
$ErrorActionPreference = "Stop"

$buildScript = "$NDKPath/build/ndk-build"
if (-not ($PSVersionTable.PSEdition -eq "Core")) {
    $buildScript += ".cmd"
}

& $buildScript NDK_PROJECT_PATH=$PSScriptRoot APP_BUILD_SCRIPT=$PSScriptRoot/Android.tests.mk NDK_APPLICATION_MK=$PSScriptRoot/Application.mk clean
# BUILD tests (compile warnings and errors only), always with a clean build
& $buildScript NDK_PROJECT_PATH=$PSScriptRoot APP_BUILD_SCRIPT=$PSScriptRoot/Android.tests.mk NDK_APPLICATION_MK=$PSScriptRoot/Application.mk
if (-not ($LastExitCode -eq 0)) {
    exit $LastExitCode
}
# Cleanup built tests so we don't accidentally upload them as artifacts
& $buildScript NDK_PROJECT_PATH=$PSScriptRoot APP_BUILD_SCRIPT=$PSScriptRoot/Android.tests.mk NDK_APPLICATION_MK=$PSScriptRoot/Application.mk clean
& $buildScript NDK_PROJECT_PATH=$PSScriptRoot APP_BUILD_SCRIPT=$PSScriptRoot/Android.mk NDK_APPLICATION_MK=$PSScriptRoot/Application.mk clean
& $buildScript NDK_PROJECT_PATH=$PSScriptRoot APP_BUILD_SCRIPT=$PSScriptRoot/Android.mk NDK_APPLICATION_MK=$PSScriptRoot/Application.mk
if (-not ($LastExitCode -eq 0)) {
    exit $LastExitCode
}
