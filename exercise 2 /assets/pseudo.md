FUNCTION main(argc, argv):
    1. Check if argc == 7. If not, print usage to stdout and exit(EXIT_FAILURE)[cite: 3, 8].
    2. Parse goal position (<x>, <y>) and distance using strtof. 
       - Check for the comma separator between x and y[cite: 2].
       - Parse canvas limits (min_x, max_x, min_y, max_y) using strtof.
       - If any parsing fails, print error to stderr and exit(EXIT_FAILURE)[cite: 2, 3, 8].

    3. Read all lines from stdin into a dynamic array:
       - For each line, strip newline and parse it into a point (x, y).
       - Validate that every line parses correctly. If a line is malformed, print error to stderr and exit(EXIT_FAILURE)[cite: 3, 8, 10].

    4. BASE CASE - 0 lines:
       - If total lines == 0, exit(EXIT_SUCCESS).

    5. BASE CASE - 1 line (or duplicate check):
       - Sort/filter out lines that are exact duplicates of the FIRST line's coordinates.
       - If only 1 unique line remains:
           * Calculate distance from that point to the goal using sqrtf and powf[cite: 2, 3].
           * If distance <= max_distance, print "distance: x,y" to stdout[cite: 2, 3].
           * exit(EXIT_SUCCESS)[cite: 2, 3].

    6. RECURSIVE STEP (n > 1 lines):
       - Compute midpoints of the current bounding box:
           mid_x = (min_x + max_x) / 2
           mid_y = (min_y + max_y) / 2

       - Set up 4 quadrants (BL, BR, TL, TR) arrays to hold points belonging to each region.
         (Remember: points on the boundary line <= mid go to left/bottom)[cite: 10].

       - For each quadrant (i from 0 to 3):
           a. Create two unnamed pipes: Pipe A (parent -> child), Pipe B (child -> parent)[cite: 3, 10].
           b. fork() the process[cite: 3, 10].
           c. IF CHILD:
               - Close unneeded pipe ends inherited from previous children[cite: 10].
               - Use dup2() to redirect Pipe A's read end to STDIN, and Pipe B's write end to STDOUT[cite: 3, 10].
               - Close original pipe fds.
               - Call execlp() to run this program again with the new quadrant's limits[cite: 2, 3, 10].
           d. IF PARENT:
               - Close the child-side ends of the pipes.
               - Save the child's PID and the write-end of Pipe A and read-end of Pipe B.

       - PARENT WRITING:
           - For each child, write its quadrant's points down Pipe A, then close Pipe A's write end (signals EOF to child)[cite: 10].

       - PARENT READING & MERGING:
           - Read from children's Pipe B in the strict required order: 
             **Bottom-Left -> Bottom-Right -> Top-Left -> Top-Right**[cite: 3, 10].
           - Copy what you read directly to stdout[cite: 10].

       - CLEANUP & WAITING:
           - Call waitpid() for all 4 children[cite: 3, 10].
           - If any child exited with a non-success status, exit(EXIT_FAILURE)[cite: 3, 10].
           - Otherwise, exit(EXIT_SUCCESS)[cite: 3, 10].