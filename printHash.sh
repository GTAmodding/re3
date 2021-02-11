#!/usr/bin/bash

> version.h

echo -n "char headCommitHash[]=\"" > "version.h"

git --version 2>&1 >/dev/null
GIT_IS_AVAILABLE=$?
if [ $GIT_IS_AVAILABLE -ne 0 ]; then
git rev-parse --short HEAD | tr -d '\n' >> version.h
fi

echo -n "\";" >> "version.h"