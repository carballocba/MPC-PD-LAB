#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
"$ROOT/scripts/dependencies.sh"
CA=${CODEX_PROXY_CERT:-/etc/ssl/certs/ca-certificates.crt}
DOCKER_ARGS=()
# Managed containers may lack the outer executor's hosts entry for the proxy.
if getent hosts proxy >/dev/null; then
  PROXY_IP=$(getent ahostsv4 proxy | awk 'NR==1 {print $1}')
  DOCKER_ARGS+=(--add-host "proxy:$PROXY_IP")
fi
docker build "${DOCKER_ARGS[@]}" --build-arg HTTP_PROXY --build-arg HTTPS_PROXY \
  --secret "id=proxy_ca,src=$CA" -f "$ROOT/scripts/Dockerfile.armv7" -t mpc-pd-armv7:phase2 "$ROOT"
docker run --rm --network=none -u "$(id -u):$(id -g)" -v "$ROOT:/project" \
  mpc-pd-armv7:phase2 bash scripts/build.sh armv7
