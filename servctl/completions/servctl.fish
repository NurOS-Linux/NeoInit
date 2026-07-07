# fish completion for servctl

complete -c servctl -f
complete -c servctl -n "__fish_use_subcommand" -a status  -d "Check if raesir is alive"
complete -c servctl -n "__fish_use_subcommand" -a list    -d "List all services and their state"
complete -c servctl -n "__fish_use_subcommand" -a start   -d "Launch a service"
complete -c servctl -n "__fish_use_subcommand" -a stop    -d "Stop a running service"
complete -c servctl -n "__fish_use_subcommand" -a restart -d "Restart a service"

# For dynamic service names if servctl list works:
# complete -c servctl -n "__fish_seen_subcommand_from start stop restart" -a "(servctl list | awk 'NR>1 {print \$1}')"
