.\build.ps1

adb shell am force-stop com.beatgames.beatsaber

adb push libs/arm64-v8a/libbeatsaber-hook.so /sdcard/Android/data/com.beatgames.beatsaber/files/libs/libbeatsaber-hook_0_3_9.so
cp obj/local/arm64-v8a/libbeatsaber-hook.so ..\libbeatsaber-hook_0_3_9.so
