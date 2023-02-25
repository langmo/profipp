sudo ip address add 192.168.0.50/255.255.255.0 dev eth0
sudo ip link set dev eth0 up

cd ./../build/examples/profizumo/
sudo ./profizumo -s -e ./