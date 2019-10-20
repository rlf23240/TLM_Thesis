#!/bin/bash

# Use for replacing `/r/n' to `/n'.

for entry in "$1"/*
do
  perl -pi.bak -e 's/\r//g' "$entry"
done
