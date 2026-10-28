#!/usr/bin/env bash
#
# test.sh - tests for supervisor and generator
#
# Usage:   ./test.sh            all tests except the challenging graph
#          ./test.sh --long     also run the challenging graph (can take a while)
#          BEEP=0 ./test.sh     disable the buzzer at the end of the run
#
# Run it from the project root (the directory that contains build/).

SUP=./build/supervisor
GEN=./build/generator
TMP=$(mktemp -d)

PASS=0
FAIL=0
GEN_PIDS=()
SUP_PID=""
SUP_ARGS=()
SUP_RC=0
GENS_OK=0

LONG=0
[ "$1" = "--long" ] && LONG=1

# ---------------------------------------------------------------- graphs
TRIANGLE=(0-1 1-2 2-0)
GRAPH7=(0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 6-0)
ACYCLIC=(1-4 1-5 2-6 3-4 3-6 4-5 6-0 6-5)
CHALLENGE=(0-2 0-9 0-11 1-4 3-2 3-6 4-2 4-9 5-2 5-11 6-2 6-4 7-2 7-4 7-5 7-8 7-16 7-17
           8-9 8-12 8-17 10-2 10-9 11-2 12-1 12-6 12-10 13-5 13-6 13-8 14-4 14-12 15-8
           15-11 15-13 16-1 16-6 16-17 17-6 17-10 17-11 18-7 18-8 18-11)

# ---------------------------------------------------------------- helpers
pass() { printf '  \033[32mPASS\033[0m  %s\n' "$1"; PASS=$((PASS + 1)); }
fail() { printf '  \033[31mFAIL\033[0m  %s\n' "$1"; FAIL=$((FAIL + 1)); }
section() { printf '\n\033[1m== %s ==\033[0m\n' "$1"; }

cleanup()
{
    [ -n "$SUP_PID" ] && kill -9 "$SUP_PID" 2>/dev/null
    for p in "${GEN_PIDS[@]}"; do kill -9 "$p" 2>/dev/null; done
    rm -rf "$TMP"
}
trap cleanup EXIT

# PC speaker buzzer. Uses beep(1) (needs the pcspkr kernel module and write access
# to the speaker device); falls back to the terminal bell if beep is missing or fails.
buzz() # count
{
    [ "${BEEP:-1}" = "0" ] && return 0
    local n=${1:-1} i
    if command -v beep >/dev/null 2>&1; then
        beep -f 120 -l 350 -d 150 -r "$n" 2>/dev/null && return 0
    fi
    for ((i = 0; i < n; i++)); do
        { printf '\a' > /dev/tty; } 2>/dev/null || printf '\a'
        sleep 0.5
    done
}

check_rc()       # name expected actual
{
    if [ "$2" = "$3" ]; then pass "$1 (exit $3)"; else fail "$1 (expected exit $2, got $3)"; fi
}

check_contains() # name file regex
{
    if grep -Eq "$3" "$2"; then pass "$1"; else fail "$1 (no match for '$3' in $(basename "$2"))"; fi
}

check_absent()   # name file regex
{
    if grep -Eq "$3" "$2"; then fail "$1 (found '$3' in $(basename "$2"))"; else pass "$1"; fi
}

check_empty()    # name file
{
    if [ ! -s "$2" ]; then pass "$1"; else fail "$1 ($(basename "$2") is not empty)"; fi
}

# stderr must contain only well-formed "Solution with N edges: u-v u-v ..." lines
check_solution_format() # file
{
    local bad
    bad=$(grep -vcE '^Solution with [0-9]+ edges:( [0-9]+-[0-9]+)*$' "$1")
    if [ "$bad" -eq 0 ]; then pass "stderr contains only well-formed solution lines"
    else fail "stderr has $bad malformed line(s)"; fi
}

# every N must be strictly smaller than the previous N (only improvements are printed)
check_strictly_decreasing() # file
{
    if awk '/^Solution with/ { n = $3; if (have && n >= prev) bad = 1; prev = n; have = 1 }
            END { exit bad }' "$1"; then
        pass "printed solutions are strictly improving"
    else
        fail "a printed solution was not better than the previous one"
    fi
}

# every edge in stderr must exist in the input graph
check_edges_valid() # file edges...
{
    local file=$1 e bad=0
    shift
    local valid=" $* "
    for e in $(grep '^Solution with' "$file" | sed 's/^Solution with [0-9]* edges://'); do
        case "$valid" in *" $e "*) ;; *) bad=1; echo "      unknown edge: $e" ;; esac
    done
    if [ $bad -eq 0 ]; then pass "all printed edges exist in the input graph"
    else fail "printed edges that are not in the input graph"; fi
}

# /dev/shm must look like before the test (no leftover shm / semaphores)
SHM_BASE=$(ls -1A /dev/shm 2>/dev/null | sort)
check_shm() # name
{
    local now
    now=$(ls -1A /dev/shm 2>/dev/null | sort)
    if [ "$now" = "$SHM_BASE" ]; then
        pass "$1: no leftover shared resources"
    else
        fail "$1: /dev/shm changed:"
        diff <(echo "$SHM_BASE") <(echo "$now") | sed 's/^/        /'
        SHM_BASE=$now
    fi
}

# wait up to 5s until all generators from GEN_PIDS are gone; kill the rest
gens_gone()
{
    local i alive p
    for i in 1 2 3 4 5 6 7 8 9 10; do
        alive=0
        for p in "${GEN_PIDS[@]}"; do kill -0 "$p" 2>/dev/null && alive=1; done
        [ $alive -eq 0 ] && return 0
        sleep 0.5
    done
    for p in "${GEN_PIDS[@]}"; do kill -9 "$p" 2>/dev/null; done
    return 1
}

start_gens() # count edges...
{
    local n=$1 i
    shift
    for ((i = 0; i < n; i++)); do
        "$GEN" "$@" >/dev/null 2>&1 &
        GEN_PIDS+=($!)
    done
}

# Starts the supervisor with SUP_ARGS (under timeout), then n generators, waits for the
# supervisor and checks that all generators terminated afterwards.
run_scenario() # timeout_seconds generator_count edges...
{
    local tmo=$1 n=$2
    shift 2
    GEN_PIDS=()
    timeout "$tmo" "$SUP" "${SUP_ARGS[@]}" >"$TMP/stdout" 2>"$TMP/stderr" &
    SUP_PID=$!
    sleep 1
    start_gens "$n" "$@"
    wait "$SUP_PID"
    SUP_RC=$?
    SUP_PID=""
    gens_gone
    GENS_OK=$?
}

# ---------------------------------------------------------------- preflight
for b in "$SUP" "$GEN"; do
    if [ ! -x "$b" ]; then
        echo "$b not found. Run 'make' in the project root first."
        exit 2
    fi
done
command -v timeout >/dev/null || { echo "'timeout' (coreutils) is required"; exit 2; }

# ================================================================ STAGE 1
section "Stage 1: command line (no generator)"

"$SUP" -x >"$TMP/o" 2>"$TMP/e"; rc=$?
check_rc "-x" 1 $rc
check_contains "-x prints usage to stderr" "$TMP/e" 'Usage'
check_absent  "-x prints no usage to stdout" "$TMP/o" 'Usage'
check_shm "-x"

"$SUP" -h >"$TMP/o" 2>"$TMP/e"; rc=$?
check_rc "-h" 0 $rc
check_contains "-h prints usage to stdout" "$TMP/o" 'Usage'
check_shm "-h"

for args in "-n abc" "-n -5" "-n" "-n 1.5" "-w abc" "-w -1" "foo" "-n 5 foo"; do
    # shellcheck disable=SC2086
    "$SUP" $args >"$TMP/o" 2>"$TMP/e"; rc=$?
    check_rc "bad args '$args'" 1 $rc
    check_contains "  usage printed" "$TMP/e" 'Usage'
    check_absent  "  no crash / assert message" "$TMP/e" 'Assertion|Segmentation|AddressSanitizer'
done
check_shm "bad arguments"

timeout 5 "$SUP" -n 0 >"$TMP/o" 2>"$TMP/e"; rc=$?
check_rc "-n 0 terminates on its own" 0 $rc
check_shm "-n 0"

"$SUP" -o /nonexistent_dir/out.txt >"$TMP/o" 2>"$TMP/e"; rc=$?
check_rc "-o with unwritable path" 1 $rc
check_shm "-o unwritable"

# ================================================================ STAGE 2
section "Stage 2: signals (supervisor + 3 generators, triangle)"

for sig in INT TERM; do
    GEN_PIDS=()
    "$SUP" >"$TMP/stdout" 2>"$TMP/stderr" &
    SUP_PID=$!
    sleep 1
    start_gens 3 "${TRIANGLE[@]}"
    sleep 2
    kill -"$sig" "$SUP_PID" 2>/dev/null

    for i in 1 2 3 4 5 6 7 8 9 10; do
        kill -0 "$SUP_PID" 2>/dev/null || break
        sleep 0.5
    done
    if kill -0 "$SUP_PID" 2>/dev/null; then
        fail "SIG$sig: supervisor still running after 5s"
        kill -9 "$SUP_PID" 2>/dev/null
        wait "$SUP_PID" 2>/dev/null
    else
        wait "$SUP_PID"; rc=$?
        check_rc "SIG$sig: supervisor exits" 0 $rc
    fi
    SUP_PID=""
    check_contains "SIG$sig: final message on stdout" "$TMP/stdout" \
        'might not be acyclic, best solution removes [0-9]+ edges\.'
    gens_gone && pass "SIG$sig: all generators terminated" || fail "SIG$sig: generators still running"
    check_shm "SIG$sig"
done

# ================================================================ STAGE 3
section "Stage 3: delay (-w 3 -n 1)"

