"""Download one MSZ release asset using only Python's standard library."""

import argparse
import json
from pathlib import Path
import shutil
import sys
from urllib.parse import quote
from urllib.request import Request, urlopen


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("platform", help="win, linux, ios, mac, android, or --list")
    parser.add_argument("tag", nargs="?", default="latest")
    parser.add_argument("directory", nargs="?", default=".")
    # Treat --list as the platform argument, keeping the same CLI in all examples.
    arguments = sys.argv[1:]
    if arguments[:1] == ["--list"]:
        arguments[0] = "list"
    args = parser.parse_args(arguments)
    args.platform = args.platform.lower()
    if args.platform.lower() not in ("list", "win", "linux", "ios", "mac", "android"):
        parser.error("platform must be win, linux, ios, mac, or android")

    endpoint = "latest" if args.tag == "latest" else "tags/" + quote(args.tag, safe="")
    url = "https://api.github.com/repos/Gameknight963/MSZVersionArchive/releases/" + endpoint
    headers = {"User-Agent": "MSZVersionArchive-example", "Accept": "application/vnd.github+json"}
    with urlopen(Request(url, headers=headers), timeout=30) as response:
        release = json.load(response)
    print("Release:", release["tag_name"])
    assets = release["assets"]
    if args.platform == "list":
        for asset in assets:
            print(asset["name"])
        return

    # Match only the platform segment, never the version or file extension.
    # Tags and filename versions differ in this archive (including the mono build).
    matches = [a for a in assets if a["name"].startswith("MiSide-Zero-")
               and "_" in a["name"]
               and args.platform in a["name"].split("_", 1)[1].split(".", 1)[0].lower()]
    if len(matches) != 1:
        raise ValueError(f"Expected one matching asset, found {len(matches)}. Use --list to inspect assets.")
    asset = matches[0]
    name = asset["name"]
    if Path(name).name != name or "/" in name or "\\" in name:
        raise ValueError("Invalid asset filename")
    directory = Path(args.directory)
    directory.mkdir(parents=True, exist_ok=True)
    destination = directory / name
    print("Downloading:", name, flush=True)
    # Exclusive creation preserves existing downloads. Only this file is removed on failure.
    with destination.open("xb") as output:
        try:
            request = Request(asset["browser_download_url"], headers={"User-Agent": headers["User-Agent"]})
            with urlopen(request, timeout=60) as response:
                shutil.copyfileobj(response, output, length=1024 * 1024)
            output.flush()
            if output.tell() != asset["size"]:
                raise IOError("Downloaded size does not match the release asset")
        except BaseException:
            output.close()
            destination.unlink()
            raise
    print("Saved:", destination.resolve())


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError) as error:
        sys.exit(f"Error: {error}")
