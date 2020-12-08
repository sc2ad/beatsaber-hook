.\build.ps1

& adb push ./obj/local/arm64-v8a/libbeatsaber-hook_1_0_0.so /sdcard/Android/data/com.beatgames.beatsaber/files/libs/libbeatsaber-hook_1_0_0.so
Copy-Item -Force obj/local/arm64-v8a/libbeatsaber-hook_1_0_0.so ../libbeatsaber-hook_1_0_0.so