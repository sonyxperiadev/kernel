#!/bin/bash

#
# Build DT blob image when `dtc' tool is available.
#

if test $# -ne 2; then
	echo "Usage: $0 <dts file> <dtb file>" >&2
	exit 1
fi

DTC=${objtree}/scripts/dtc/dtc

if [ -z "${DTC}" ]; then
	# Doesn't exist
	echo '"dtc" command not found - dt-blob image will not be built' >&2
	exit 1;
fi

${DTC} -I dts -O dtb -o $2 $1

exit 0
