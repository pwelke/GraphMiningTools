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
The first line of a graph description starts with a hashtag and gives, in this order, a graph id (integer), a graph (target) label (integer), the number of vertices n (integer), and the number of edges m (integer).
The second line gives exactly n vertex labels.
The third line gives exactly m edge definitions, each starting with start and end vertex of the edge and a label.
Vertex indexing starts with 1.
Labels (of vertices and edges) may be arbitrary strings. 
However, some symbols should not be used; these are 

- any kind of whitespace
- ( ) $ #

It is probably best to restrict labels to alphanumeric symbols, only.

There are several converters available for the aids format, e.g. from and to gaston, gspan, fsg format.