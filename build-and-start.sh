#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${LUMENDSP_BUILD_DIR:-$ROOT/build}"
BUILD_TYPE="${LUMENDSP_BUILD_TYPE:-Release}"
TARGET="${LUMENDSP_TARGET:-LumenDSP_Standalone}"

CLEAN=0
BUILD=1
LAUNCH=1
CONFIGURE=0
JOBS=""

usage() {
  cat <<'EOF'
Build LumenDSP and start the standalone app.

Options:
  -h, --help          Show this help
  -c, --clean         Remove the build directory before configuring
  -j, --jobs N        Parallel build jobs (default: CPU count)
  --build-dir DIR     Build directory (default: ./build)
  --debug             Build Debug instead of Release
  --configure         Force re-run cmake configure
  --build-only        Build but do not launch the app
  --open-only         Launch the last built app without building
  --vst3              Also build the VST3 target (All)

Environment:
  LUMENDSP_BUILD_DIR    Override build directory
  LUMENDSP_BUILD_TYPE   Override build type (Release|Debug)
  LUMENDSP_TARGET       CMake target (default: LumenDSP_Standalone)
EOF
}

log()  { printf '==> %s\n' "$*"; }
die()  { printf 'error: %s\n' "$*" >&2; exit 1; }

default_jobs() {
  if command -v nproc >/dev/null 2>&1; then
    nproc
  elif command -v sysctl >/dev/null 2>&1; then
    sysctl -n hw.ncpu
  else
    echo 4
  fi
}

find_app() {
  local candidates=(
    "$BUILD_DIR/LumenDSP_artefacts/$BUILD_TYPE/Standalone/LumenDSP.app"
    "$BUILD_DIR/LumenDSP_artefacts/Standalone/LumenDSP.app"
    "$BUILD_DIR/LumenDSP_artefacts/Release/Standalone/LumenDSP.app"
    "$BUILD_DIR/LumenDSP_artefacts/Debug/Standalone/LumenDSP.app"
  )
  local c
  for c in "${candidates[@]}"; do
    if [[ -d "$c" ]]; then
      printf '%s\n' "$c"
      return 0
    fi
  done

  local found
  found="$(find "$BUILD_DIR" -maxdepth 6 -type d -name 'LumenDSP.app' 2>/dev/null | head -n 1 || true)"
  if [[ -n "$found" && -d "$found" ]]; then
    printf '%s\n' "$found"
    return 0
  fi
  return 1
}

sync_web_into_app() {
  local app="$1"
  local web_src="$ROOT/Resources/web"
  local web_dst="$app/Contents/Resources/web"
  if [[ -d "$web_src" && -d "$app/Contents/Resources" ]]; then
    log "Syncing Resources/web into app bundle"
    mkdir -p "$web_dst"
    if command -v rsync >/dev/null 2>&1; then
      rsync -a --delete "$web_src/" "$web_dst/"
    else
      rm -rf "$web_dst"
      mkdir -p "$web_dst"
      cp -R "$web_src/." "$web_dst/"
    fi
  fi
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -h|--help)
      usage
      exit 0
      ;;
    -c|--clean)
      CLEAN=1
      CONFIGURE=1
      shift
      ;;
    -j|--jobs)
      JOBS="${2:-}"
      [[ -n "$JOBS" ]] || die "--jobs requires a number"
      shift 2
      ;;
    --build-dir)
      BUILD_DIR="${2:-}"
      [[ -n "$BUILD_DIR" ]] || die "--build-dir requires a path"
      shift 2
      ;;
    --debug)
      BUILD_TYPE="Debug"
      shift
      ;;
    --configure)
      CONFIGURE=1
      shift
      ;;
    --build-only)
      LAUNCH=0
      shift
      ;;
    --open-only)
      BUILD=0
      LAUNCH=1
      shift
      ;;
    --vst3)
      TARGET="LumenDSP_All"
      shift
      ;;
    *)
      die "unknown option: $1 (try --help)"
      ;;
  esac
done

JOBS="${JOBS:-$(default_jobs)}"

command -v cmake >/dev/null 2>&1 || die "cmake not found (need CMake 3.24+)"
command -v git >/dev/null 2>&1 || die "git not found (required to fetch JUCE / NAM)"

cd "$ROOT"

if [[ "$CLEAN" -eq 1 ]]; then
  log "Cleaning $BUILD_DIR"
  rm -rf "$BUILD_DIR"
fi

need_configure=0
if [[ "$CONFIGURE" -eq 1 || ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
  need_configure=1
fi

if [[ "$BUILD" -eq 1 ]]; then
  if [[ "$need_configure" -eq 1 ]]; then
    log "Configuring ($BUILD_TYPE) → $BUILD_DIR"
    cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
  else
    log "Using existing configure in $BUILD_DIR"
  fi

  log "Building target $TARGET ($BUILD_TYPE, -j$JOBS)"
  cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" --target "$TARGET" -j"$JOBS"
  log "Build finished"
fi

APP=""
if ! APP="$(find_app)"; then
  if [[ "$LAUNCH" -eq 1 ]]; then
    die "could not find LumenDSP.app under $BUILD_DIR — build first"
  fi
  log "No app bundle found (build-only without artefacts path)"
  exit 0
fi

log "App: $APP"

if [[ "$BUILD" -eq 1 ]]; then
  sync_web_into_app "$APP"
fi

if [[ "$LAUNCH" -eq 1 ]]; then
  if pgrep -x LumenDSP >/dev/null 2>&1; then
    log "Quitting running LumenDSP"
    if command -v osascript >/dev/null 2>&1; then
      osascript -e 'tell application "LumenDSP" to quit' 2>/dev/null || true
    fi
    pkill -x LumenDSP 2>/dev/null || true
    sleep 0.4
  fi

  log "Launching standalone"
  if [[ "$(uname -s)" == "Darwin" ]]; then
    open "$APP"
  else
    bin="$APP/Contents/MacOS/LumenDSP"
    if [[ -x "$bin" ]]; then
      "$bin" &
    elif [[ -x "$BUILD_DIR/LumenDSP_artefacts/$BUILD_TYPE/Standalone/LumenDSP" ]]; then
      "$BUILD_DIR/LumenDSP_artefacts/$BUILD_TYPE/Standalone/LumenDSP" &
    else
      die "do not know how to launch on this platform — open: $APP"
    fi
  fi
  log "Done"
else
  log "Skipping launch (--build-only). App at: $APP"
fi
