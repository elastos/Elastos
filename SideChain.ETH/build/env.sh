#!/bin/sh

set -e

if [ ! -f "build/env.sh" ]; then
    echo "$0 must be run from the root of the repository."
    exit 2
fi

# Create fake Go workspace if it doesn't exist yet.
workspace="$PWD/build/_workspace"
root="$PWD"
ethdir="$workspace/src/github.com/elastos"
if [ ! -L "$ethdir/Elastos.ELA.SideChain.ETH" ]; then
    mkdir -p "$ethdir"
    cd "$ethdir"
    ln -s ../../../../../. Elastos.ELA.SideChain.ETH
    cd "$root"
fi

# Set up the environment to use the workspace.
GOPATH="$workspace"

# Run the command inside the workspace.
cd "$ethdir/Elastos.ELA.SideChain.ETH"
PWD="$ethdir/Elastos.ELA.SideChain.ETH"

# Launch the arguments with the configured environment.
exec "$@"
