# Big Snatch Vehicles III
[![Build status](https://ci.appveyor.com/api/projects/status/hyiwgegks122h8jg?svg=true)](https://ci.appveyor.com/project/aap/re3/branch/master)
<a href="https://discord.gg/jYpXxTm"><img src="https://img.shields.io/badge/discord-join-7289DA.svg?logo=discord&longCache=true&style=flat" /></a>
<a href="https://ci.appveyor.com/api/projects/aap/re3/artifacts/bin/DebugCI/re3.dll?branch=master&job=Configuration%3A+DebugCI"><img src="https://img.shields.io/badge/download-debug-9cf.svg" /></a>
<a href="https://ci.appveyor.com/api/projects/aap/re3/artifacts/bin/ReleaseCI/re3.dll?branch=master&job=Configuration%3A+ReleaseCI"><img src="https://img.shields.io/badge/download-release-blue.svg" /></a>

The aim of this project is to reverse GTA III for PC by replacing
parts of the game [one by one](https://en.wikipedia.org/wiki/Ship_of_Theseus)
such that we have a working game at all times.
## Getting Started
### Prerequisites
Re3 requires assets to work, so you need to own copy of GTA III.

Also re3 heavily depends on MSVC (memory layout), futher more, only windows is supported for now. 
### Installing
1. Put script file "main_freeroam.scm" into data catalog. (available in gamefiles)
2. Put binary of re3 into main catalog of game.
 
   Apparently you can download its latest version here:
   [Debug](https://ci.appveyor.com/api/projects/aap/re3/artifacts/bin/DebugCI/re3.dll?branch=master&job=Configuration%3A+DebugCI), 
   [Release](https://ci.appveyor.com/api/projects/aap/re3/artifacts/bin/ReleaseCI/re3.dll?branch=master&job=Configuration%3A+ReleaseCI).

3. re3 is actually available as dynamic linked library, so you need to load it into game.

   To achive that you can use [simple dll loader](https://github.com/aap/simpledllloader).

    -  Put [x86 version of dinput8.dll into main catalog.](https://github.com/aap/simpledllloader/releases/download/v1.2/simpledllloader_1.2.zip)
    -  Create empty file `dlls.cfg`(in main catalog of game) and add inside it `re3`. (To indicate what it should load.)
### Standart mission script

Press `g` when game is loading.

### Development and Contributing

Please visit wiki for details.
