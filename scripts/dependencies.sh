#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
DEPS=${DEPS_DIR:-$ROOT/work/deps}
mkdir -p "$DEPS"
fetch() {
  local name=$1 url=$2 pin=$3
  if [[ ! -d "$DEPS/$name/.git" ]]; then
    git clone "$url" "$DEPS/$name"
    git -C "$DEPS/$name" checkout --detach "$pin"
  fi
  [[ $(git -C "$DEPS/$name" rev-parse HEAD) == "$pin" ]] || { echo "Wrong $name revision" >&2; exit 1; }
  [[ -z $(git -C "$DEPS/$name" status --porcelain --untracked-files=no) ]] || { echo "Dirty $name sources" >&2; exit 1; }
}
fetch libpd https://github.com/libpd/libpd.git ba0dc63262901d658af8bbda5e619a60fa975e78
fetch mpc-vst-plugins https://github.com/sd88me/mpc-vst-plugins.git 208745830fc2ed90b2fd0dc480ef5c554284292f
git -C "$DEPS/libpd" submodule update --init --recursive
[[ $(git -C "$DEPS/libpd/pure-data" rev-parse HEAD) == f009fd8d7b537e209e09898d487fdf1bf547da2b ]]
