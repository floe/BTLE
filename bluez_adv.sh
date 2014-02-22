#!/bin/bash

# bring up the host controller
sudo hciconfig hci0 up

# enable non-connectable undirected advertisements
sudo hciconfig hci0 leadv 3

# send custom HCI command:

# 0x08       opcode group (LE)
# 0x0008     opcode command (set LE adv data)

# 0e         adv data length (should be at most 0x15 for compatibility with NRF24L01+)
# 02 01 05   flags (LE-only device, non-connectable) 
# 07 09 ...  name
# 02 ff fe   custom data (02 length, ff type, fe data)
# 00 00 ...  padding (to 32 bytes including length)

sudo hcitool -i hci0 cmd 0x08 0x0008  0e  02 01 05  07 09 66 6f 6f 62 61 72  02 ff fe  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
