
# Tips & Tricks

?> If you are new to Linux/OSX you should learn these basics, all of these are run through the terminal.

## OSX / Ubuntu

#### Running Background Processes

When starting out I suggest you just have a dozen terminal windows open, for example on my Macbook
I run 4 terminals per desktop in a 2x2 grid. This gives you a good view of what's happening, but as you
progress it may be optimal to run some processes in the background.

Typically we recommend `nohup ./ela 2>error.log 1>/dev/null &` for running a process in the background.

- `./ela` is the executable, replace this with whatever you are trying to run

- This pipes `2` **STDERR** which is the errors to `error.log`

- This pipes `1` **STDOUT** to `/dev/null` suppressing it

You can optionally send **STDOUT** to a file if you want, e.g. `output.log`, or you can send both
the **STDOUT** and **STDERR** to a single file e.g. `nohup ./ela > all.log 2>&1 &`.


#### View Background Processes and Stop Them

