#!/bin/bash

# bring up the host controller
sudo hciconfig hci0 up

# enable non-connectable undirected advertisements (only works with recent hciconfig)
#sudo hciconfig hci0 leadv 3

# custom hci commands take two parameters:

# 0x08       opcode group (LE)
# 0x0008     opcode command (set LE adv data)

# set random device address
sudo hcitool -i hci0 cmd 0x08 0x0005 12 34 56 78 9A BC

# set advertising parameters

# 00 08 00 08  min/max adv. interval
# 03           non-connectable undirected advertising
# 01           own address is random (see previous command)
# 00           target address is public (not used for undirected advertising)
# 00 00 00 ... target address (not used for undirected advertising)
# 07           adv. channel map (enable all)
# 00           filter policy (allow any)
sudo hcitool -i hci0 cmd 0x08 0x0006  00 08  00 08  03  01  00  00 00 00 00 00 00  07  00

# enable advertising (00 = disable)
sudo hcitool -i hci0 cmd 0x08 0x000A  01

# set advertisement data (_after_ advertising is enabled)

# 0e         adv data length (should be at most 0x15 for compatibility with NRF24L01+)
# 02 01 05   flags (LE-only device, non-connectable) 
# 07 09 ...  name
# 02 ff fe   custom data (02 length, ff type, fe data)
# 00 00 ...  padding (to 32 bytes including length)
sudo hcitool -i hci0 cmd 0x08 0x0008  0e  02 01 05  07 09 66 6f 6f 62 61 72  02 ff fe  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
