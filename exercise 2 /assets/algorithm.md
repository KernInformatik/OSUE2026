# Sketching

Idea: every process is one node of the quadtree. It reads its points from `stdin`, validates them, splits them into the 4 quadrants, forks one child per quadrant (`fork` + `execlp` of the same program) and sends each child only the points of its quadrant via pipe. The child does the same again with a smaller rectangle, until only 1 line is left -> distance check -> print. The parent collects what the children print and passes it up in a fixed order.

So: not 1 fork per point, always exactly 4 children per process (one per quadrant). The recursion happens through the processes themselves.

No shared memory, no delays: pipes + `wait` already do the synchronizing.

<br>

## Where is the recursion? (no quadtree struct needed)

I do NOT implement a quadtree data structure. No nodes, no pointers, no tree struct. The recursion is the program calling itself as a new process. `main()` is the recursive function, and one run of the program does exactly ONE level.

| normal recursive function | this exercise |
|---|---|
| function calls itself | program starts itself (`fork` + `execlp(argv[0], ...)`) |
| parameters (array, bounds) | `stdin` (the points) + command line args (goal, distance, limits) |
| return value | `stdout` (the result lines) + exit status |
| call stack | process tree |
| base case: `if (n <= 1) return` | base case: 0 or 1 line -> print and exit, no fork |

```
main():
    parse args
    read all lines from stdin
    if 0 lines:   exit
    if 1 line:    check distance, print, exit        <- base case, recursion stops
    split the lines into 4 groups by quadrant
    for each group: start a child of this same program   <- the "recursive call"
                    (limits = the smaller rectangle)
    feed each child its group via pipe                   <- "passing the parameters"
    read each child's output, print it in order          <- "using the return values"
    wait for the children
```

The "tree" is only the tree of processes that exists while the program runs. It is never built in memory.

Trace for `./quadtree 0,0 4.5 -10 10 -10 10 < 1.txt` (points `1,1`, `2,-2`, `-3,3`, `-4,-4`):

```
process 1  (limits -10..10, 4 lines)  -> mid = 0,0
  ├─ BL child: -4,-4   -> 1 line: distance 5.66 > 4.5, prints nothing
  ├─ BR child:  2,-2   -> 1 line: prints "2.828: 2,-2"
  ├─ TL child: -3,3    -> 1 line: prints "4.243: -3,3"
  └─ TR child:  1,1    -> 1 line: prints "1.414: 1,1"
process 1 reads BL, BR, TL, TR in that order and prints the combined output
```

A child that gets more than 1 line does the same again and starts its own 4 children. The rectangle gets smaller on every level, so there are fewer points per process.

What I actually write:
1. parsing + checking (args, lines)
2. the base cases
3. quadrant decision + the 4 new limits (only a few lines of arithmetic, this is the whole "quadtree" part)
4. the plumbing: pipes, fork, exec, feeding, reading, waiting (most of the work)

<br>

## Args

`./quadtree <goal_x>,<goal_y> <distance> <min_x> <max_x> <min_y> <max_y>`

- **`argc != 7` -> print usage to `stdout`, `EXIT_FAILURE`** (testcase 08: `./quadtree 10`). Usage message contains `argv[0]` and the synopsis.
- order of the 4 limits: `min_x max_x min_y max_y` (as in the SYNOPSIS)
- parse with **`strtof`** (the assignment says `strtof`/`powf`/`sqrtf`, so use `float` everywhere), check that the whole string was consumed (`endptr`)
- goal: check for the comma between x and y
- anything fails to parse -> error message on `stderr` (with `argv[0]`), `EXIT_FAILURE`
- `<goal>` and `<distance>` never change, they get passed 1:1 to every child. Only the 4 limits change.
- ONE helper `parse_point(const char *line, float *x, float *y)` is used for the goal, for every input line (leaf AND split), and returns an error if the comma is missing, a number is bad or there is trailing garbage. Strip the trailing `\n` before parsing.

<br>

## main.c should contain

- parse args (see above)
- read ALL lines until EOF into a dynamic array (`getline` or `fgets` + `realloc`, no idea how many lines come). Initial capacity = macro.
- store the raw text of each line AND the parsed `float x, y` per entry (struct). The child gets exactly the raw text, nothing changes through printing/rounding. The parsed values are used for splitting + duplicate check.
- validate: every line must parse (testcase 09: input `1,` -> error message on `stderr`, `EXIT_FAILURE`). This happens BEFORE any fork, so no children have to be cleaned up.
- base cases (see below)
- split into 4 quadrants + 4 children
- merge the output + `wait`