GEN_PIDS=()
"$SUP" -w 3 -n 1 >"$TMP/stdout" 2>"$TMP/stderr" &
SUP_PID=$!
START=$(date +%s)
sleep 1
start_gens 1 "${TRIANGLE[@]}"
sleep 1
check_empty "nothing read after ~2s (delay still running)" "$TMP/stderr"
wait "$SUP_PID"; rc=$?
SUP_PID=""
END=$(date +%s)
check_rc "supervisor exits" 0 $rc
if [ $((END - START)) -ge 3 ]; then pass "ran at least 3s"; else fail "finished after $((END - START))s, expected >= 3s"; fi
check_contains "solution printed after the delay" "$TMP/stderr" '^Solution with'
gens_gone && pass "generator terminated" || fail "generator still running"
check_shm "delay"

# ================================================================ STAGE 4
section "Stage 4: streams and -o (-n 1)"

SUP_ARGS=(-n 1 -o "$TMP/out.txt")
run_scenario 20 1 "${TRIANGLE[@]}"
check_rc "supervisor stops by itself" 0 $SUP_RC
check_contains "stderr has a solution line"            "$TMP/stderr" '^Solution with [0-9]+ edges:'
check_absent  "stderr has no debug output"             "$TMP/stderr" 'woke up'
check_absent  "stderr has no final message"            "$TMP/stderr" 'might not be acyclic|is acyclic'
check_contains "stdout has the final message"          "$TMP/stdout" 'might not be acyclic, best solution removes [0-9]+ edges\.'
check_absent  "stdout has no solution lines"           "$TMP/stdout" '^Solution with'
check_contains "out.txt has the solution line"         "$TMP/out.txt" '^Solution with [0-9]+ edges:'
check_solution_format "$TMP/stderr"
[ $GENS_OK -eq 0 ] && pass "generator terminated" || fail "generator still running"
check_shm "-n 1 -o"

# ================================================================ STAGE 5
section "Stage 5: assignment examples"

echo "-- triangle, 10 generators, -n 3000"
SUP_ARGS=(-n 3000)
run_scenario 120 10 "${TRIANGLE[@]}"
check_rc "supervisor stops after 3000 solutions" 0 $SUP_RC
check_contains "final message: best solution removes 1 edges" "$TMP/stdout" 'removes 1 edges\.'
check_solution_format "$TMP/stderr"
check_strictly_decreasing "$TMP/stderr"
check_edges_valid "$TMP/stderr" "${TRIANGLE[@]}"
[ $GENS_OK -eq 0 ] && pass "all generators terminated" || fail "generators still running"
check_shm "triangle"

echo "-- graph from the first page, 10 generators, -n 3000"
SUP_ARGS=(-n 3000)
run_scenario 120 10 "${GRAPH7[@]}"
check_rc "supervisor stops after 3000 solutions" 0 $SUP_RC
check_contains "final message: best solution removes 1 edges" "$TMP/stdout" 'removes 1 edges\.'
check_strictly_decreasing "$TMP/stderr"
check_edges_valid "$TMP/stderr" "${GRAPH7[@]}"
[ $GENS_OK -eq 0 ] && pass "all generators terminated" || fail "generators still running"
check_shm "graph 7 nodes"

echo "-- acyclic graph, 10 generators, no limit"
SUP_ARGS=()
run_scenario 120 10 "${ACYCLIC[@]}"
check_rc "supervisor ends by itself (not killed by timeout)" 0 $SUP_RC
check_contains "stdout: The graph is acyclic!" "$TMP/stdout" '^The graph is acyclic!$'
check_absent  "stdout: no 'might not be acyclic'" "$TMP/stdout" 'might not be acyclic'
check_edges_valid "$TMP/stderr" "${ACYCLIC[@]}"
[ $GENS_OK -eq 0 ] && pass "all generators terminated" || fail "generators still running"
check_shm "acyclic"

if [ $LONG -eq 1 ]; then
    echo "-- challenging graph (acyclic), 10 generators, no limit, timeout 300s"
    SUP_ARGS=()
    run_scenario 300 10 "${CHALLENGE[@]}"
    check_rc "supervisor ends by itself (not killed by timeout)" 0 $SUP_RC
    check_contains "stdout: The graph is acyclic!" "$TMP/stdout" '^The graph is acyclic!$'
    check_edges_valid "$TMP/stderr" "${CHALLENGE[@]}"
    [ $GENS_OK -eq 0 ] && pass "all generators terminated" || fail "generators still running"
    check_shm "challenging graph"
else
    echo "-- challenging graph skipped (run './test.sh --long')"
fi

# ---------------------------------------------------------------- summary
section "Summary"
printf '  passed: %d   failed: %d\n' "$PASS" "$FAIL"

# buzzer: one buzz if everything passed, three if something failed
if [ "$FAIL" -eq 0 ]; then
    buzz 1
    exit 0
else
    buzz 3
    exit 1
fi