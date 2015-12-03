Title: Home
Date: 2015-12-01
Modified: 2015-12-01
Category: Documentation
Tags: pelican, publishing
Slug: Home
Authors: Pascal Welke
Summary: Landing Page of the Documentation

Welcome to the wiki of the smallgraph graph kernel library and program suite.
I'll try to document as much as possible that exceeds the bare documentation of functions in the C code here.

## Description of the Project

This project is the by-product of my research on mining databases of small to medium sized graphs.
It was initially started to compute the cyclic pattern kernel of Horvath et al. for databases consisting of chemical molecules but grew into a large software suite consisting of several programs. A description of the functionalities can be found in the [Section about Programs]({filename}/pages/programs.md)

To this end, it provides data structures for graphs and several graph algorithms, like

- computing the biconnected components
- checking subgraph isomorphism of one tree into another
- deciding if a graph is outerplanar
- etc.


## Folder Structure

The code is organized into different modules that provide different graph algorithms like a subgraph isomorphism test for two trees in [subtreeIsomorphism.c](../subtreeIsomorphism.c) and [subtreeIsomorphism.h](../subtreeIsomorphism.h).

The [executables/]({filename}../executables/) folder contains source files that provide main methods which, after compilation, result in executable files.
The build process is described below.


## Build Process

The root folder contains a [Makefile]({filename}../Makefile) that contains a target for each executable that can be generated right away from this library.
Furthermore, it contains the target all that builds all executables.

For information on how to add support for your own executable to this Makefile, see [Add your own Make Target]({filename}/pages/addMakeTarget.md), for a starting point of your executable, look at the [Tutorial]({filename}/pages/tutorial.md).
