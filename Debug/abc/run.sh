    #!/bin/sh
    if P=$(pgrep SYSTEM)
    then
        echo "SYSTEM is running, PID is $P"
    else
        echo "SYSTEM is not running, starting..."
        /root/SYSTEM &
    fi

    if P=$(pgrep RD_SMART)
    then
        echo "RD_SMART is running, PID is $P"
    else
        echo "RD_SMART is not running, starting..."
        /root/RD_SMART &
    fi

    if P=$(pgrep UDP2)
    then
        echo "UDP2 is running, PID is $P"
    else
        echo "UDP2 is not running, starting..."
        /root/UDP2 &
    fi


