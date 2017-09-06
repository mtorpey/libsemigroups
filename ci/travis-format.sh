#!/bin/bash
set -e

# Install cpplint
sudo apt-get install clang-format-4.0
clang-format --version

# Setup
ci/travis-setup.sh

# clang-format the file
make format

# did anything change?
( ! git diff )
