# This file must be used with "source vars.sh"
if [ "${BASH_SOURCE-}" = "$0" ]; then
  echo "You must 'source' this script: source $0" >&2
  exit 1
fi
if [ "$MKXP_PREFIX" ]; then
  echo "Already done" >&2
  return
fi

MKXP_ENVDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
MKXP_ARCH="$(uname -m)"

export MKXP_PREFIX="${MKXP_ENVDIR}/build-${MKXP_ARCH}"
export PKG_CONFIG_PATH="${MKXP_PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH}"
export CMAKE_PREFIX_PATH="${MKXP_PREFIX}:${CMAKE_PREFIX_PATH}"

PATH="${MKXP_PREFIX}/bin:${PATH}"
LD_LIBRARY_PATH="${MKXP_PREFIX}/lib:${LD_LIBRARY_PATH}"
