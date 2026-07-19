# Releasing LumenDSP

This document is the **source of truth** for version numbers, git tags, and how GitHub Actions ships installable apps.

## Semantic versioning (semver)

We use [SemVer 2.0](https://semver.org/) while pre-1.0:

```
MAJOR.MINOR.PATCH
```

| Component | When to bump | Example |
|-----------|----------------|---------|
| **MAJOR** | Breaking changes (preset format, signal path, public API) | `0.x` → `1.0.0` when we call it stable; later `1.x` → `2.0.0` |
| **MINOR** | New features, backwards-compatible | `0.1.0` → `0.2.0` (new FX, library features) |
| **PATCH** | Bug fixes, polish, docs only | `0.2.0` → `0.2.1` |

**Pre-1.0 policy (`0.y.z`):** the product is still evolving. Breaking changes may happen in **MINOR** bumps if unavoidable; prefer documenting them in the release notes. After **1.0.0**, breaking changes require a MAJOR bump.

### Pre-releases (optional)

Tags may use a suffix:

| Tag | Meaning |
|-----|---------|
| `v0.2.0-rc.1` | Release candidate |
| `v0.2.0-beta.1` | Beta |

GitHub marks these as **pre-release**. The `VERSION` file should still hold the base `0.2.0` (no suffix) for the app’s `CFBundleShortVersionString` / product version, **or** you may set `VERSION` to `0.2.0-rc.1` if you want the binary to show that string. Prefer base version in `VERSION` and suffix only on the git tag.

---

## Source of truth

| Item | Location |
|------|----------|
| App / CMake version | **`VERSION`** file at repo root (single line, e.g. `0.1.0`) |
| Git tag | **`v` + VERSION**, e.g. `v0.1.0` |
| CMake `project(... VERSION ...)` | Read automatically from `VERSION` |
| GitHub Release title | `LumenDSP v0.1.0` |

**Rule:** for a stable release, the tag `vX.Y.Z` **must** match the contents of `VERSION` (`X.Y.Z`). The release workflow fails if they disagree.

---

## How a release is created

### Recommended: tag a release (explicit)

```bash
# 1. Bump version (from repo root)
./scripts/bump-version.sh patch   # or: minor | major | 0.3.0

# 2. Commit the VERSION change with your feature/fix PR (or a chore PR)
git add VERSION
git commit -m "chore(release): bump version to $(cat VERSION)"
git push origin main

# 3. Tag and push the tag — this starts Release workflow
git tag -a "v$(cat VERSION)" -m "Release v$(cat VERSION)"
git push origin "v$(cat VERSION)"
```

What happens next:

1. **Release** workflow builds on `macos-14` (arm64) and `windows-latest` (x64).
2. Packages:
   - `LumenDSP-vX.Y.Z-macos-arm64.zip` — `LumenDSP.app` + `LumenDSP.vst3`
   - `LumenDSP-vX.Y.Z-windows-x64.zip` — `LumenDSP.exe` + `FactoryContent` + `web` (+ VST3 if built)
3. Creates a **GitHub Release** on that tag with both zips attached.
4. Uploads **stable asset names** so README links never break:
   - `LumenDSP-macos-arm64.zip`
   - `LumenDSP-windows-x64.zip`
5. Rewrites the README **Download** block (between `BEGIN_LATEST_RELEASE` markers) and pushes to `main`.

### Manual: Actions UI

1. GitHub → **Actions** → **Release** → **Run workflow**
2. Optionally enter a version (`0.2.1`); leave empty to use `VERSION`
3. Optionally mark pre-release
4. Workflow may create the tag and publish the release

### Optional: auto-release when `VERSION` changes on `main`

If you want a merge to main that only bumps `VERSION` to publish automatically:

1. Repo → **Settings** → **Secrets and variables** → **Actions** → **Variables**
2. Add `RELEASE_ON_VERSION_BUMP` = `true`

Then:

- Merge a PR that updates `VERSION`
- On push to `main`, Release workflow tags `v$(cat VERSION)` and publishes

**Default is off** so ordinary feature merges do not spam releases.

### What does **not** create a release

| Event | Behaviour |
|-------|-----------|
| Open / update a PR | **CI** builds only (no release) |
| Push to `main` without tag / VERSION bump | **CI** builds only |
| Push to `main` with VERSION bump and auto flag off | **CI** only; you still tag manually |

---

## Installable artefacts

| Platform | Runner | Contents |
|----------|--------|----------|
| macOS Apple Silicon | `macos-14` | Standalone `.app` (ad-hoc signed), VST3, `INSTALL.txt` |
| Windows x64 | `windows-latest` | Standalone `.exe` + resource folders, optional VST3, `INSTALL.txt` |

**Not included yet:** notarized Developer ID signing, Intel macOS universal binary, Linux, AU/CLAP installers. Those can be added later with certificates and extra matrix jobs.

### Gatekeeper (macOS)

CI builds are **not notarized**. Users may need:

**Right-click → Open** the first time, or System Settings → Privacy & Security → Open Anyway.

---

## Checklist before tagging

- [ ] `VERSION` bumped correctly (patch / minor / major)
- [ ] App runs locally (standalone smoke test)
- [ ] Factory presets / critical fixes verified
- [ ] `README` / `QUICKSTART` still accurate if UX changed
- [ ] Tag message is clear: `Release vX.Y.Z`
- [ ] Tag pushed: `git push origin vX.Y.Z`

---

## Version history convention

Prefer Conventional Commits in history so release notes stay readable:

| Prefix | Use |
|--------|-----|
| `feat:` | New user-facing feature → usually **minor** |
| `fix:` | Bug fix → usually **patch** |
| `perf:` | Performance → **patch** (or minor if large) |
| `docs:` | Docs only → **patch** or no release |
| `chore:` | Build / CI / deps → often no product release |
| `BREAKING CHANGE:` footer | **major** (or minor under 0.x with a loud note) |

---

## Troubleshooting CI

| Problem | Fix |
|---------|-----|
| Tag rejected: version mismatch | Align `VERSION` with tag, retag |
| macOS build fails fetching JUCE | Re-run job; check network / tag `LUMENDSP_JUCE_GIT_TAG` |
| Windows MSVC errors | Ensure Ninja + MSVC job uses `ilammy/msvc-dev-cmd` |
| Release empty / no assets | Check `build` job logs; artefact names must match `publish` download pattern |
| Permission denied creating release | Workflow needs `permissions: contents: write` (already set) |

---

## Quick reference

```text
VERSION file  ──►  CMake project version  ──►  app bundle version
       │
       └──►  git tag vX.Y.Z  ──►  GitHub Actions Release  ──►  .zip installers
```

**Ship a release:** bump `VERSION` → commit → `git tag v…` → `git push origin v…`
