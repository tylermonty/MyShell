I have implemented all functions and have everything working
correctly except for wildcards. I actually ended up getting wildcards to work
most of the time, but I could not figure out how to free up the memory I was
using for them and was getting some memory leaks, so I commented it out. You
can see what I had written to do wildcards, and it works, but I thought it
would be more important to have all my memory cleared up instead of having that
functionality. My commented code is where I tokenize the command line and
set my args array. Also, when using a command I have not built in, valgrind
will show that I have leaks at first, but if you exit my shell, all my
memory leaks are freed. Shen expressed this was not an issue to be worried
about.
