#!/bin/bash

test_cfg="./offmsg_tests.conf"
test_cmd="./offmsg_tests"
msg_from_file="./offmsg_from.tmp"
msg_to_file="./offmsg_to.tmp"

invalid_val="undefined"
sender_addr=$invalid_val
sender_userid=$invalid_val
receiver_addr=$invalid_val
receiver_userid=$invalid_val

function init_tests_node()
{
    local result=`$test_cmd --init --$1 -c $test_cfg`

    if [ $? -ne 0 ]; then
        return -1
    fi

    case $1 in
    "sender")
        sender_addr=`echo $result | awk '{print $1}'`
        sender_userid=`echo $result | awk '{print $2}'`

        if [ $sender_addr = $invalid_val -o $sender_userid = $invalid_val ]; then
            return -1
        fi
        ;;

    "receiver")
        receiver_addr=`echo $result | awk '{print $1}'`
        receiver_userid=`echo $result | awk '{print $2}'`

        if [ $receiver_addr = $invalid_val -o $receiver_userid = invalid_val ]; then
            return -1
        fi
        ;;
    esac

    return 0
}

function add_friend()
{
    `$test_cmd --add-friend --$1 -a $2 -u $3 -c $test_cfg`

    if [ $? -ne 0 ]; then
        return -1
    else
        return 0
    fi
}

function make_friends()
{
    local pids=''
    local pid=0
    local result=0

    add_friend sender $receiver_addr $receiver_userid &
    pid=$!
    pids="$pids $pid"

    add_friend receiver $sender_addr $sender_userid &
    pid=$!
    pids="$pids $pid"

    for pid in $pids
    do
        wait $pid
        result=($? -a $result)
    done

    if [ $result -ne 0 ]; then
        return -1
    else
        return 0
    fi
}

function message()
{
   `$test_cmd --message --$1 --refmsg $2 -u $3 -c $test_cfg`

   if [ $? -ne 0 ]; then
        return -1
   else
        return 0
   fi
}

init_tests_node sender
if [ $? -ne 0 ]; then
    echo "Sender got address and user ID unsuccessfully"
    exit
else
    echo "Sender got address and user ID successfully"
fi

echo "sender address:$sender_addr"
echo "sender user id:$sender_userid"

init_tests_node receiver
if [ $? -ne 0 ]; then
    echo "Receiver got address and user ID unsuccessfully"
    exit
else
    echo "Receiver got address and user ID successfully"
fi

echo "receiver address:$receiver_addr"
echo "receiver user id:$receiver_userid"

make_friends
if [ $? -ne 0 ]; then
    echo "Sender and Receiver made friends with each other unsuccessfully"
    exit
else
    echo "Sender and Receiver made friends with each other successfully"
fi

echo "Last week, I went to the theater." > $msg_from_file
echo "I had a very good seat." >> $msg_from_file
echo "The play was very interesting." >> $msg_from_file

message sender $msg_from_file $receiver_userid
if [ $? -ne 0 ]; then
    echo "Sender sent offline messages unsuccessfully"
    rm -f $msg_from_file
    exit
else
    echo "Sender sent offline messages successfully"
fi

message receiver $msg_to_file $sender_userid
if [ $? -ne 0 ]; then
    echo "Receiver received offline messages unsuccessfully"
    rm -f $msg_from_file
    rm -f $msg_to_file
    exit
else
    echo "Receiver received offline messages successfully"
fi

diff $msg_from_file $msg_to_file
if [ $? -ne 0 ]; then
    echo "Test case ran unsuccessfully"
else
    echo "Test case ran successfully"
fi

rm -f $msg_from_file
rm -f $msg_to_file