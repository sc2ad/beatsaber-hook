# Beat Saber Quest (il2cpp) modloader/function Hooking Framework

_known by literally no one as pineappl_

## Information

This library offers a wide realm of C++11 to C++20 features for modding il2cpp games on Android devices (specifically meant for the Oculus Quest 1 and 2).

Do not be put off by the name: `beatsaber-hook`, this is entirely game agnostic.

The only dependency it requires is a way of determining the application ID, this is currently done by calling a modloader exposed function, from: [my proxy libmain modloader](https://github.com/sc2ad/QuestLoader/tree/staticModloader)

It also requires that the game is of a unity version >= 2018.0 and < 2020.x.

It is also MUCH more useful if il2cpp functionality is not stripped, ex: exports from the `libil2cpp.so` match most of the symbol names here (with an `il2cpp_` prefix): [shared/utils/il2cpp-functions.hpp](https://github.com/sc2ad/beatsaber-hook/blob/master/shared/utils/il2cpp-functions.hpp#L89).
This will provide a good deal of functionality, and some degree of these functions will be resolved by xref tracing to find non-exported symbols for use in other cases.

## High Level Functionality

This framework provides a good deal of functionality, including, but not limited to, the following:

- `il2cpp_utils` API, for accessing the exported API in a more ergonomic and type safe manner.
- Opt-in runtime type checking
- Detailed exceptions, opt-in-able
- Helpers for installing inline hooks (build within each mod: `And64InlineHook.hpp`)
- Helpers for parsing many ARM64 instructions (`instruction-parsing.hpp`)
- Macros for hook installation + trampoline allocation, type safety conversions
- Performant context logging (including to file)
- Interop with [the modloader](https://github.com/sc2ad/QuestLoader/tree/staticModloader)
- Exposal of many non-exported il2cpp API functions
- And many other things that I forgot while writing this list

## Usage

`beatsaber-hook` is what I consider to be a _library_. Thus, it does not do anything on ELF load, instead only when functions are called from mods does it do anything.

The most important thing to note is timing: **Most beatsaber-hook functions require `il2cpp_init` to be called!**

This means that if you have a modloader that performs things _before_ that call (as mine does), you will need to delay calling any of these functions (`il2cpp_utils` or `il2cpp_functions`) until that point.

Typically, the workflow for a mod involves hooking into some existing game functionality on `load` (called after `il2cpp_init`) and initializing the bs-hook API, which will cache most of its accesses.

This is done simply by calling `il2cpp_functions::Init()`, or by calling any `il2cpp_utils` API function that uses `il2cpp_functions` internally.

## Building + Contributing

All contributions are welcome, [license available here](https://github.com/sc2ad/beatsaber-hook/blob/master/LICENSE)

In order to build, you require the 64/32 bit version of the modloader that you need to link against, along with its headers.

This can be done either by building/downloading a version of [my modloader](https://github.com/sc2ad/QuestLoader/tree/staticModloader) or by using [qpm](https://github.com/sc2ad/QuestPackageManager) and performing a `qpm restore`

When building, you can use the `build.ps1` script, but you must first create an `ndkpath.txt` with your path to NDK. You should have at least NDK r21b, r22 is recommended for C++20 features.

There are a bunch of defines that can control the built code a bit, TODO add them all here as a list. (for now just read the locations of `#ifdef` or `#ifndef`)

## Acknowledgements

This wouldn't have been possible without a few people who have helped immensely.

- [emulamer](https://github.com/emulamer/): Created [BeatOn](https://github.com/emulamer/BeatOn), provided support, tested, and in general just a great person to talk to. This wouldn't exist at all if not for him motivating me to keep working.
- [jakibaki](https://github.com/jakibaki/): Created the library that actually supports all of the mod loading, as well as a lot of support when developing (and bearing with my annoying questions).
- [leo60228](https://github.com/leo60228): Created the custom colors mod using jakibaki's mod loader. The first mod that used his library.
- Trueavid#8335 (Discord): Helped test various things.
