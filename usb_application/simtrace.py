#!/usr/bin/env python 

import argparse
import sniffer
import ccid

import usb.core
import usb.util

import hashlib
import os
import random
import re

cmd1 = {0x00, 0x10, 0x00, 0x00}
cmd2 =  {0x00, 0x20, 0x00, 0x00, 0x02}
cmd_poweron = {0x62, 0x62, 0x00, 0x00}
cmd_poweroff = {0x63, 0x63, 0x00, 0x00}
cmd_get_slot_stat = {0x65, 0x65, 0x00, 0x00}
cmd_get_param = {0x00, 0x6C, 0x00, 0x00}

class find_class(object):
    def __init__(self, class_):
        self._class = class_
    def __call__(self, device):
        # first, let's check the device
        if device.bDeviceClass == self._class:
            return True
        # ok, transverse all devices to find an
        # interface that matches our class
        for cfg in device:
            # find_descriptor: what's it?
            intf = usb.util.find_descriptor(
                                        cfg,
                                        bInterfaceClass=self._class
                                )
            if intf is not None:
                return True

        return False


# main code
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-C", "--conf", type=int, choices=[1, 2, 3], help="Set USB config")
    parser.add_argument("-b", "--read_bin", help="read ICCID, IMSI, etc.", action='store_true')
    parser.add_argument("-c", "--cmd", help="cmds to send to sim card (Not supported yet)", 
        choices=["cmd1", "cmd2", "cmd_poweron", "cmd_poweroff", "cmd_get_slot_stat", "cmd_get_param"])
    parser.add_argument("-s", "--sniff", help="Sniff communication!", action='store_true') 
    
    args = parser.parse_args()
    print("args: ", args)

# FIXME: why is it a ccid function?
    if args.conf is not None:
        devs = usb.core.find(find_all=1, custom_match=find_class(0xb))  # 0xb = Smartcard
        for dev in devs:
            dev.set_configuration(args.conf)

    if args.read_bin is True: 
        ccid.pySim_read() 

    if args.cmd is not None:
        devs = usb.core.find(find_all=1, custom_match=find_class(0xb))  # 0xb = Smartcard
        for dev in devs:
            dev.write(0x1, args.cmd)
            ret = dev.read(0x82, 64)
            print(ret)
    if args.sniff is True:
        sniffer.sniff()
    return

#    (epi, epo) = find_eps(dev)
    while True:
        #ep_out.write("Hello")
        try:
            ans = dev.read(0x82, 64, 1000)
            print("".join("%02x " % b for b in ans))
        except KeyboardInterrupt:
            print("Bye")
            sys.exit()
        except: 
            print("Timeout")
    #    print(ep_in.read(1, 5000));

main()