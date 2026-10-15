alias cleanupWorkingDir='cd "$(pwd -P)"'

LABYRINTH_MY_OLD_PS1="$PS1"

function hardMode() {
    if [ "$1" = "yes" ]
    then
        PS1="$ "
        alias ls="ls --color=never"
    else
        echo "Hard mode will hide the current working directory from your shell prompt and make ls a little less helpful"
        echo "If you want to try hard mode type:"
        echo
        echo "    hardMode yes"
        echo
        echo "If you want to go back to the normal setup just call hardMode again (without any argument)"
        echo "I will now try to make your setup the same way it was before hardMode, the shell prompt should change"
        PS1="${LABYRINTH_MY_OLD_PS1}"
    fi
}

echo "You can now use"
echo
echo "    cleanupWorkingDir"
echo
echo "try it, it is harmless."
echo
echo "and you can also try switching to"
echo
echo "    hardMode"
