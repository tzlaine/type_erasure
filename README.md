# type_erasure

This repo serves multiple purposes.

- First, it contains several "hand-rolled" implementations of the same erased
type, for reference.

- Second, it includes the "Type Erasure" presentation given at CppCon 2014.

- Third, it contains the `emptypen` tool, which generates type erasure code
based on a user-supplied implementation form.

Skip to the relevant section below, depending on what you came here for.
Build instructions can be found at the end.


## Reference Implmentations

Check out the `hand_rolled` and `boost_type_erasure` directories for examples
of various approaches to type erasure.  Note that there are `emtypen` form and
header files for most of the approaches -- everything but the manual v-table
and Boost.TypeErasure approaches.  All examples were written against the C++11
standard.


## Presentation

If you want to look at the presentation slides live in the tubes, go to:

http://tzlaine.github.io/type_erasure

Otherwise, the CppCon 2014 presentation can be found in `index.html`, with the
supporting files found in the `presentation` directory.  Point your browser at
`index.html` and get ready to rumble.  I don't know why I said that last part.


## `emptypen`

`emptypen` generates C++ code for one or more "erased types" (types that can
hold anything that fits a given interface) from a C++ header that defines the
required interface(s).  It performs this magic by asking the `libclang`
library (a wrapper around the Clang front end) what is in the given C++
header.  Detailed usage instructions can be found by passing `--help` or
`--manual` to `emtypen`.


## Build Instructions

First, note that the code in this repo is written against the C++11 standard.
It is known to work with Clang 3.4, GCC 4.9, and Visual Studio 2013.  It may
work with earier versions of Clang or GCC, but will not work with any earlier
version of Visual Studio.

To build the entire project, simply install CMake 3.0 and use it to build this
repo.  Specific steps are given at the end.

Many of the examples have coverage tests that require headers from Boost.Test.
The Boost.TypeErasure example also obviously requires Boost.  Neither requires
a compiled Boost library; the headers will suffice.  If you want to build
these items, define the CMake variable `BOOST_ROOT` (see below).  If you do
not want to build these items, it won't break anything not to define
`BOOST_ROOT`.

`emptypen` requires libclang headers and libs.  If you want to build
`emptypen`, define the CMake variable `CLANG_ROOT` (see below).  If you do not
want to build `emptypen`, it won't break anything not to define `CLANG_ROOT`.
At the time of this writing, there are no pre-built Clang binaries built with
Visual Studio 2013, but it's now very easy to build Clang from source with
Visual Studio.  There's a small wrinkle, however.  CMake doesn't like import
libs with extensions other than `.lib`, and `libclang`'s is built as
`libclang.imp`.  There's probably some obscure CMake incantation that fixes
this, but it's easier just to rename `libclang.imp` to `libclang.lib`.

If you build Clang from sources, it will make your life much easier to install
it somewhere, and then set `CLANG_ROOT` to the installed location.  This will
set up the directory structure expected by the `emptypen` build; the structure
of the build products for the built-in-place Clang is different.

Here are the specific steps to build this repo with CMake.  It is assumed that
this repo has been cloned into a directory caled "type_erasure":

- From a command shell, create a directory into which the build products will
  be generated, e.g. `type_erasure/build`.

- Move into the build directory.

- Execute `cmake <path> [-DBOOST_ROOT=<path_to_boost>] [-DCLANG_ROOT=<path_to_clang>`,
  where `<path>` is the relative path from your build directory to "type_erasure", e.g. `..`.

This will generate your build files.  On UNIX-like systems this will default
to generating makefiles, and on Windows this will default to generating Visual
Studio solution files.  You know what to do from here.
