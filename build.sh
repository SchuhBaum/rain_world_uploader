#!/bin/bash

# I found a symlink to it inside a subfolder of sdk64 as well. I could have just
# copied that.
# mkdir -p $HOME/.steam/sdk64
# ln -s $HOME/snap/steam/common/.local/share/Steam/linux64/steamclient.so $HOME/.steam/sdk64/steamclient.so

prev_wd="$(pwd)"

cur_wd_relative="$(dirname "${BASH_SOURCE[0]}")"
cur_wd="$(cd $cur_wd_relative && pwd)"

input="$cur_wd/main.cpp"
output="-o $cur_wd/rain_world_uploader/rain_world_uploader.out"

# You need to download the Steamworks Sdk yourself.
common_args="-std=c++17 -I$cur_wd/steamworks_sdk/public/steam -L$cur_wd/lib -Wl,-rpath=\$ORIGIN"
common_libs="-lsteam_api -static-libstdc++ -static-libgcc"

if [ "$1" == "r" ]; then
    g++ -w -Ofast \
    $common_args $input $output $common_libs
elif [ "$1" == "d" ]; then
    g++ -Wall -Wextra -ggdb3 \
    $common_args $input $output $common_libs
else
    g++ -w -ggdb3 \
    $common_args $input $output $common_libs
fi

# ctags -R -o $cur_wd/tags $cur_wd/*

cd "$prev_wd"
