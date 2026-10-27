#!/usr/bin/env python3
"""
Computes the package version from git tags, so releasing only takes a `vX.Y.Z` tag.

- On a tagged commit: `X.Y.Z`
- N commits after the tag `vX.Y.Z`: `X.Y.(Z+1).devN+g<hash>`
- In an sdist, where there's no git: the version written to `.version` when the sdist was made
- Without git or tags (e.g. a shallow clone): `0.0.0.dev0`

Usage:
    version.py              prints the version
    version.py --write V    writes V to $MESON_DIST_ROOT/.version (used as a meson dist script)
"""

import os
import re
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
VERSION_FILE = ".version"
FALLBACK_VERSION = "0.0.0.dev0"


def version_from_file():
    path = ROOT / VERSION_FILE
    if not path.is_file():
        return None
    return path.read_text().strip()


def version_from_git():
    try:
        description = subprocess.run(
            ["git", "describe", "--tags", "--long", "--match", "v[0-9]*"],
            cwd=str(ROOT),
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            universal_newlines=True,
            check=True,
        ).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return None

    # `git describe --long` gives `<tag>-<commits since tag>-g<hash>`
    match = re.fullmatch(r"v(.+)-(\d+)-g([0-9a-f]+)", description)
    if match is None:
        return None
    tag, distance, commit = match.groups()

    if distance == "0":
        return tag

    release = re.fullmatch(r"(\d+)\.(\d+)\.(\d+)", tag)
    if release is None:
        # A pre-release tag, such as `v1.3.0rc1`
        return f"{tag}.post{distance}.dev0+g{commit}"

    major, minor, patch = release.groups()
    return f"{major}.{minor}.{int(patch) + 1}.dev{distance}+g{commit}"


def main():
    if len(sys.argv) == 3 and sys.argv[1] == "--write":
        Path(os.environ["MESON_DIST_ROOT"], VERSION_FILE).write_text(sys.argv[2] + "\n")
        return

    print(version_from_file() or version_from_git() or FALLBACK_VERSION)


if __name__ == "__main__":
    main()
