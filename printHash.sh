#!/usr/bin/env sh
if [ -z "${1}" ]
 then
  printf "%s\n" "Input the path to the file for writing the commit hash to."
 else
  printf "%s" "#define GIT_SHA1 \"" > $1

  if (command -v "git" >/dev/null) then
   git rev-parse --short HEAD | tr -d '\n' >> $1
  fi

  printf "%s\n" "\"" >> $1
  printf "%s\n" "const char* g_GIT_SHA1 = GIT_SHA1;" >> $1
fi
