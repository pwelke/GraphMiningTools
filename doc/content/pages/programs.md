Title: Program Functionality
Date: 2015-12-01
Modified: 2015-12-01
Category: Documentation
Tags: pelican, publishing
Authors: Pascal Welke
Summary: Description of the executables that can be generated from the smallgraph library.

# General Comments on the Executables

The executables are compiled on a recent ubuntu linux version and try to follow unix philosophy. 
That is, they accept command line parameters and each executable has a dedicated purpose that can often be chained together to achieve things.
Command line parameters are processed using the getopt library and hence behave similarly to the gnu utils.

Each program typically expects data on standard input and writes to standard output, unless specified otherwise by some flags.
Extensive help is provided using the -h flag:

```
./lwg -h
```
for example, prints help on the usage of the levelwise graph mining executable.