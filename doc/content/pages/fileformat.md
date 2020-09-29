Title: Graph Database File Format
Date: 2015-12-03
Modified: 2015-12-03
Category: Documentation
Tags: pelican, publishing
Authors: Pascal Welke
Summary: Description of the graph database format used by the smallgraph library.

# Graph Database Format
There are two file formats that can be processed by (some of) the programs.

- A generic graph database format, suited for collections of arbitrary labeled graphs.
- A canonical string format, suited for collections of trees.

Due to historic reasons, the first (and main) file format is called aids, the second is called canonical string or cstring.

# The aids format
The file format is an ascii file containing exactly three lines per graph. 
The file format is terminated by a single dollar sign on a new line.
Individual symbols are separated by spaces.

Let's have a small example:

```
# 1 0 3 3
1 0 1
1 2 a 1 3 b 2 3 b
# 3 1 3 2
ax bx cx
1 2 1 1 3 1
$
```

This example file contains two graphs with three vertices, each.
Here are they, plotted:

!(a0)[a0.png]
!(a1)[a1.png]

The first line of a graph description starts with a hashtag and gives, in this order, a graph id (integer), a graph (target) label (integer), the number of vertices n (integer), and the number of edges m (integer).
The ids must be unique within the database but do not need to be consecutive.
The second line gives exactly n vertex labels.
The third line gives exactly m edge definitions, each starting with start and end vertex of the edge and a label.
Vertex indexing starts with 1.
Labels (of vertices and edges) may be arbitrary strings. 
However, some symbols should not be used; these are 

- any kind of whitespace
- ( ) $ #

It is probably best to restrict labels to alphanumeric symbols, only.

There are several converters available for the aids format, e.g. from and to gaston, gspan, fsg format.


# The cstring format
The mining program (lwg, lwgr) outputs several files. 
Among them is the list of patterns, which are returned in a canonical string format.
That is, the canonical string is something you can compute for any tree and that is unique up to isomorphism. 
That is, the canonical strings of two trees T, T' are identical if and only if T and T' are isomorphic.

Let's have an example:

```
4	1	2 
3	13	2 ( 1 2 ) ( 1 6 ) 
4	12	2 ( 1 2 ) ( 1 2 ) 
3	92	2 ( 1 1 ) ( 1 2 ( 2 2 ( 1 2 ) ) ) ( 2 2 ( 1 1 ) ) 
3	87	2 ( 1 1 ) ( 1 2 ( 2 2 ( 1 2 ) ) ) ( 2 2 ( 1 2 ) ) 
3	125	2 ( 1 2 ) ( 1 2 ( 1 1 ) ( 2 2 ( 1 1 ) ( 1 2 ) ) ) ( 2 2 ( 1 2 ( 1 1 ) ) ) 
```

This file format is ascii based, as well. 
It contains one line per tree with three things in it, which are separated by tabs.
First, it reports a target value (int) for a tree. 
Typically, and in the case of the mining program in particular, this is the frequency of the pattern in a graph database.
Second, it reports an id for the pattern (which must be unique) and third there is a canonical string representation of the tree.

The canonical string of the first row reads as follows: 
There is a vertex with label 2 that is connected by an edge with label 1 to another vertex with label 2 and by another edge with label 1 to a vertex with label 6.
(The tree has id 13 and occurs three times in the database that was used here).

The canonical string representation is recursive; an open bracket starts an edge from the current vertex and a closing bracket closes the current subtree. 
It must always terminate with a space. 
E.g. '2 ' is a canonical string of a tree containing a single vertex labeled 2, while '2' is an invalid string that results in a segfault in some places.
See the drawings of the five graphs below.

!(0)[0.png]
!(1)[1.png]
!(2)[2.png]
!(3)[3.png]
!(4)[4.png]

You can convert trees from and to the cstring format using the cstring executable.
