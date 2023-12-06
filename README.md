# ndi_player

A new Flutter project.


build with shrinking
flutter build apk --release --shrink --obfuscate --split-debug-info=misc/mapping/${version}

# If rust build cannot find clang++
export PATH="$HOME/Android/Sdk/ndk/23.1.7779620/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH"

# If error occurred: Failed to find tool. Is `aarch64-linux-android-ar` installed?
export PATH="$HOME/Android/Sdk/ndk/21.1.6352462/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH"