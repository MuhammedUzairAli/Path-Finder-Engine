For anyone wanting to use this

firstly the c file must be compiled before anything
you can do this if you have a gcc compiler by

on windows
gcc path_finder_engine.c -o path_finder_engine.exe

on macOS/linux
gcc path_finder_engine.c -o path_finder_engine
chmod +x path_finder_engine

Input:

1.For pure CSV
csv must have trailing commas , at end
csv must be named terrain

N is for a road risk/traffic can be specified by typing a number before or after it
in form
,2N, or ,N2, or ,1N1, in the last case the numbers on both sides are added.
V is village this is your destination every Source will try and get a path to it.
H is source this will get a path to each destination.

V and H can be passed through. and can also be assigned risk but it will be internal only if you want them to be impassable assign them a high risk such as 9999.

B is blocked road/ green terrain.

any symbol/ character outside these will be treated as B inluding whitespace/ " ".

empty space is not advised ie ,,,,,

2.For image processor:
img files can be jpg or png and can have anyname

erything is read pixel by pixel so one road should only be 1 pixel 
especially with H and V multiple pixels for those will result in multiple H and multiple V
 
greys around the 128 128 128 range are considered road (between 200 and 40 also each channel must be within 10 of each other transparency not advised keep everything opaque if possible)
to give risk the redder the road the more risk upto a max of 5 but it can be changed in source code to more on img_to_map line 11 in rLvl = int(1 + (((r-max(g,b)) - 50) * 4) / 205) change 4 to max -1

blue is Hub
and black is Vil
any other colour outside these is B or terrain will be displayed green
multiple imgs can be gone through

Output: 

1.Render (pure csv):
this will take and output results for your csv

2.img_to_map:
this will take and output result for each img file in terrain images
