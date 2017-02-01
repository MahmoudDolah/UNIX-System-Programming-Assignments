# UNIX-System-Programming-Assignments
Unfortunately, my implementation does not work.

## My Approach            
First, I created two structs, one for an individual cell (called cell), and another for the game board (called world). The cell struct has two members, neighbors and alive. Obviously, alive is whether the is alive or not. The neighbors field is a pointer to an array of the surrounding cells. The idea was that it would be easier to update the board if you have direct access to the nearby cells.           
Now, the world struct has a pointer to the cells (called positions), the number of rows, number of columns, and, most importantly, a pointer to the memory. A big reason why I chose to structure my code this way was to avoid memory leaks. My deleteWorld function only has to free up the memory that is already being tracked by the board itself.                          
I thought this would be a good idea because I have used C before when I took an Intro to C class back in high school and did something similar.
I think a big problem with my attempt is that it is a classic case of over engineering. After speaking with a few classmates, all of their examples simply made the game board a matrix (an array containing an array) and if that field contained a " * " character, then it meant that the cell was alive.


## What Went Wrong and Other Approaches
It causes a segmentation fault in the initializeWorld function and, despite my attempts, I am unable to find the mistake. However, I do believe that I confused rows, columns, and the line length at some point. I have worked my way backwards with gdb, but am unable to find the solution.
During my attempts to use gdb, it kept claiming that there was a problem with "iofgets.c", which leads to believe that there is an error associated with the way I used the fgets function. However, I have been unable solve this (although, perhaps I could have followed Linus's Law to get a solution).              
For completeness, I have written the entire program for you to investigate.


## What I Learned
KISS - Keep It Simple, Stupid.        
To be honest, I believe that I made my solution too complex, especially since this is the first time I have touched C (or C++) in well over a year and a half (since I took OS in fall of 2015).

## How to Run the Code
You can run the code by entering the directory and typing
```
make
```
This will create an executable called ```life```.
Now, enter
```
./life <rows> <columns> <filename> <generations>
```
to run the program.