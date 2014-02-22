#!/bin/bash

# bring up the host controller
sudo hciconfig hci0 up

# enable non-connectable undirected advertisements
sudo hciconfig hci0 leadv 3

# 0x08       opcode group (LE)
# 0x0008     opcode command (set LE adv data)

# 0b         adv data length
# 02 01 05   flags (LE-only device, non-connectable) 
# 07 09 ...  name
# 00 00 ...  padding (to 32 bytes including length)

sudo hcitool -i hci0 cmd 0x08 0x0008  0b  02 01 05  07 09 66 6f 6f 62 61 7a  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
