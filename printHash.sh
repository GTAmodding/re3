#!/usr/bin/env bash

> $1

echo -n "#define GIT_SHA1 \"" > $1

if (command -v "git" >/dev/null) then
git rev-parse --short HEAD | tr -d '\n' >> $1
fi

echo "\"" >> $1
echo "const char* g_GIT_SHA1 = GIT_SHA1;" >> $1