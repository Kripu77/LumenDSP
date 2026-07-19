#!/usr/bin/env bash
# Bump the VERSION file (semver). Does not commit or tag.
#
# Usage:
#   ./scripts/bump-version.sh patch
#   ./scripts/bump-version.sh minor
#   ./scripts/bump-version.sh major
#   ./scripts/bump-version.sh 1.2.3
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VERSION_FILE="${ROOT}/VERSION"

if [[ ! -f "${VERSION_FILE}" ]]; then
  echo "error: ${VERSION_FILE} not found" >&2
  exit 1
fi

current="$(tr -d '[:space:]' < "${VERSION_FILE}")"
if [[ ! "${current}" =~ ^([0-9]+)\.([0-9]+)\.([0-9]+)([.-].*)?$ ]]; then
  echo "error: invalid current version '${current}'" >&2
  exit 1
fi

major="${BASH_REMATCH[1]}"
minor="${BASH_REMATCH[2]}"
patch="${BASH_REMATCH[3]}"

arg="${1:-}"
if [[ -z "${arg}" ]]; then
  echo "usage: $0 patch|minor|major|X.Y.Z" >&2
  echo "current: ${current}" >&2
  exit 1
fi

case "${arg}" in
  patch) next="${major}.${minor}.$((patch + 1))" ;;
  minor) next="${major}.$((minor + 1)).0" ;;
  major) next="$((major + 1)).0.0" ;;
  *)
    if [[ "${arg}" =~ ^[0-9]+\.[0-9]+\.[0-9]+([.-].*)?$ ]]; then
      next="${arg}"
    else
      echo "error: expected patch|minor|major|X.Y.Z, got '${arg}'" >&2
      exit 1
    fi
    ;;
esac

printf '%s\n' "${next}" > "${VERSION_FILE}"
echo "VERSION: ${current} → ${next}"
echo "Next: commit VERSION, then: git tag -a v${next} -m \"Release v${next}\" && git push origin v${next}"
