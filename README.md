# Personal-Unix-Shell

Personal-Unix-Shell is a toy shell that interprets (runs) a small subset of shell commands. Overview: 3 cases: (1) echo, (2) fork-exec a program, (3) list of commands. Moreover, in each case, and at each level, there are also stdout redirection capabillities. 

Example Code:
```
{
echo A 
{ ls -l ; echo B > f1 ; cat f1 ; } > f2 
echo C 
} 
```

Command Semantics:
- Echo: Print to stdout
- Forx: This is the fork-exec case. struct forx_d has the program pathname and the command line arguments, in a format ready for straight passing to a suitable exec syscall.
  - If the child exits, the return value is the child’s exit status.
  - If the child is killed by signal, the return value is 128+signal (analogous to real shells). 
- List: Multiple commands in an array to run sequentially; waiting for one to finish before running the next. Except: If running a command results in the return value of 128+SIGINT, if there is a kill signal.
