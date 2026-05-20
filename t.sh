#!/bin/bash
rm -f FIFO_*
trap '' SIGUSR2

./animate_server 4 > /tmp/server_out.txt &
sleep 0.3
SPID=$(grep "Server PID:" /tmp/server_out.txt | awk '{print $3}')
echo "Server PID: $SPID"

CLIENT_PID=$$
echo "Client PID: $CLIENT_PID"

kill -USR1 $SPID
sleep 0.5

echo "FIFOs:"
ls FIFO_* 2>/dev/null

# open both ends simultaneously like real client does
exec 3> FIFO_C2S_$CLIENT_PID  # open write end
exec 4< FIFO_S2C_$CLIENT_PID  # open read end

echo "Login ExcitableFabricator" >&3
RESPONSE=$(timeout 2 cat <&4)
echo "Response: '$RESPONSE'"

exec 3>&-
exec 4<&-
killall animate_server 2>/dev/null
rm -f FIFO_*