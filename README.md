# GraphMiningTools

This is very much work in progress basically all the time.

A suite of graph related tools that try to be efficient and offer a unix-philosophy interface to transactional graph databases. 
I.e., you have a bunch of small- to medium-size labeled graphs and want to do something with it. 
The programs accept command line parameters and each executable has a dedicated purpose that can often be chained together to achieve more complicated things.
Command line parameters are processed using the getopt library and hence behave similarly to the gnu utils.

Different parts of this suite implement my PhD thesis and various papers.

## Installation and Usage

Clone this repository and run 
```bash
make all 
```
to compile all executables in this repository. Development and everything else happens on a recent Ubuntu Linux distribution. I cannot guarantee compatibility to other flavors of Unix and would be amazed if anything of this ran on Windows ever at all. 

Each program typically expects data on standard input and writes to standard output, unless specified otherwise by some flags.
Extensive help is provided using the -h flag:

```bash
./lwg -h
```
shows you how to use the levelwise graph mining utility.

## Data Format and further documentation

You can find a file describing the data formats in ```doc/content/pages/fileformat.md```
Some other brief documentation can be found in the same folder. 
