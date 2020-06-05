# re3
[![Build status](https://ci.appveyor.com/api/projects/status/hyiwgegks122h8jg/branch/master?svg=true)](https://ci.appveyor.com/project/aap/re3/branch/master)
<a href="https://discord.gg/jYpXxTm"><img src="https://img.shields.io/badge/discord-join-7289DA.svg?logo=discord&longCache=true&style=flat" /></a>
| Platform | Debug | Release |
|------------------|-------------|-------------|
| Windows Direct3D9 | [![Download](https://api.bintray.com/packages/gtamodding/revc/Debug_win-x86-librw_d3d9-mss/images/download.svg)](https://bintray.com/gtamodding/revc/Debug_win-x86-librw_d3d9-mss/_latestVersion) | [![Download](https://api.bintray.com/packages/gtamodding/revc/Release_win-x86-librw_d3d9-mss/images/download.svg)](https://bintray.com/gtamodding/revc/Release_win-x86-librw_d3d9-mss/_latestVersion) |
| Windows OpenGL3.3 | [![Download](https://api.bintray.com/packages/gtamodding/revc/Debug_win-x86-librw_gl3_glfw-mss/images/download.svg)](https://bintray.com/gtamodding/revc/Debug_win-x86-librw_gl3_glfw-mss/_latestVersion) | [![Download](https://api.bintray.com/packages/gtamodding/revc/Release_win-x86-librw_gl3_glfw-mss/images/download.svg)](https://bintray.com/gtamodding/revc/Release_win-x86-librw_gl3_glfw-mss/_latestVersion) |

## Intro

The aim of this project is to reverse GTA III for PC by replacing
parts of the game [one by one](https://en.wikipedia.org/wiki/Ship_of_Theseus)
such that we have a working game at all times.

## How can I try it?

- re3 requires game assets to work, so you **must** own a copy of GTA III.
- Build re3 or download it from one of the above links (Debug or Release).
- (Optional) If you want to use optional features like Russian language or menu map, copy the files in /gamefiles folder to your game root folder.
- Move re3.exe to GTA 3 directory and run it.

## Preparing the environment for building

Currently only building on VS2015/2017/2019 (Windows) and GCC (Linux) is tested.

- Clone the repo.
- Run `git submodule init` and `git submodule update`.
- Point GTA_III_RE_DIR environment variable to GTA3 root folder.
- Run premake
	- On Windows: one of the `premake-vsXXXX.cmd` variants on root folder
	- On Linux: proceed to [Building on Linux](https://github.com/GTAmodding/re3/wiki/Building-on-Linux).
- There are various settings at the very bottom of [config.h](https://github.com/GTAmodding/re3/tree/master/src/core/config.h), you may want to take a look there. i.e. FIX_BUGS define fixes the bugs we've come across.

> :information_source: **Rendering engine** re3 uses completely homebrew RenderWare-replacement rendering engine; [librw](https://github.com/aap/librw/). librw comes as submodule of re3, but you also can use LIBRW enviorenment variable to specify path to your own librw.

## Contributing

### Unreversed / incomplete classes (at least the ones we know)
The following classes have only unused or practically unused code left:
```
CCullZone - only mobile stuff
CCullZones - only mobile stuff
```

### Coding style

I started writing in [Plan 9 style](http://man.cat-v.org/plan_9/6/style),
but realize that this is not the most popular style, so I'm willing to compromise.
Try not to deviate too much so the code will look similar across the whole project.

To give examples, these two styles (or anything in between) are fine:

```
type
functionname(args)
{
	if(a == b){
		s1;
		s2;
	}else{
		s3;
		s4;
	}
	if(x != y)
		s5;
}

type functionname(args)
{
	if (a == b) {
		s1;
		s2;
	} else {
		s3;
		s4;
	}
	if (x != y)
		s5;
}
```

This one (or anything more extreme) is heavily discouraged:

```
type functionname ( args )
{
  if ( a == b )
  {
    s1;
    s2;
  }
  else
  {
    s3;
    s4;
  }
  if ( x != y )
  {
    s5;
  }
}
```

i.e. 

* Put the brace on the same line as control statements

* Put the brace on the next line after function definitions and structs/classes

* Put an `else` on the same line with the braces

* Don't put braces around single statements

* Put the function return type on a separate line

* Indent with TABS

As for the less cosmetic choices, here are some guidelines how the code should look:

* Don't use magic numbers where the original source code would have had an enum or similar.
Even if you don't know the exact meaning it's better to call something `FOOBAR_TYPE_4` than just `4`,
since `4` will be used in other places and you can't easily see where else the enum value is used.

* Don't just copy paste code from IDA, make it look nice

* Use the right types. In particular:

    * don't use types like `__int16`, we have `int16` for that

    * don't use `unsigned`, we have typedefs for that

    * don't use `char` for anything but actual characters, use `int8`, `uint8` or `bool`

    * don't even think about using win32 types (`BYTE`, `WORD`, &c.) unless you're writing win32 specific code

    * declare pointers like `int *ptr;`, not `int* ptr;`

* As for variable names, the original gta source code was not written in a uniform style,
but here are some observations:

    * many variables employ a form of hungarian notation, i.e.:

    * `m_` may be used for class member variables (mostly those that are considered private)

    * `ms_` for (mostly private) static members

    * `f` is a float, `i` or `n` is an integer, `b` is a boolean, `a` is an array

    * do *not* use `dw` for `DWORD` or so, we're not programming win32

* Generally, try to make the code look as if R* could have written it
