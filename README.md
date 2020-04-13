# re3
[![Build status](https://ci.appveyor.com/api/projects/status/hyiwgegks122h8jg?svg=true)](https://ci.appveyor.com/project/aap/re3/branch/master)
<a href="https://discord.gg/jYpXxTm"><img src="https://img.shields.io/badge/discord-join-7289DA.svg?logo=discord&longCache=true&style=flat" /></a>
<a href="https://ci.appveyor.com/api/projects/aap/re3/artifacts/bin/Debug/re3.dll?branch=master&job=Configuration%3A+Debug"><img src="https://img.shields.io/badge/download-debug-9cf.svg" /></a>
<a href="https://ci.appveyor.com/api/projects/aap/re3/artifacts/bin/Release/re3.dll?branch=master&job=Configuration%3A+Release"><img src="https://img.shields.io/badge/download-release-blue.svg" /></a>

## Intro

The aim of this project is to reverse GTA III for PC by replacing
parts of the game [one by one](https://en.wikipedia.org/wiki/Ship_of_Theseus)
such that we have a working game at all times.

## How can I try it?

- re3 requires game assets to work, so you need to own a copy of GTA III.
- Since re3 is a DLL that works with original GTA III for now, you need Simple DLL Loader. You can get it [here](https://github.com/aap/simpledllloader).
- Build re3 or download it from one of the above links (Debug or Release).
- Make sure you included the re3 in `plugins.cfg` or `dlls.cfg`.

![#ffa500](https://placehold.it/15/ffa500/000000?text=+) **Notice if you will build it**

There are various settings at the very bottom of `config.h`, you may want to take a look there. i.e. FIX_BUGS define fixes the bugs we've come across.

https://github.com/GTAmodding/re3/tree/master/src/core/config.h

## I want to contribute, where should I start?

A good approach is to start at the fringes of the code base,
i.e. classes that don't depend on code that we don't have reversed yet.
If a function uses only few unreversed functions that would be inconvenient
to reverse at the time, calling the original functions is acceptable.

### Unreversed / incomplete classes (at least the ones we know)
```
cAudioManager - WIP
CBulletInfo
CPedPath
CRoadBlocks
CWeapon
CWorld
GenericLoad
```

The following classes have only unused or practically unused code left:
```
CCullZone - only mobile stuff
CCullZones - only mobile stuff
CSceneEdit
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

### Environment Variables
Here you can find a list of variables that you might need to set in windows:
```
"GTA_III_RE_DIR" * path to "gta3_re" game folder usually where this plugin run.
"GTA_III_DIR" * path to "GTAIII" game folder.
```
