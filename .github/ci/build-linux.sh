#!/usr/bin/env bash
# Linux build check, run by .github/workflows/linux-build.yml inside a clean Ubuntu container (it can also be
# run by hand in one, from the repository root): installs what sources/Makefile links and builds the server.
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive
export CCACHE_DIR="${CCACHE_DIR:-$PWD/.ccache}" # the workflow caches .ccache between builds
apt-get update -qq
apt-get install -y -qq --no-install-recommends \
	build-essential pkg-config ccache \
	libboost-dev libboost-filesystem-dev libboost-regex-dev libboost-system-dev libboost-thread-dev \
	libboost-iostreams-dev libgmp3-dev libxml2-dev libsqlite3-dev libgoogle-perftools-dev liblua5.1-0-dev \
	libmysqlclient-dev libssl-dev > /dev/null

. /etc/os-release
echo "${PRETTY_NAME} | $(g++ --version | head -1) | Boost $(grep 'define BOOST_LIB_VERSION' /usr/include/boost/version.hpp | cut -d'"' -f2)"

cd sources
make -j"$(nproc)" CXX="ccache g++"
ls -l theotxserver
