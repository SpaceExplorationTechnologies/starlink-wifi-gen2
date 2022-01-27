#!/bin/bash -ex

# Updates desired git repository with a full copy of the open source components
# of our Wifi Router build.
if [[ "$1" == "" ]]; then
    set +x
    echo
    echo "####################################################################################################"
    echo ""
    echo "Usage:"
    echo "  ./publish-open-source.sh <path/to/open/source/repo>"
    echo ""
    echo "####################################################################################################"
    exit 1
fi
OPEN_SOURCE_REPO="$(realpath "$1")"
CURR_BRANCH="$(git --git-dir="$OPEN_SOURCE_REPO"/.git rev-parse --abbrev-ref HEAD)"
if [[ $CURR_BRANCH == "master" || $CURR_BRANCH == "main" ]]; then
    set +x
    echo "Error: won't overwrite master/main, please set $OPEN_SOURCE_REPO to a development branch!"
    exit 1
fi

cd "$(dirname "$0")"

rsync -av --delete --exclude='.git' --filter=':- .gitignore' \
    --exclude-from='publish-exclude.txt' ../ $OPEN_SOURCE_REPO

DATE="$(date +%Y-%m-%d)"
COMMIT="$(git rev-parse HEAD)"

cd "$OPEN_SOURCE_REPO"
git add --all .
git commit -m "$DATE: Update to upstream $COMMIT."
