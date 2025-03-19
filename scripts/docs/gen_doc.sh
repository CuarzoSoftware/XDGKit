#!/bin/bash
sudo apt install -y doxygen
mkdir -p ../../../xdgkit_tmp
mkdir -p ../../../xdgkit_tmp/html

# Get ENV variables
cd ..
chmod +x env.sh
source env.sh
cd ../doxygen

doxygen Doxyfile

cd ..

