using System.Text.Json;

try
{
    if (args.Length is < 1 or > 3)
        throw new ArgumentException("Usage: DownloadExample <win|linux|ios|mac|android|--list> [tag|latest] [directory]");

    string platform = args[0].ToLowerInvariant();

    if (platform is not ("win" or "linux" or "ios" or "mac" or "android" or "--list"))
        throw new ArgumentException("Unknown platform. Use win, linux, ios, mac, or android.");

    string tag = args.Length > 1 ? args[1] : "latest";
    string directory = args.Length > 2 ? args[2] : ".";
    string endpoint = tag == "latest" ? "latest" : "tags/" + Uri.EscapeDataString(tag);

    using HttpClient client = new() { Timeout = TimeSpan.FromMinutes(30) };
    client.DefaultRequestHeaders.UserAgent.ParseAdd("MSZVersionArchive-example");

    using HttpRequestMessage request = new(HttpMethod.Get,
        "https://api.github.com/repos/Gameknight963/MSZVersionArchive/releases/" + endpoint);

    request.Headers.Accept.ParseAdd("application/vnd.github+json");

    using HttpResponseMessage response = await client.SendAsync(request);
    response.EnsureSuccessStatusCode();

    using JsonDocument release = JsonDocument.Parse(await response.Content.ReadAsStringAsync());

    string releaseTag = release.RootElement.GetProperty("tag_name").GetString()!;
    JsonElement[] assets = release.RootElement.GetProperty("assets").EnumerateArray().ToArray();

    Console.WriteLine($"Release: {releaseTag}");

    if (platform == "--list")
    {
        foreach (JsonElement asset in assets)
            Console.WriteLine(asset.GetProperty("name").GetString());

        return 0;
    }

    // Match only the platform segment, never the version or file extension.
    // Tags and filename versions differ in this archive (including the mono build).
    JsonElement[] matches = assets.Where(asset =>
    {
        string name = asset.GetProperty("name").GetString()!;
        int separator = name.IndexOf('_');

        return name.StartsWith("MiSide-Zero-", StringComparison.Ordinal) && separator >= 0
            && name[(separator + 1)..].Split('.')[0].Contains(platform, StringComparison.OrdinalIgnoreCase);
    }).ToArray();

    if (matches.Length != 1)
        throw new InvalidOperationException($"Expected one matching asset, found {matches.Length}. Use --list to inspect assets.");

    JsonElement selected = matches[0];
    string filename = selected.GetProperty("name").GetString()!;

    if (Path.GetFileName(filename) != filename || filename.Contains('/') || filename.Contains('\\'))
        throw new InvalidOperationException("Invalid asset filename.");

    Directory.CreateDirectory(directory);
    string destination = Path.Combine(directory, filename);

    Console.WriteLine($"Downloading: {filename}");

    // CreateNew preserves existing downloads. Only this file is removed on failure.
    using (FileStream output = new(destination, FileMode.CreateNew, FileAccess.Write))
    {
        try
        {
            using CancellationTokenSource cancellation = new(TimeSpan.FromMinutes(30));

            using HttpResponseMessage download = await client.GetAsync(selected.GetProperty("browser_download_url").GetString(),
                HttpCompletionOption.ResponseHeadersRead, cancellation.Token);

            download.EnsureSuccessStatusCode();

            await download.Content.CopyToAsync(output, cancellation.Token);
            await output.FlushAsync(cancellation.Token);

            if (output.Position != selected.GetProperty("size").GetInt64())
                throw new IOException("Downloaded size does not match the release asset.");
        }
        catch
        {
            output.Close();
            File.Delete(destination);
            throw;
        }
    }

    Console.WriteLine($"Saved: {Path.GetFullPath(destination)}");
    return 0;
}
catch (Exception error)
{
    Console.Error.WriteLine($"Error: {error.Message}");
    return 1;
}
