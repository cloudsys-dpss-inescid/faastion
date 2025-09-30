# file: run_completion
# run parameter-completion

_complete() { #  By convention, the function name
  #+ starts with an underscore.

  # Pointer to current completion word.
  local cur

  COMPREPLY=() # Array variable storing the possible completions.
  cur=${COMP_WORDS[COMP_CWORD]}
  case "$cur" in
  t | te | tes | test)
    while IFS="" read -r line; do COMPREPLY+=("$line"); done < <(compgen -W 'test tests' -- "$cur")
    ;;
  tests)
    while IFS="" read -r line; do COMPREPLY+=("$line"); done < <(compgen -W 'tests' -- "$cur")
    ;;
  p*)
    while IFS="" read -r line; do COMPREPLY+=("$line"); done < <(compgen -W 'plot' -- "$cur")
    ;;
  i*)
    while IFS="" read -r line; do COMPREPLY+=("$line"); done < <(compgen -W 'install-deps' -- "$cur")
    ;;
  me*)
    while IFS="" read -r line; do COMPREPLY+=("$line"); done < <(compgen -W 'measurements' -- "$cur")
    ;;
  mo*)
    while IFS="" read -r line; do COMPREPLY+=("$line"); done < <(compgen -W 'monitoring' -- "$cur")
    ;;
  b*)
    while IFS="" read -r line; do COMPREPLY+=("$line"); done < <(compgen -W 'build' -- "$cur")
    ;;
  d*)
    while IFS="" read -r line; do COMPREPLY+=("$line"); done < <(compgen -W 'deploy' -- "$cur")
    ;;
  h*)
    while IFS="" read -r line; do COMPREPLY+=("$line"); done < <(compgen -W 'help' -- "$cur")
    ;;
  esac

  return 0
}

complete -F _complete -o filenames run
