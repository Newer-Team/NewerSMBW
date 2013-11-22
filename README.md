# NEWER Super Mario Bros. Wii
## Source Code Public Release

### Preamble

Well, here it is, the thing you've been awaiting for... years. Yep.
My original plans were to majorly clean up and reorganise the codebase and
tools, because I really wasn't satisfied with the current state of them.

Unfortunately, this didn't quite pan out - as much as I'd like to have done
this, getting everything into the state I wanted would require months upon
months of work - something that's a bit difficult to do nowadays, since Newer
is done and I don't wish to devote yet more of my life to projects related to
NSMBW.

So... I've decided to dump the source code here so other people can do
something with this. Much of it isn't really in a usable state - but I'm
releasing it in case someone else wants to take up development and work on
getting everything polished and ready.

*-Treeki, 22nd November 2013*

### Licensing

The Newer SMBW source code and tools are released under the MIT license.
See the `LICENSE` file in this repository for full details.

Copyright (c) 2010-2013 Treeki, Tempus, megazig

### Requirements

#### For compiling the Newer game hacks:

- OS and setup where you can compile LLVM and Clang
- devkitPPC
- Python 2.x
- Python libraries: PyYAML, pyelftools

#### For using the Newer tools:

- Windows, Linux or Mac OS
- Python 2.x
- PyQt
- Possibly something else I forgot?

### What's Here

#### Clang Patchset

Newer requires a modified version of the [Clang](http://www.llvm.org)
compiler. Clang is a modern C/C++ compiler created as part of the LLVM project,
and Newer uses it to generate compiled code that can be injected into the game.

Most Wii homebrew development is done using GCC (usually as part of devkitPPC),
but regular compilers cannot be used because the generated code uses a
different C++ ABI from CodeWarrior (which is what Nintendo uses) and therefore
is not compatible with the NSMBW binary.

My (admittedly, very kludgey) patches to Clang modify various aspects of the
generated code to work round this. It's not perfect: in particular, multiple
inheritance and RTTI are not compatible and possibly entirely broken.
Thankfully NSMBW doesn't really make use of these features, so it works just
fine for Newer.

Another caveat is that I stopped working on this a few months back, so you'll
need to pull an old-ish version of LLVM and Clang from their SVN repos - the
patch I included is based off SVN revision 184655. LLVM is unmodified, but you
should probably also stick to this version of it to avoid possible
incompatibilities.

#### Kamek

All the Newer hacks are inside the Kamek folder in this repository because
that's how our files have always been organised, but technically, Kamek is
really just a simple build system for calling the tools to compile/assemble
each source file and putting together an output file you can inject into your
game.

Unlike most typical build systems, though, there's a bunch of features
specific to game hacks like Newer:

- Kamek is configured through a game-specific project file; each project
  pulls in various modules, which can each contain C++ and assembly source
  files, and hooks
- Hooks are different types of patches that are used to inject things into the
  game: patch static code/data in RAM, overwrite pointers with a reference to
  a symbol exported from Kamek code, inject branch instructions, ...
- Memory addresses in the linker script and used by hooks are automatically
  converted to match all supported game versions - Kamek uses the original
  version of the PAL/European NSMBW release as a reference.
- The compiled output is converted to a specific format expected by the
  Newer loader.

There's also a few things I'm not quite happy with...

- The projects/modules system turned out to be rather messy in Newer
- I think there's some features which just aren't used any more because the
  Newer loader changed over time.
- I wanted to add some NSMBW-specific features, like an easy way to inject
  new actors and sprites without having to modify the data structures using
  very-easy-to-mess-up Kamek hooks, but I never got round to this

#### Newer Hacks

The stuff that makes this game tick. Thousands of lines of C++ code, a
smattering of assembly, tons of patches, and a lot of blood, sweat and
tears. (No Treekis or Tempuses were harmed in the creation of these. Well.
Not harmed THAT much, anyway.)

You'll need to set up the stuff in the above two sections to be able to build
your own copy of the hacks.

This is the source code for the unfinished release with translation support,
so there's likely to be some bugs here. In particular:

- Expect to see some broken stuff with translations: wrong messages, etc.
- The final boss battle doesn't work. I never managed to figure out why.
- Possibly other things are broken?

#### Koopatlas

Where do I even begin with this...

This is the editor half of Koopatlas - a totally new 2D map engine we wrote
for Newer. Without going into too much detail, here's a quick roundup:

- Ridiculously buggy and unpolished editor
- 2D maps with an unlimited* amount of layers
- Tileset layers, supporting an unlimited* amount of tilesets
- Doodad layers, allowing you to place arbitrary textures on the map at any
  position and scale/rotate them
- Doodad animations
- Unlockable paths and level nodes
- More hardcoded things than you can shake a stick at (possibly rivalling
  Nintendo's 3D maps)
- Multiple maps with entrances/exits a la Reggie
- Maps are stored in a ".kpmap" format for easy editability - a JSON file in a
  specific format - and exported to an optimised ".kpbin" format for usage
  in-game

*\*Unlimited: Not really. This is the Wii, a game console which was
underpowered when it was released in 2006. There's not a lot of room in RAM
for lots of tilesets and doodads. A couple of the Newer maps use up almost all
the available space, so...*

If you want to make maps, feel free to try it. Then bash your head against a
wall when you accidentally close the editor and lose your unsaved work because
there's no warning against that. Or when it crashes on you, which might happen.

#### Misc Tools

There's a bunch of scripts in here which do... things. Most of them might not
be useful in their current state. Just look through and see what you can
gather, I guess.

Don't ask me why the random tiles script is in Ruby. Well, if you really want
to know, I was learning Ruby at the time and wanted to write stuff in it to
improve my skills...


### So how DO I use this junk?

Good question.

I could probably write an entire book (or two) documenting how to set up the
tools, the internals of the NSMBW game engine, and everything I added in Newer.
But... do you have any idea how long that would take?

The setup procedures aren't really documented at all, but I'm hoping that
anyone with the skills to actually work on this stuff will be able to figure
out what they're doing and get it running by looking at the error messages.

As for the game engine APIs - I'd love to document these, but it would simply
take way, way too long. Let me know if you have any technical questions
involving these: post in the thread on RVLution, or query me on Freenode IRC
(my nick is always Treeki).

