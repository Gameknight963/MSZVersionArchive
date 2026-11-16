![Downloads](https://img.shields.io/github/downloads/gameknight963/MSZVersionArchive/total)
![Latest Release](https://img.shields.io/github/v/release/gameknight963/MSZVersionArchive)

# Miside Zero versions archive

### Why does this exist?

itch.io doesn't have a useful API, and I wanted to fetch MSZ builds through code. Now it's also become more of a preservation thing since you can't access the old builds through itch.io.

### Disclaimer

This repo is NOT an official download source! It is an unoffial archive primarily for use in my tool [launcher.NET](https://github.com/Gameknight963/launcher.NET).

All trademarks and copyrights belong to their respective owners.

Consider checking out the developer's 
[official itch page](https://voidwaygames.itch.io/misidezero)
to support them!

# Using MSZVersionArchive programmatically

## File structure

The file format and structure distributed in this repo differs from the original itch page.

All Windows artifacts are flat archives. This is because Explorer creates a new folder for you when extracting, so it's the convention for distributing Windows applications.

<details>
<summary>Example</summary>

**Offical:**
```
└───MiSide Zero Alpha.zip
    └───MiSide Zero Alpha
        └───MiSide Zero_Data
            ├───il2cpp_data
            ├───Plugins
            ├───Resources
            └───StreamingAssets
```

**This repo**
```
└───MiSide Zero Alpha 0.72.zip
    └───MiSide Zero_Data
        ├───il2cpp_data
        ├───Plugins
        ├───Resources
        └───StreamingAssets
```

</details>

All Linux builds are wrapped tarballs. You won't get tarbombed by extracting them.

Android builds are just `.apk` files, and iOS builds are `.ipa` files. MacOS builds are a zip file containing the `MisideZero.app/` folder.

## Artifact naming conventions

```
MiSide-Zero-{version}_{platform}.{extension}
```

All artifacts begin with `MiSide-Zero-`, then the version, which is the tag, but without the leading `v`, followed by an underscore, and then the platform name. The platform name is guaranteed to have no dots (`.`) in it, so read until you find one. Anything past that point is the file extension.

The platform name isn't as standardized as it probably should be, but you should be able to get by checking if it _contains_ the following strings, case-insensitive:

 - `win` for Windows
 - `linux` for Linux
 - `ios` for iOS
 - `mac` for MacOS
 - `android` for Android

## Download examples

Runnable examples are available in [C++](examples/cpp/download.cpp),
[C#](examples/csharp/Program.cs), and [Python](examples/python/download.py).
