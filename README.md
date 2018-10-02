# EDA093_OperatingSystems

##Tests

###Simple Commands

+ date: Day Month Time Time-zone Year
+ hello: No such file or directory. A child process is never created since thers is no program in the programlist.
      + The difference between out implementation of the shell and the normal terminal is that out implementation
        answers with 'No such file or directory' but the terminal says '-bash: hello: command not found'.

Both commands is printed on a new line in the shell. 

###Commands with parameters

+ ls -al -p: It shows the total number of files recursivily in each of the directories in the current directory.
+ ls parse*: Works in a normal terminal but in out shell it prints 'ls: parse*: No such file or directory'

###Redirections with in and out files

+ ls -al > tmp.1: No response, however the file 'tmp.1' is created with the info from ls -al.
+ cat < tmp.1 > tmp.2: The input from ls -al is moved from tmp.1 to tmp.2. Cat creates a child.
+ diff tmp.1 tmp.2: Since the previous commands moved the output to tmp.2 it differs with all lines.

###Running background programs

+ emacs &: Create a child for emacs but tou're still able to write in the shell
+ emacs &: Does the same as above but creates another child.
+ emacs: Creates a third child .
+ ps: By running the command several times, we can use 'kill' to kill the process and not leaving any zombies.
But with xkill, we get stuck in the while loop that collect the return values of the children.

###Process communications

+ ls -al | wc: It shows the a list of all the files with the word count flag and then the prompt appears on next line.
+ ls | sort -r: It sors the list reversed and also shows the prompt after the prompt.
+ ls | wc &: The prompt now appears after the output but you don't get a new command line.
+ cat < tmp.1 | wc > tmp.3: Creates a child when it does a word count on the information in tmp.1 and writes it to tmp.3.
But you get stuck in the cat command.
+ cat tmp.1 | wc: The two last outputs are not the same, being '17 146 950' and '3 3 13'. It does not create a child.
+ abf | wc: abf is not command
+ ls | abf: no such file or directory
+ grep apa | ls: Create a child for grep but get stuck in a loop. Doesn't display anything.

###Built-in commands

+ cd .. : 
+ cd lab1
+ cd tmp.tmp
No error is generated.??? built-in ???
+ cd ..
+ cd lab1 | abf : 'No such file or directory'
+ ls : The ls command still works
+ cd : It generates a 'Bad address' error
+ grep exit < tmp.1 : It tries to find 'exit' in a file and does not quit.
+   exit : It exits.
+ grep exit | hej : Still searches for 'exit' in the 'hej' file.
+ grep cd | wc : Considers cd as a string and can't be find. It appears after pressing Ctrl-D and prints the word count.
+ exit : It exists and there are no zombies.
+ ls | wc- : A 'No such file or directory' error is generated.
