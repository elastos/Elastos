## Overview

This document introduce **how-to** to use scripts and binary **offmsg-tests** to construct tests-suites to test offline-message features.

## Usage:

With **-h** or **—help** option, check the usage of command **offmsg_tests**.

```shell
$ offmsg_tests -h | --help
usage: offmsg_tests [options] --sender
   or: offmsg_tests [options] --receiver

    -c, --config[=<config-path>] test config file
    --init                       create carrier instance, and print
                                 its address and userid.

    --sender                     the role to send offline messages
    --receiver                   the role to receive offline messages

    -a, --remote-address=<remote-address>
                                 the address of remote tests node
    -u, --remote-userid=<remote-userid>
                                 the userid of remote tests node

    --refmsg-from=<offmsg-text>  the text file that sender read from
                                 and send it as offline message to
                                 remote tests node.
    --refmsg-to=<offmsg-text>    the text file that receiver store
                                 offline message into after receiving
                                 them from tests node.
```

## How to test

### Step-1

Try to create a carrier node instance as sender tests, and output it's address and userid,  then exit right away.

```shell
$ offmsg_tests --init --sender -c offmsg.conf
address: SZ3Ugfv54V3CR4vWyDvvwBf33sugFxX3bVT7WaxgPoPBym58mP4A
userid: CdHewREbmAS4MG3t2BhKogYBYkRRp3rTe1H72Q6K4diM
```

### Step-2

Create another carrier node instance as receiver tests, and output its address and userid,  then exit right away.

```shell
$ offmsg_tests --init --receiver -c offmsg.conf
address: GgYffuAF3XXwD5VgQhfQ51Hjw5Kt6CyoSxC7DgXic9GGzMpo9kTC
userid: 88pQwxiAAXzQuCpMCUmvoKMtdPx1ouadwyFzcvUJCAb4
```
### Step-3

With receiver's address and userid, start running sender tests node to make a friend with receiver node if they are not friend yet.

```shell
$ offmsg_tests --sender -c offmsg.conf \
              -a GgYffuAF3XXwD5VgQhfQ51Hjw5Kt6CyoSxC7DgXic9GGzMpo9kTC \
              -u 88pQwxiAAXzQuCpMCUmvoKMtdPx1ouadwyFzcvUJCAb4
```

### Step-4

With sender's address and userid, start running receiver tests node to make a friend with sender node if they are not friend yet while sender tests is running.

```shell
$ offmsg_tests --receiver -c offmsg.conf \
              -a SZ3Ugfv54V3CR4vWyDvvwBf33sugFxX3bVT7WaxgPoPBym58mP4A \
              -u CdHewREbmAS4MG3t2BhKogYBYkRRp3rTe1H72Q6K4diM
```

As long as sender node and receiver node become friends to each, then both of them exit gracefully.

### Step-5

Now, start to run sender tests node to send offline message to receiver node, which for now is being offline state.

```shell
$ offmsg_tests --sender -c offmsg.conf \
              --refmsg-from=offmsg.txt \
              -a GgYffuAF3XXwD5VgQhfQ51Hjw5Kt6CyoSxC7DgXic9GGzMpo9kTC \
              -u 88pQwxiAAXzQuCpMCUmvoKMtdPx1ouadwyFzcvUJCAb4
```

Where sender node will read message from **offmsg.txt** one by one in line and send it as offline message to receiver node. After finishing sending offline messages, then  sender node exit gracefully.

### Step-6

It\'s turn to start receiver tests node and try to receive offline messages while sender node is being offline now.

```shell
$ offmsg_tests --receiver -c offmsg.conf \
              --refmsg-to=offmsg-to.txt \
              -a SZ3Ugfv54V3CR4vWyDvvwBf33sugFxX3bVT7WaxgPoPBym58mP4A \
              -u CdHewREbmAS4MG3t2BhKogYBYkRRp3rTe1H72Q6K4diM
```

As long as receiver node received message, it will append the message to **offmsg-to.txt**. After a while no messages could be received, then exit.

### Step-7

Use **diff** tool to check missing offline message or not.

```shell
$ diff offmsg.txt offmsg-to.txt
```

## Scripts

Run the shell script **offmsg-tests.sh** to test offline message with whole steps listed above.
```shell
$ ./offmsg-tests.sh
```

On windows, change to run the batch script **offmsg-tests.bat**

```powershell
$ offmsg-tests.bat
```

