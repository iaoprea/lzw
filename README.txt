/*
 * Program name:        unlzw - lzw decompressor
 * Program author:      Alex Oprea <ionutalexoprea@gmail.com>
 */


Contents:
	(1.1) Build
	(1.2) Build for debug
	(1.3) Coding style
	(1.4) Cppcheck
	(1.5) Run
	(1.6) Run debug
	(2) Environment
	(3) Compressor performance

(1.1) Build
To build the executable run:
	make 

(1.2) Build for debug
To build the binary for debugging run:
	make debug

(1.3) Coding style
To run the coding style checker (checkpatch.pl from kernel.org) run: 
        make coding_style 
If the checkpatch.pl script is not available,
uncomment the 2 lines from the makefile that will download it.

(1.4) Cppcheck
To run a basic cppcheck on the source file, run 
        make cppcheck 
If cppcheck is not installed, it can be installed 
by uncommenting the line in the makefile.

(1.5) Run
To run the executable:
	make run
Change the name of the compressed an decompressed files in the makefile.

(1.6) Run debug
To run the debug binary in valgrind type:
	make run_debug
Change the name of the compressed an decompressed files in the makefile.



(2) Environment 
Solution development was done on Linux Mint 17.2 Rafaela virtual machine, uname output: 
        "Linux mintvm 3.16.0-38-generic #52~14.04.1-Ubuntu SMP x86_64 x86_64 x86_64 GNU/Linux" 
Compiler version: 
        "gcc (Ubuntu 4.8.4-2ubuntu1~14.04) 4.8.4"



(3) Compressor performace
An easy way to implement a lzw compressor is to sequentially loop over the
entire dictionary to find the longest string that matches the input.

An optimized way of implementing a lzw compressor would be:
	- keep several tries that together form the dictionary.
	- each trie includes all the words of a specific length.
	- when trying to determine the existence of a specific string,
	  search only within that specific trie that stores strings of the
	  same length as the searched string.
