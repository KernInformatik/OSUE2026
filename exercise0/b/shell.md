Setup

Open a shell (that's it, see sub-exercises for more).

Consider using commands like grep, sed, cat, cut, man, and wc. You can find out more about each one using man.
0.b.0

You can see the manual page (manpage) for a program cmd, by using:

man cmd

Goal

Find out how often the word "more" appears in the manpage of less using a single line in the shell.


0.b.1

Run the following commands (you can also copy them into the shell):

# this should print "success"
true && (exit 0) && echo "success"

# this should print nothing
false || (exit 0) || echo "success"

# this should also print nothing
(exit 0) && (exit 1) && echo "success"

# this should print "foobar\nsuccess"
echo "foobar" | grep "foo" && echo "success"

# this should print nothing
echo "foobar" | grep "fobar" && echo "success"

Goals

    Modify the lines and find out how they work.
    How do you have to modify the lines that print nothing so they print "success"?
    How do you get the lines that print "success", to not print anything (without removing echo)?


0.b.2

You can use the output of a command to build another command line (for example):

echo Today is a $(date +%A) and the month is $(date +%B)

    Today is a Wednesday and the month is October

Goals

Pick a file and write a line that produces output like the following:

    The file main.c is 535 bytes long

Note: Use wc, see man wc for instructions.

Next, try printing a line that gives the combined length of multiple files, for example, all those ending in .c. It should print something like:

    The files list.c main.c combined are 2862 bytes long

