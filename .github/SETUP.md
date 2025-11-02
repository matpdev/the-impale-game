# GitHub Actions & Pages Setup

This document explains how to set up GitHub Actions and GitHub Pages for the Impale game.

## Prerequisites

- Repository hosted on GitHub
- Admin access to repository settings

## GitHub Actions Setup

The workflows are already configured in `.github/workflows/`:
- `build.yml` - Builds for all platforms
- `pages.yml` - Deploys web version to GitHub Pages

**No additional setup required** - workflows will run automatically on push.

## GitHub Pages Setup

To enable GitHub Pages deployment:

### 1. Enable GitHub Pages

1. Go to repository **Settings**
2. Navigate to **Pages** (left sidebar)
3. Under **Build and deployment**:
   - **Source**: Select "GitHub Actions"
   - **Branch**: No need to select (handled by workflow)
4. Click **Save**

### 2. Verify Deployment

1. Push to master/main branch
2. Go to **Actions** tab
3. Wait for "Deploy to GitHub Pages" workflow to complete
4. Your game will be available at: `https://<username>.github.io/<repository>/`

For this repository: `https://matpdev.github.io/the-impale-game/`

## Workflows Explained

### build.yml

Runs on:
- Push to master/main
- Pull requests to master/main
- Release creation

Jobs:
- **build-linux**: Builds native Linux x86_64 binary
- **build-windows**: Builds native Windows x64 binary
- **build-macos**: Builds native macOS x86_64 binary
- **build-web**: Builds WebAssembly version

Artifacts:
- Uploaded to workflow run
- Attached to releases (if triggered by release)

### pages.yml

Runs on:
- Push to master/main
- Manual trigger (workflow_dispatch)

Jobs:
- **build**: Compiles WASM version
- **deploy**: Deploys to GitHub Pages

Features:
- Creates `index.html` redirect to game
- Uploads entire build directory
- Deploys to `gh-pages` environment

## Creating a Release

To create a release with downloadable binaries:

1. Go to **Releases** → **Draft a new release**
2. Create a new tag (e.g., `v0.1.0`)
3. Fill in release notes
4. Click **Publish release**
5. GitHub Actions will automatically build and attach binaries

## Troubleshooting

### Pages Not Deploying

**Issue**: Workflow succeeds but page doesn't update

**Solution**:
1. Check **Settings** → **Pages** → Source is "GitHub Actions"
2. Verify workflow permissions: **Settings** → **Actions** → **General**
   - Workflow permissions: "Read and write permissions"
   - Allow GitHub Actions to create PRs: Enabled

### Build Failures

**Issue**: Build workflow fails

**Solution**:
1. Check **Actions** tab for error details
2. Common issues:
   - Missing Emscripten (web build): Verify `setup-emsdk` action runs
   - xmake configuration errors: Check xmake version compatibility
   - Package installation: Ensure dependencies are available

### Artifacts Not Uploading

**Issue**: Release doesn't have attached files

**Solution**:
1. Ensure release trigger: workflow must run on `release` event
2. Check `GITHUB_TOKEN` permissions in workflow
3. Verify `softprops/action-gh-release` step completes

## Advanced Configuration

### Custom Domain

To use a custom domain for GitHub Pages:

1. Add `CNAME` file to `build/wasm/release/` before deployment
2. Configure DNS records at your domain provider
3. Update repository settings with custom domain

### Build Optimization

To optimize build times:

1. **Cache dependencies**: Add caching for xmake packages
2. **Matrix builds**: Parallelize platform builds
3. **Conditional workflows**: Skip web build on docs-only changes

Example cache step:
```yaml
- name: Cache xmake packages
  uses: actions/cache@v3
  with:
    path: ~/.xmake
    key: ${{ runner.os }}-xmake-${{ hashFiles('xmake.lua') }}
```

## Monitoring

- **Build Status**: Check badges in README.md
- **Deployment Status**: View in Actions tab
- **Pages Health**: Settings → Pages shows deployment status
- **Workflow Logs**: Detailed logs available in Actions tab

## Security

The workflows use:
- `GITHUB_TOKEN`: Automatically provided, scoped to repository
- `actions/checkout@v4`: Verified official action
- `xmake-io/github-action-setup-xmake@v1`: Official xmake action
- `mymindstorm/setup-emsdk@v14`: Popular Emscripten setup action

No secrets or personal tokens required for basic functionality.

---

**Need help?** Check [GitHub Actions documentation](https://docs.github.com/en/actions) or open an issue.
