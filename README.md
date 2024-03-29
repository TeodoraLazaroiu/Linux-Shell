# Shell implementation in C


### Compilation and execution in Linux:
```
gcc shell.c -o shell -lreadline && ./shell
```

### Show all the commands:
```
SHELL: /current-path$ help

Commands:
--------------------------------------------------------

history: prints the command history of the current session
clear: clears the terminal
cd: changes the path to a particular directory
pwd: prints the path of the current directory
ls: lists all files and directories in the current directory
touch: creates a new empty file
rm: deletes a particular file 
cp: copies the content of a file to another file
makedir: creates a new directory
removedir: deletes an already existing directory
echo: displays a string that is passed as an argument
quit: exits the shell

-----------------------------------------------------
```

### Example of commands:
```
// touch fisier | makedir
// echo sir de caractere | echo
// echo folder | makedir

// lss || ls
// cd something || ls || history
// cd something || ls && history

// cd something && ls
// help && ls
// ls && lss && help
// ls && cd something || ls
```
