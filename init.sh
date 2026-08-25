#!/bin/bash

id=`ipcs | grep 3e8 | awk {'print $2'}`

if [ ! -z $id ]; then
    ipcrm -m $id
fi
