#!/usr/bin/env bash
# Rewrite the auto-managed Download block in README.md for a release tag.
#
# Usage:
#   ./scripts/update-readme-release.sh v0.1.0
#   ./scripts/update-readme-release.sh 0.1.0
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
README="${ROOT}/README.md"
TAG_RAW="${1:-}"

if [[ -z "${TAG_RAW}" ]]; then
  if [[ -f "${ROOT}/VERSION" ]]; then
    TAG_RAW="$(tr -d '[:space:]' < "${ROOT}/VERSION")"
  else
    echo "usage: $0 vX.Y.Z" >&2
    exit 1
  fi
fi

VERSION="${TAG_RAW#v}"
TAG="v${VERSION}"
REPO="${GITHUB_REPOSITORY:-Kripu77/LumenDSP}"
BASE="https://github.com/${REPO}/releases"
LATEST="${BASE}/latest"
MAC_STABLE="${LATEST}/download/LumenDSP-macos-arm64.zip"
WIN_STABLE="${LATEST}/download/LumenDSP-windows-x64.zip"
MAC_VER="${BASE}/download/${TAG}/LumenDSP-${TAG}-macos-arm64.zip"
WIN_VER="${BASE}/download/${TAG}/LumenDSP-${TAG}-windows-x64.zip"
BADGE="https://img.shields.io/github/v/release/${REPO}?label=latest&style=flat-square"

if [[ ! -f "${README}" ]]; then
  echo "error: ${README} not found" >&2
  exit 1
fi

BLOCK="$(cat <<EOF
<!-- BEGIN_LATEST_RELEASE -->
## Download

[![Latest release](${BADGE})](${LATEST})

**Latest release: [\`${TAG}\`](${LATEST})**

| Platform | Download |
|----------|----------|
| **macOS** (Apple Silicon) | [**LumenDSP-macos-arm64.zip**](${MAC_STABLE}) |
| **Windows** (x64) | [**LumenDSP-windows-x64.zip**](${WIN_STABLE}) |

Same files for this tag (pinned): [macOS ${TAG}](${MAC_VER}) · [Windows ${TAG}](${WIN_VER})

**Install notes**
- **macOS:** unzip → open \`LumenDSP.app\` (right-click → **Open** if Gatekeeper warns). Optional: copy \`LumenDSP.vst3\` to \`~/Library/Audio/Plug-Ins/VST3/\`.
- **Windows:** unzip → run \`LumenDSP.exe\` (keep \`FactoryContent/\` and \`web/\` next to the exe).
- Builds are not notarized. Allow Microphone / Bluetooth when prompted so audio devices appear.

All releases: ${BASE}
<!-- END_LATEST_RELEASE -->
EOF
)"

export LUMENDSP_README="${README}"
export LUMENDSP_README_BLOCK="${BLOCK}"

python3 <<'PY'
import os
import pathlib
import re

readme_path = pathlib.Path(os.environ["LUMENDSP_README"])
block = os.environ["LUMENDSP_README_BLOCK"].strip() + "\n"
text = readme_path.read_text(encoding="utf-8")
pattern = re.compile(
    r"<!-- BEGIN_LATEST_RELEASE -->.*?<!-- END_LATEST_RELEASE -->\n?",
    re.DOTALL,
)
if pattern.search(text):
    text = pattern.sub(block, text, count=1)
else:
    lines = text.splitlines(keepends=True)
    if not lines or not lines[0].startswith("# "):
        text = block + "\n" + text
    else:
        insert_at = 1
        while insert_at < len(lines) and lines[insert_at].strip() == "":
            insert_at += 1
        while insert_at < len(lines) and lines[insert_at].strip() != "":
            insert_at += 1
        while insert_at < len(lines) and lines[insert_at].strip() == "":
            insert_at += 1
        lines.insert(insert_at, "\n" + block + "\n")
        text = "".join(lines)

readme_path.write_text(text, encoding="utf-8")
print(f"Updated {readme_path}")
PY
