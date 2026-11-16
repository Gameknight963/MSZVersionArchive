# Download examples

All three use the same arguments:

```text
<win|linux|ios|mac|android|--list> [tag|latest] [directory]
```

## Python

Requires Python 3.8 or newer, with no third-party packages.

```sh
python examples/python/download.py --list
python examples/python/download.py win
python examples/python/download.py linux EXACT_TAG downloads
```

## C#

Requires the .NET 8 SDK or newer. No NuGet packages are needed.

```sh
dotnet run --project examples/csharp -- --list
dotnet run --project examples/csharp -- win
dotnet run --project examples/csharp -- android EXACT_TAG downloads
```

## C++

Requires a C++17 compiler, CMake 3.16 or newer, libcurl, and nlohmann/json. For example, on Debian/Ubuntu:

```sh
sudo apt install cmake g++ libcurl4-openssl-dev nlohmann-json3-dev
cmake -S examples/cpp -B examples/cpp/build
cmake --build examples/cpp/build --config Release
./examples/cpp/build/msz-download --list
./examples/cpp/build/msz-download win
./examples/cpp/build/msz-download mac EXACT_TAG downloads
```

On Windows with Visual Studio and vcpkg:

```powershell
vcpkg install curl:x64-windows nlohmann-json:x64-windows
cmake -S examples/cpp -B examples/cpp/build -A x64 "-DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build examples/cpp/build --config Release
./examples/cpp/build/Release/msz-download.exe --list
./examples/cpp/build/Release/msz-download.exe win
```