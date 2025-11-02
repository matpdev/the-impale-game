# Release Guide

This guide explains how to create releases for the Impale game using Git tags.

## Quick Start

### Using the Release Script (Recommended)

```fish
# Create a new release
./release.fish 0.1.0

# With custom message
./release.fish 1.0.0 -m "First stable release"

# Preview without executing
./release.fish 0.2.0 --dry-run
```

### Manual Release

```fish
# 1. Create annotated tag
git tag -a v0.1.0 -m "Release version 0.1.0"

# 2. Push tag to GitHub
git push origin v0.1.0
```

## What Happens Automatically

When you push a tag (starting with `v`):

1. **`release.yml`** creates a GitHub Release
2. **`build.yml`** builds binaries for all platforms:
   - Linux (x86_64)
   - Windows (x64)
   - macOS (x86_64)
   - Web (WASM)
3. **Binaries are attached** to the release automatically
4. **Web version deploys** to GitHub Pages

## Version Numbering

Follow [Semantic Versioning](https://semver.org/):

- **Major.Minor.Patch** (e.g., `1.0.0`)
- **Pre-release**: Add suffix (e.g., `0.1.0-beta`, `1.0.0-rc1`)

Examples:
```fish
./release.fish 0.1.0        # First alpha
./release.fish 0.2.0-beta   # Beta version
./release.fish 1.0.0-rc1    # Release candidate
./release.fish 1.0.0        # Stable release
./release.fish 1.1.0        # Feature update
./release.fish 1.1.1        # Bug fix
```

## Workflow Timeline

```
Push tag → Release created (30s)
         → Builds start (1-2 min)
         → Binaries attached (5-10 min total)
         → Pages deployed (if master)
```

## Viewing Releases

- **All releases**: https://github.com/matpdev/the-impale-game/releases
- **Latest release**: https://github.com/matpdev/the-impale-game/releases/latest
- **Specific release**: https://github.com/matpdev/the-impale-game/releases/tag/v0.1.0

## Editing Release Notes

After the release is created:

1. Go to **Releases** page
2. Click **Edit** on your release
3. Update the description with:
   - New features
   - Bug fixes
   - Breaking changes
   - Known issues
4. Save changes

## Deleting a Release

### Delete Release (keep tag)
```fish
# Use GitHub UI: Releases → Delete release
# Or use GitHub CLI:
gh release delete v0.1.0
```

### Delete Tag and Release
```fish
# Delete local tag
git tag -d v0.1.0

# Delete remote tag
git push origin :refs/tags/v0.1.0

# Delete release (GitHub UI or CLI)
gh release delete v0.1.0
```

## Pre-releases

Mark releases as pre-release in GitHub UI:

1. Create release as normal
2. Check "Set as a pre-release" before publishing
3. Won't show as "latest" release

Or automate with tag pattern:
```yaml
# In release.yml
prerelease: ${{ contains(github.ref, 'beta') || contains(github.ref, 'alpha') || contains(github.ref, 'rc') }}
```

## Troubleshooting

### Release Created But No Binaries

**Cause**: `build.yml` workflow didn't run or failed

**Solution**:
1. Check **Actions** tab for workflow status
2. Verify `build.yml` has `release: types: [created]` trigger
3. Re-run failed jobs if needed

### Tag Exists Error

**Cause**: Tag already exists locally or remotely

**Solution**:
```fish
# Delete local tag
git tag -d v0.1.0

# Delete remote tag
git push origin :refs/tags/v0.1.0

# Try again
./release.fish 0.1.0
```

### Wrong Version Tagged

**Cause**: Typo in version number

**Solution**:
```fish
# Delete the wrong tag
git tag -d v0.1.0
git push origin :refs/tags/v0.1.0

# Delete the release (GitHub UI)
# Create new tag with correct version
./release.fish 0.1.1
```

## Best Practices

1. **Test before releasing**
   ```fish
   # Build and test locally first
   ./build.fish native --run
   ./build.fish web
   ```

2. **Update version in README/docs** if needed

3. **Write meaningful release notes**
   - What changed?
   - Why should users upgrade?
   - Any breaking changes?

4. **Use semantic versioning**
   - Increment MAJOR for breaking changes
   - Increment MINOR for new features
   - Increment PATCH for bug fixes

5. **Tag from master/main**
   ```fish
   git checkout master
   git pull origin master
   ./release.fish 1.0.0
   ```

## GitHub CLI (Optional)

Install GitHub CLI for more control:

```fish
# Install (Arch Linux)
yay -S github-cli

# Create release manually
gh release create v0.1.0 \
  --title "Release 0.1.0" \
  --notes "Release notes here"

# List releases
gh release list

# View release
gh release view v0.1.0
```

## Example Release Workflow

```fish
# 1. Ensure you're on master and up to date
git checkout master
git pull origin master

# 2. Build and test locally
./build.fish native --run
./build.fish web

# 3. Commit any final changes
git add .
git commit -m "Prepare for v0.1.0 release"
git push origin master

# 4. Create release
./release.fish 0.1.0

# 5. Wait for builds (monitor in Actions tab)

# 6. Edit release notes on GitHub

# 7. Announce release!
```

---

**Need help?** Check [GitHub Releases documentation](https://docs.github.com/en/repositories/releasing-projects-on-github) or open an issue.
