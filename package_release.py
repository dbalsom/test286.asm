# SPDX-License-Identifier: GPL-3.0-or-later
"""Package a built test286 ROM and its documentation into a dated release ZIP."""

import argparse
from datetime import date
import hashlib
from pathlib import Path
import re
from zipfile import ZIP_DEFLATED, ZipFile

ROOT = Path(__file__).resolve().parent

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, default=ROOT / "output",
                        help="Directory containing test286.bin and test286.lst")
    parser.add_argument("--output-dir", type=Path, default=ROOT / "output",
                        help="Directory for the release ZIP")
    args = parser.parse_args()

    try:
        # Our file manifest
        sources = [args.build_dir / "test286.bin", args.build_dir / "test286.lst"]
        sources.extend(ROOT / name for name in ("README.md", "COPYING", "CHANGELOG.md"))
        contents = {path.name: path.read_bytes() for path in sources}
        
        # Size sanity check (must be 64KiB exact)
        if len(contents["test286.bin"]) != 65536:
            raise ValueError("test286.bin must be exactly 65,536 bytes")

        # Get ROM version directly from ROM header        
        header = re.match(rb"test286\.asm v([0-9]+\.[0-9]+\.[0-9]+)\x00",
                          contents["test286.bin"])
        if header is None:
            raise ValueError("test286.bin has no valid version header")
        version = header.group(1).decode("ascii")

        # Split alternating byte addresses into two 32 KiB ROM images.
        contents["test286_even.bin"] = contents["test286.bin"][0::2]
        contents["test286_odd.bin"] = contents["test286.bin"][1::2]

        # Include a sha256sum-compatible checksum file for each ROM image.
        digests = {}
        for name in ("test286.bin", "test286_even.bin", "test286_odd.bin"):
            digest = hashlib.sha256(contents[name]).hexdigest()
            digests[name] = digest
            contents[f"{name}.sha256"] = f"{digest}  {name}\n".encode("ascii")

        # Create output dir
        args.output_dir.mkdir(parents=True, exist_ok=True)

        # Create output zip
        archive = args.output_dir / f"test286_v{version}_{date.today():%Y_%m_%d}.zip"
        with ZipFile(archive, "w", compression=ZIP_DEFLATED, compresslevel=9) as release:
            for name, data in contents.items():
                release.writestr(name, data)
    
    except (OSError, ValueError) as error:
        parser.exit(1, f"Error: {error}\n")

    print(archive.resolve())
    for name, digest in digests.items():
        print(f"SHA-256: {digest}  {name}")
    
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
