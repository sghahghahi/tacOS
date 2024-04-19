# shim – sh improved

Author: Shyon Ghahghahi

## About This Project
<strong>TLDR:</strong> Cooler version of `sh`.

`shim` is a custom shell with support for basic command execution, scripting, command history with history execution, and pipes and redirection. `shim` uses a doubly linked list to keep track of the past 100 user-inputted commands.


### How To Use `shim`
Once inside tacOS, ditch `sh` behind and enter `shim` by running `/shim`:
```
$ make qemu
$ /shim
(prompt appears here)
```
To run an executable script, add it as an argument to `shim` (file extension doesn't have to be `.sh`):
```
$ /shim script.sh
... (command output) ...
```

## Limitations
`shim` has trouble supporting standalone executable scripts –> scripts starting with `#!/shim`. Passing an executable script as an argument to `shim` works without error: `/shim script.sh`.

`shim` currently does not support background jobs.

`shim` also currently does not recognize `#` as a special character if it is not at the beginning of a command.

## Future Plans
`shim` is a work-in-progress and will be continually updated and improved. Keep checking back for more updates and new features.
