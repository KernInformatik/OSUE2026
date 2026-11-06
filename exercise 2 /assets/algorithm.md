# Sketching

Idea: every process is one node of the quadtree. It reads its points from `stdin`, splits them into the 4 quadrants, forks one child per quadrant (`fork` + `execlp` of the same program) and sends each child only the points of its quadrant via pipe. The child does the same again with a smaller rectangle, until only 1 line is left -> distance check -> print. The parent collects what the children print and passes it up in a fixed order.

So: not 1 fork per point, always exactly 4 children per process (one per quadrant). The recursion happens through the processes themselves.

No shared memory, no delays: pipes + `wait` already do the synchronizing.

<br>

## Args

`./quadtree <goal_x>,<goal_y> <distance> <min_x> <max_x> <min_y> <max_y>`

- parse with `strtod`, check that the whole string was consumed (`endptr`)
- goal: check for the comma between x and y
- anything fails to parse -> `EXIT_FAILURE`
- order of the 4 limits: double check in the assignment, I assume `min_x max_x min_y max_y`
- `<goal>` and `<distance>` never change, they get passed 1:1 to every child. Only the 4 limits change.

<br>

## main.c should contain

- parse args (see above)
- read ALL lines until EOF into a dynamic array (`realloc`, no idea how many lines come)
- store the raw text of each line, not only the doubles. The child then gets exactly the same text, nothing changes through printing/rounding
- base cases (see below)
- split into 4 quadrants + 4 children
- merge the output + `wait`

A SoA per quadrant isn't really needed: one array with all lines, and per child just go through it and write the lines that belong to its quadrant. Less mallocs/frees.

<br>

## Base cases of one process

- 0 lines -> `EXIT_SUCCESS`, print nothing (happens all the time, empty quadrants)
- 1 line -> distance = `sqrt(pow(x - goal_x, 2) + pow(y - goal_y, 2))`. If `<= <distance>` print `%.3f: x,y`, then `EXIT_SUCCESS`. The distance is always to the goal, never to the pivot.
- n > 1 lines -> ignore all lines equal to the first line. If all lines are the same -> `EXIT_SUCCESS`. Otherwise split.

<br>

## Pre-Algorithm

- Split the canvas vertically and horizontally at the center of the own rectangle (NOT at the goal):
  - `mid_x = (min_x + max_x) / 2`
  - `mid_y = (min_y + max_y) / 2`

```
 max_y +-----------+-----------+
       |    TL     |    TR     |
 mid_y +-----------+-----------+
       |    BL     |    BR     |
 min_y +-----------+-----------+
     min_x       mid_x       max_x
```

- Limits for each child:

| child | min_x | max_x | min_y | max_y |
|-------|-------|-------|-------|-------|
| BL    | min_x | mid_x | min_y | mid_y |
| BR    | mid_x | max_x | min_y | mid_y |
| TL    | min_x | mid_x | mid_y | max_y |
| TR    | mid_x | max_x | mid_y | max_y |

- Which quadrant does a point belong to: `x < mid_x` -> left, else right. `y < mid_y` -> bottom, else top. Points exactly on a line need ONE fixed rule, the assignment doesn't say which, just stay consistent.
- The limits go to the children as text -> print the midpoints with enough digits (`%.17g` or so), otherwise the boundaries drift on deeper levels and points land in the wrong quadrant.

<br>

## Algorithm (one process)

1. parse args, read `stdin` until EOF
2. base cases (0 lines / 1 line / duplicates)
3. compute pivot + the 4 limits
4. per child (BL, BR, TL, TR): create the 2 pipes, `fork`. This step only STARTS the 4 children, no writing / reading / waiting yet (see "Step 4 in detail")
5. in the child: `dup2`, close everything unused, `execlp`
6. in the parent: write the lines of each quadrant into its child, close the write ends
7. read the results in the order BL, BR, TL, TR and copy them to `stdout`
8. `waitpid` for all 4, check the exit status

<br>

## Step 4 in detail

- "per child" = per quadrant 2 pipes (A: parent -> child, B: child -> parent) -> 8 pipes and 4 forks in total
- keep everything in arrays with 1 slot per quadrant, index 0..3 = BL, BR, TL, TR (pipe ends of A, pipe ends of B, pid). Same index everywhere, so the pipes never get mixed up.
- the loop over the 4 quadrants only starts the processes: create pipes -> `fork` -> next quadrant. NO write, read or `wait` inside this loop.
- only when all 4 children exist: feed them (step 6) -> read from them (step 7) -> wait for them (step 8)
- why: if I do the whole cycle per child (fork -> feed -> read -> wait) before starting the next one, the children run one after another. The assignment says they must run simultaneously.
- the order BL, BR, TL, TR for the forks doesn't change the result, the output order only comes from the order of reading in step 7
- inherited ends: when forking child 2, the parent still has the pipe ends of child 1 open and child 2 gets a copy of them. If child 2 keeps them, child 1 never sees EOF -> hang. So in every child, FIRST close all parent-side ends (its own and those of the earlier children), THEN `dup2` its own two ends, close the originals, `execlp`.

<br>

## Pipes (per child)

```
parent --write--> [pipe A] --read--> child stdin
parent <--read--- [pipe B] <--write-- child stdout
```

Child, before `execlp` (same program again via `argv[0]`, args = same goal + distance + its own 4 limits):
- `dup2` read end of A -> `STDIN_FILENO`
- `dup2` write end of B -> `STDOUT_FILENO`
- close ALL pipe ends it has (the originals + the ones of the earlier children, they get inherited!)
- if `execlp` fails -> `EXIT_FAILURE`

Parent:
- close the read end of A and the write end of B of each child
- write the lines of the quadrant into A, then close A. The child only sees EOF after that, and only then it can continue.
- read B in the fixed order, copy to `stdout` until EOF. No parsing needed, it already is the final format.

Hang = most likely a write end that is still open somewhere. Every process closes every end it doesn't use.

Children must run simultaneously: fork all 4 -> feed all 4 -> read all 4 -> wait. NOT fork one -> wait -> fork next.

<br>

## Exit status

- `waitpid` for all 4 children. If one of them is not `EXIT_SUCCESS` -> `EXIT_FAILURE`
- otherwise `EXIT_SUCCESS`
- free everything (the line array), check return values of `fork`, `pipe`, `dup2`, `execlp`, `write`

<br>

## Open questions (ask tutors / check assignment)

- "ignore all lines equal to the first line": in the example `1,1` is the first line and still gets printed. So what exactly happens with the first line itself?
- all lines identical -> only `EXIT_SUCCESS`, or print the point (if in range) once?
- points exactly on the pivot line -> which quadrant?
- 0 lines -> I assume no output + `EXIT_SUCCESS`
- order of the limits in the args

<br>

## Order to build it

1. parse args + read `stdin`, 1 line case works already (leaf)
2. split without any processes, just print which quadrant each line goes to
3. 1 child with both pipes, get a line through and back
4. 4 children at the same time + merge in order BL, BR, TL, TR
5. exit status + error handling

<br>

## Tests

- `./quadtree 0,0 4.5 -10 10 -10 10 < 1.txt` -> `2.828: 2,-2`, `4.243: -3,3`, `1.414: 1,1` (this order = BR, TL, TR)
- same with `| sort`, compare with `diff`
- empty input, 1 line, 2 identical lines, bad args (no comma, letters)
- `strace -f` to see the forks and pipes, debug prints to `stderr`, `valgrind` for leaks