#!/usr/bin/env bash

# source this file to learn repair spell

(return 0 2>/dev/null)
if [ $? != 0 ]
then
    echo "usage: source $0"
    echo "This scripts needs to be 'sourced'. Running it without 'source' will not do anything useful!"
    echo "Execute"
    echo "    source $0"
else

    function repairSpell() { 
        tr '[T-Z"a-k.l-p/q-z 0-9A-S_]' '[A-Z"a-k.l-p/q-z 0-9'\'']' < "$1"
    }
    # tr '[Q-Z"a-l/m-r.s-u_w-z 0-9A-P]' '[A-Z"a-k.l-p/q-w'\''x-z 0-9]' < "$1"

    echo "You can now read jumbled text using"
    echo "    repairSpell file"
    echo
    echo "try piping the result into a new file"
fi
