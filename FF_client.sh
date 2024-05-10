#/bin/bash

cnt=0
while [ $cnt -lt 5 ]
    do
        ./client "fifo_$cnt" &
        ((cnt++))
    done

exit
