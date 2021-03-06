#!/usr/bin/env python

import sys
import subprocess
import os

target = sys.argv[1]
exe = sys.argv[2]
simba_path = sys.argv[3]
runlog = sys.argv[4]
run_end_pattern = sys.argv[5]
run_end_pattern_success = sys.argv[6]
mcu = sys.argv[7]
hex_file = sys.argv[8]
settings_bin = sys.argv[9]
rest = sys.argv[10:]

def run(command):
    """Run a command.
    """
    
    print " ".join(command)
    proc = subprocess.Popen(command, stdout=subprocess.PIPE)
    lines_iterator = iter(proc.stdout.readline, b"")
    with open(runlog, "w") as fout:
        for line in lines_iterator:
            sys.stdout.write(line)
            fout.write(line)
    res = proc.wait()
    if res != 0:
        sys.exit(res)


def upload():
    """Upload.

    """

    run(["avrdude", "-p", mcu]
        + rest
        + ["-U", "eeprom:w:" + settings_bin + ":r"])
    run(["avrdude", "-p", mcu] + rest + ["-U", "flash:w:" + hex_file])


if target == "run":
    upload()
    run([os.path.join(simba_path, "make/run.py"),
         run_end_pattern,
         run_end_pattern_success,
         "38400"])
elif target == "upload":
    upload()
elif target == "dump":
    run(["avrdude", "-p", mcu] + rest + ["-U", "eeprom:r:eeprom.bin:r"])
    run([os.path.join(simba_path, "make/dumpdecoder.py"), "eeprom.bin"])
else:
    print "Bad target " + target
    sys.exit(1)
