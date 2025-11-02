#!/usr/bin/env fish
# Release management script for the-impale-game
# Creates a new version tag and pushes it to trigger a release

set -l SCRIPT_DIR (dirname (status --current-filename))

function print_usage
    echo "Usage: ./release.fish <version> [OPTIONS]"
    echo ""
    echo "Arguments:"
    echo "  version       Version number (e.g., 0.1.0, 1.0.0-beta)"
    echo ""
    echo "Options:"
    echo "  --message, -m  Custom tag message (default: 'Release version X.Y.Z')"
    echo "  --dry-run, -n  Show what would be done without actually doing it"
    echo "  --help, -h     Show this help message"
    echo ""
    echo "Examples:"
    echo "  ./release.fish 0.1.0                    # Create v0.1.0 release"
    echo "  ./release.fish 1.0.0 -m 'First stable'  # With custom message"
    echo "  ./release.fish 0.2.0-beta --dry-run     # Preview without executing"
    echo ""
    echo "What this does:"
    echo "  1. Creates an annotated Git tag (v<version>)"
    echo "  2. Pushes the tag to origin"
    echo "  3. Triggers GitHub Actions to:"
    echo "     - Create a release"
    echo "     - Build binaries for all platforms"
    echo "     - Attach binaries to the release"
    echo "     - Deploy web version to GitHub Pages"
end

# Parse arguments
set -l version ""
set -l tag_message ""
set -l dry_run false

if test (count $argv) -eq 0
    print_usage
    exit 1
end

for i in (seq (count $argv))
    switch $argv[$i]
        case --message -m
            set tag_message $argv[(math $i + 1)]
        case --dry-run -n
            set dry_run true
        case --help -h
            print_usage
            exit 0
        case '-*'
            # Skip flags
        case '*'
            if test -z "$version"
                set version $argv[$i]
            end
    end
end

if test -z "$version"
    echo "Error: Version number required"
    print_usage
    exit 1
end

# Add 'v' prefix if not present
if not string match -q 'v*' $version
    set version "v$version"
end

# Set default message if not provided
if test -z "$tag_message"
    set tag_message "Release version $version"
end

cd $SCRIPT_DIR

echo "🏷️  Creating release $version..."

# Check for uncommitted changes
if not git diff-index --quiet HEAD --
    echo "⚠️  Warning: You have uncommitted changes"
    echo ""
    read -l -P "Continue anyway? [y/N] " confirm
    if test "$confirm" != "y" -a "$confirm" != "Y"
        echo "Aborted."
        exit 1
    end
end

# Check if tag already exists
if git rev-parse $version >/dev/null 2>&1
    echo "❌ Error: Tag $version already exists"
    echo ""
    echo "To delete the tag:"
    echo "  git tag -d $version"
    echo "  git push origin :refs/tags/$version"
    exit 1
end

# Show what will be done
echo ""
echo "📋 Release Summary:"
echo "  Version:    $version"
echo "  Message:    $tag_message"
echo "  Tag:        $version"
echo "  Remote:     origin"
echo ""

if test "$dry_run" = true
    echo "🔍 Dry run - would execute:"
    echo "  git tag -a $version -m \"$tag_message\""
    echo "  git push origin $version"
    echo ""
    echo "✅ Dry run complete (no changes made)"
    exit 0
end

# Confirm
read -l -P "Create and push this release? [y/N] " confirm
if test "$confirm" != "y" -a "$confirm" != "Y"
    echo "Aborted."
    exit 1
end

# Create annotated tag
echo ""
echo "📝 Creating tag..."
git tag -a $version -m "$tag_message"

if test $status -ne 0
    echo "❌ Failed to create tag"
    exit 1
end

echo "✅ Tag created locally"

# Push tag
echo ""
echo "🚀 Pushing tag to origin..."
git push origin $version

if test $status -ne 0
    echo "❌ Failed to push tag"
    echo ""
    echo "To remove local tag:"
    echo "  git tag -d $version"
    exit 1
end

echo ""
echo "✅ Release $version created successfully!"
echo ""
echo "🎬 GitHub Actions will now:"
echo "  1. Create a GitHub Release"
echo "  2. Build binaries for Linux, Windows, macOS, and Web"
echo "  3. Attach binaries to the release"
echo "  4. Deploy web version to GitHub Pages"
echo ""
echo "📍 Track progress:"
echo "  Actions: https://github.com/matpdev/the-impale-game/actions"
echo "  Release: https://github.com/matpdev/the-impale-game/releases/tag/$version"
echo ""
echo "⏱️  Build typically takes 5-10 minutes"
