#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using nlohmann::json;
namespace fs = std::filesystem;

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

size_t append(char* data, size_t size, size_t count, void* context) {
    const auto bytes = size * count;
    // Exceptions must not escape a C callback.
    try { static_cast<std::string*>(context)->append(data, bytes); }
    catch (...) { return 0; }
    return bytes;
}

size_t write_file(char* data, size_t size, size_t count, void* context) {
    return std::fwrite(data, 1, size * count, static_cast<FILE*>(context));
}

void perform(CURL* curl, const std::string& url) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    auto result = curl_easy_perform(curl);
    if (result != CURLE_OK)
        throw std::runtime_error(curl_easy_strerror(result));
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    if (status != 200)
        throw std::runtime_error("HTTP " + std::to_string(status));
}

int run(int argc, char** argv) {
    if (argc < 2 || argc > 4)
        throw std::runtime_error("Usage: msz-download <win|linux|ios|mac|android|--list> [tag|latest] [directory]");
    const std::string platform = lower(argv[1]);
    const std::vector<std::string> platforms = {"win", "linux", "ios", "mac", "android", "--list"};
    if (std::find(platforms.begin(), platforms.end(), platform) == platforms.end())
        throw std::runtime_error("Unknown platform");
    const std::string tag = argc > 2 ? argv[2] : "latest";
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
    if (!curl) throw std::runtime_error("Cannot initialize libcurl");
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "MSZVersionArchive-example");
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 30L);
    curl_easy_setopt(curl.get(), CURLOPT_LOW_SPEED_LIMIT, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_LOW_SPEED_TIME, 60L);
    std::unique_ptr<char, decltype(&curl_free)> escaped(
        curl_easy_escape(curl.get(), tag.c_str(), static_cast<int>(tag.size())), curl_free);
    if (!escaped) throw std::runtime_error("Cannot encode release tag");
    const std::string endpoint = tag == "latest" ? "latest" : "tags/" + std::string(escaped.get());
    std::string body;
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, append);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &body);
    perform(curl.get(), "https://api.github.com/repos/Gameknight963/MSZVersionArchive/releases/" + endpoint);
    const auto release = json::parse(body);
    const auto release_tag = release.at("tag_name").get<std::string>();
    std::cout << "Release: " << release_tag << '\n';
    if (platform == "--list") {
        for (const auto& asset : release.at("assets"))
            std::cout << asset.at("name").get<std::string>() << '\n';
        return 0;
    }

    // Match only the platform segment, never the version or file extension.
    // Tags and filename versions differ in this archive (including the mono build).
    const std::string prefix = "MiSide-Zero-";
    std::vector<json> matches;
    for (const auto& asset : release.at("assets")) {
        const auto name = asset.at("name").get<std::string>();
        const auto separator = name.find('_');
        if (name.compare(0, prefix.size(), prefix) != 0 || separator == std::string::npos) continue;
        const auto suffix = name.substr(separator + 1);
        if (lower(suffix.substr(0, suffix.find('.'))).find(platform) != std::string::npos)
            matches.push_back(asset);
    }
    if (matches.size() != 1)
        throw std::runtime_error("Expected one matching asset, found " + std::to_string(matches.size()) + ". Use --list to inspect assets.");
    const auto& asset = matches.front();
    const auto name = asset.at("name").get<std::string>();
    if (name.find_first_of("/\\") != std::string::npos || fs::path(name).filename() != name)
        throw std::runtime_error("Invalid asset filename");
    const fs::path directory = argc > 3 ? argv[3] : ".";
    fs::create_directories(directory);
    const auto destination = directory / name;
    // C11 exclusive mode preserves existing downloads (supported by MSVC and glibc).
    std::unique_ptr<FILE, decltype(&std::fclose)> output(
        std::fopen(destination.string().c_str(), "wbx"), std::fclose);
    if (!output) throw std::runtime_error("Cannot create output file (it may already exist)");
    try {
        std::cout << "Downloading: " << name << std::endl;
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_file);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, output.get());
        perform(curl.get(), asset.at("browser_download_url").get<std::string>());
        const int flushed = std::fflush(output.get());
        const int closed = std::fclose(output.release());
        if (flushed != 0 || closed != 0 || fs::file_size(destination) != asset.at("size").get<std::uintmax_t>())
            throw std::runtime_error("Downloaded size or file write check failed");
    } catch (...) {
        output.reset();
        std::error_code ignored;
        fs::remove(destination, ignored);
        throw;
    }
    std::cout << "Saved: " << fs::absolute(destination).string() << '\n';
    return 0;
}

int main(int argc, char** argv) {
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) return 1;
    int result = 1;
    try { result = run(argc, argv); }
    catch (const std::exception& error) { std::cerr << "Error: " << error.what() << '\n'; }
    curl_global_cleanup();
    return result;
}