A SoA per quadrant isn't really needed: one array with all entries, and per child just go through it and write the lines that belong to its quadrant. Less mallocs/frees.

<br>

## Base cases of one process

- 0 lines -> `EXIT_SUCCESS`, print nothing (happens all the time, empty quadrants; not specified in the assignment but the natural reading)
- 1 line -> (already validated) distance = `sqrtf(powf(x - goal_x, 2) + powf(y - goal_y, 2))`. If `<= <distance>` print `%.3f: x,y` (raw text of the line), then `EXIT_SUCCESS`. The distance is always to the goal, never to the pivot.
- n > 1 lines -> **dedup**: keep the FIRST line, drop every later line whose parsed values equal the first line (compare `x`/`y`, not the text: `1.0,1` == `1,1`). The first line itself is kept exactly once.
  - if exactly 1 line is left (= all lines were identical) -> treat it like the 1-line case (distance check + print, `EXIT_SUCCESS`). This is what "additional-1" (`1,1` twice -> `1.414: 1,1`) expects.
  - otherwise split. (The dedup guarantees every child gets fewer lines than the parent has, or at least a smaller rectangle -> no endless recursion.)

Note: only the lines equal to the *first* line are dropped. Duplicates of other points are still passed down; they get dropped on a deeper level when they become the first line of a child.

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

- **Which quadrant does a point belong to:** `x <= mid_x` -> left, else right. `y <= mid_y` -> bottom, else top. **The `<=` is not a free choice:** testcase 03 (easy-2) only gives the expected output order if points on the center line go to the left/bottom quadrant (with `<` the points `2,0` / `2,1` land in the same child as `3,0` and the output order is wrong). Hand-traced the whole recursion for testcase 03 with `<=`: matches the expected output.
- The limits go to the children as text -> print the midpoints with enough digits for `float`: `%.9g`. That round-trips exactly through `strtof`, so the boundaries don't drift on deeper levels. (If I used `double` instead it would be `%.17g`, but then parent and child must also use `double` everywhere. Staying with `float`.)

<br>

## Algorithm (one process)

1. check `argc`, parse args, read `stdin` until EOF, parse + validate every line
2. base cases (0 lines / 1 line / dedup -> maybe 1 line)
3. compute pivot + the 4 limits
4. per child (BL, BR, TL, TR): create the 2 pipes, `fork`. This step only STARTS the 4 children, no writing / reading / waiting yet (see "Step 4 in detail")
5. in the child: close parent-side ends, `dup2`, close the originals, `execlp`
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
- **in the parent, right after each `fork`: close the child's ends** (read end of A, write end of B). Then later children inherit fewer stray ends.
- inherited ends: when forking child 2, the parent still has the parent-side ends of child 1 open (write end of A, read end of B) and child 2 gets a copy of them. If child 2 keeps them, child 1 never sees EOF -> hang. So in every child, FIRST close all parent-side ends (its own and those of the earlier children), THEN `dup2` its own two ends, close the originals, `execlp`.

<br>

## Pipes (per child)

```
parent --write--> [pipe A] --read--> child stdin
parent <--read--- [pipe B] <--write-- child stdout
```

