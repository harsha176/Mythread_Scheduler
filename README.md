Group Information:
-----------------------------
hmalipa Harshavardhan Reddy Malipatel
jjohn Joice John
srmale Santosh Reddy Male

Files
-----------------------------
All files are located under hmalipa_hw3/a5/ directory


Build steps:
----------------------------
Compiling:
      make -f Makefile
      or make
This command compiles all the files related to thread library and creates an executable "driver" which runs our testcase defined in mytest.c
      
Clean:
      make -f Makefile clean
This command cleans all the object files and executables related to thread library as well as a3 and a4 executables

Testcase:
-----------------------------
mytest.c

The concurrency level is set to 2. In this program 10 threads are created with their priority values changing from 10 to 19. Thread 0 has highest priority value 19 and so is the least priority thread and it executes at the end.
