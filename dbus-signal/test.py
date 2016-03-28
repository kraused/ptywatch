#!/usr/bin/python2

import sys
import os

stdin = sys.stdin.read()

with open("test.out", "a+") as f:
	f.write(str((sys.argv, stdin)) + "\n")