Child, before `execlp` (same program again via `argv[0]`, args = same goal + distance + its own 4 limits):
- close the parent-side ends of its own pipes and of all earlier children (they get inherited!)
- `dup2` read end of A -> `STDIN_FILENO`
- `dup2` write end of B -> `STDOUT_FILENO`
- close the original ends of A and B
- if `execlp` fails -> error message, `EXIT_FAILURE` (use `_exit`/`exit` so the parent's buffered stdio output is not flushed twice; nothing should be buffered at that point anyway)

Parent:
- close the read end of A and the write end of B of each child right after its `fork`
- write the lines of the quadrant into A (raw text + `\n`), then close A. The child only sees EOF after that, and only then it can continue.
- read B in the fixed order, copy to `stdout` until EOF. No parsing needed, it already is the final format.
- **SIGPIPE:** if a child dies early, `write` into its pipe raises SIGPIPE and kills the parent. Use `signal(SIGPIPE, SIG_IGN)` at the start (stays ignored across `exec`, fine) and treat `EPIPE` as an error.
- **I/O (guideline 29):** don't mix stdio and `read`/`write` on the same fd. `getline` on `stdin` + `write(2)` on the pipe fds is fine (different fds). For reading B either `read(2)` + `write(2)` to fd 1, or `fdopen` + stdio, but be consistent per fd. If I print the leaf result with `printf`, `fflush`/exit properly before the process ends.

Hang = most likely a write end that is still open somewhere. Every process closes every end it doesn't use.

Children must run simultaneously: fork all 4 -> feed all 4 -> read all 4 -> wait. NOT fork one -> wait -> fork next.

**Deadlock check (done):** feeding the 4 children one after another cannot deadlock, because every process reads its whole `stdin` to EOF before it produces any output. A child therefore never blocks on writing while the parent is still feeding it, and the parent reads the results only after all inputs are closed.

<br>

## Exit status + error path

- `waitpid` for all 4 children. If one of them is not `EXIT_SUCCESS` (or didn't exit normally) -> `EXIT_FAILURE`
- otherwise `EXIT_SUCCESS`
- free everything (the line array), check return values of `fork`, `pipe`, `dup2`, `execlp`, `write`, `malloc`/`realloc`
- **failure after some children already started** (`pipe`/`fork` fails, `write` fails): don't just `exit`. Close all remaining pipe ends (children then see EOF and end by themselves, no `kill`!), `waitpid` for every child that was already forked, free memory, then `EXIT_FAILURE`. One cleanup function for this, used by all error paths after step 4. No zombies, no orphans.
- error/debug output only to `stderr`. Debug prints only under `#ifdef DEBUG` (`-DDEBUG`), never unconditionally.

<br>

## Resolved questions

| Question | Answer |
|---|---|
| Order of the limits | `min_x max_x min_y max_y` (SYNOPSIS) |
| What happens to the first line | kept once, only later equal lines are dropped |
| All lines identical | the one remaining line is checked + printed if in range ("additional-1") |
| Points on the pivot line | `<=` -> left / bottom (testcase 03) |
| 0 lines | no output, `EXIT_SUCCESS` (natural reading, not specified) |
| Wrong `argc` | usage on `stdout`, `EXIT_FAILURE` (testcase 08) |
| Malformed line like `1,` | error on `stderr`, `EXIT_FAILURE` (testcase 09) |

Still open (optional, ask tutor): "additional-2" expects `2.828: 2,2` for distance 1.5 from the origin, which can't be right (`2,2` is 2.828 away). Its commands are also malformed (`echo -n` with `\n`, `>` instead of `|`). Not run by the submission system, so not treated as ground truth.

<br>

## Order to build it

1. parse args (+ usage) + `parse_point` + read `stdin`, 1 line case works already (leaf, incl. testcases 01, 08, 09)
2. dedup + split without any processes, just print which quadrant each line goes to (check `<=` rule)
3. 1 child with both pipes, get a line through and back
4. 4 children at the same time + merge in order BL, BR, TL, TR
5. exit status + error handling + cleanup path + SIGPIPE
6. Makefile, doc comments, `-DDEBUG` only for debug output

<br>

## Build / style checklist

- compile: `gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c quadtree.c` without warnings; link with `-lm`
- Makefile: `all` first target, `clean` idempotent, header with author + program name
- Doxygen header per file (`@author`, `@brief`, `@details`, `@date`), every function (also `static`) documented
- constants as macros (initial array capacity, number of args = 7, number of children = 4, indices BL/BR/TL/TR), no globals
- no `scanf`, `atoi`, `gets`

<br>

## Tests

- all testcases 01-09 from the assignment, incl. exit codes (`echo $?`)
- testcase 03 first: it is the one that checks the boundary rule and the output order
- `./quadtree 0,0 4.5 -10 10 -10 10 < 1.txt` -> `2.828: 2,-2`, `4.243: -3,3`, `1.414: 1,1` (this order = BR, TL, TR)
- same with `| sort`, compare with `diff`
- empty input, 1 line, 2 identical lines (`1,1` twice -> `1.414: 1,1` for goal 0,0 distance 1.5), points on the center lines, bad args (no comma, letters, wrong `argc`), malformed line in the middle of the input (must fail before any fork)
- "additional" 2500 / 10000 points only after the rest works (deep recursion, many processes)
- `strace -f` to see the forks and pipes, `valgrind --trace-children=yes` for leaks and unclosed fds (`--track-fds=yes`)
- after every run: `pgrep quadtree` must show nothing (no zombies / hanging children)