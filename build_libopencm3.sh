#!/bin/bash

if [ -z "$1" ]; then
	echo "Missing prefix path."
	echo "Usage: $0 <prefix_path>"
	exit 1
fi


script_dir=$(dirname "$(readlink -e "$0")")

install_prefix=$(readlink -m "$1")
if [ $? -ne 0 ]; then
	exit 1
fi


curr_dir="${PWD}"


mkdir -p "${install_prefix}/lib"
if [ $? -ne 0 ]; then
	exit 1
fi

cd "$(dirname $0)/libsrc/libopencm3"
if [ $? -ne 0 ]; then
	cd "$curr_dir"
	exit 1
fi

make clean
if [ $? -ne 0 ]; then
	cd "$curr_dir"
	exit 1
fi

make
if [ $? -ne 0 ]; then
	cd "$curr_dir"
	exit 1
fi

lib_files=$(find ./lib -name "*.a")

cp -v $lib_files ${install_prefix}/lib
if [ $? -ne 0 ]; then
	cd "$curr_dir"
	exit 1
fi

cp -v -R ./include ${install_prefix}

cd "$curr_dir"
