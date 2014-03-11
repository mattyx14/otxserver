#!/bin/bash
# CCache + multicore compilation script
# For "The OTX Server"

# Example:
# # make clean
# # time ./build.sh

# Gives:
# real	3m27.070s
# user	6m4.066s
# sys	0m16.659s

# CCACHE recompile (from scratch):
# # make clean
# # time ./build.sh

# Gives:
# real	0m27.620s
# user	0m43.744s
# sys	0m4.766s

# 1/7 of the original compile time!
# When more you do it, more ccache will cache, default is limited to use 1GB storage

echo "The OTX Server build script - "

# Enable CCache
if test -x `which ccache`; then
	echo "Using ccache"
	if [ -f /usr/lib/ccache/bin ]; then
		export PATH=/usr/lib/ccache/bin/:$PATH
		echo "CCache binaries located in /usr/lib/ccache/bin"
	else
		export PATH=/usr/lib/ccache/:$PATH
		echo "CCache binaries located in /usr/lib/ccache"
	fi
else
	echo "Warning: NOT using ccache, increase build time"
fi

# Get number cores
CORES=`grep processor /proc/cpuinfo | wc -l`
# Set make processes - 1 + number of cores
MAKEOPT=$(($CORES + 1))
echo ""
echo "Start building on $CORES cores, using $MAKEOPT processes"
echo ""
make V=0 -j $MAKEOPT
