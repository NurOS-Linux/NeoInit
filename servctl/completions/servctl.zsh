#compdef servctl

_servctl() {
    local -a commands
    commands=(
        'status:Check if raesir is alive'
        'list:List all services and their state'
        'start:Launch a service'
        'stop:Stop a running service'
        'restart:Restart a service'
    )

    if (( CURRENT == 2 )); then
        _describe -t commands 'servctl commands' commands
    elif (( CURRENT == 3 )); then
        case $words[2] in
            start|stop|restart)
                # Ideally: _values 'services' $(servctl list | awk 'NR>1 {print $1}')
                _message "service name"
                ;;
        esac
    fi
}

_servctl "$@"
