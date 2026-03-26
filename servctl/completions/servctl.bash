_servctl_completion() {
    local cur prev opts
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    opts="status list start stop restart help"

    if [[ ${COMP_CWORD} -eq 1 ]]; then
        COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
        return 0
    fi

    # For commands that take a service name, we could ideally
    # query servctl list, but for now we'll leave it to the user.
    case "${prev}" in
        start|stop|restart)
            # Potentially: COMPREPLY=( $(compgen -W "$(servctl list | awk '{print $1}')" -- ${cur}) )
            ;;
    esac
}
complete -F _servctl_completion servctl
