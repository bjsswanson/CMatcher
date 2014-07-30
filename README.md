CMatcher
========

A port of my ImageMatcher Java program to C++

This is now a command line program instead of RESTful

Tested with OpenCV 2.4.9

Usage: ./CMatcher [-i input_file input_file] [-c -o output_file] [-f -d search_dir]

```
-i input_file input_file
```
This will compare 2 input files and print the number of good matches between them to cout

```
-c -o output_file
```
This will take an image with the camera and save it to the output file

```
-f -d search_dir
```
This will take an image and compare it to a folder of images and will return the relative path of the image with the most good matches to the image taken

Notes
====

Please note that both commands that take images also print "Cleaned up camera" to cout. This seems to be part of the libary and cannot be turned off.
