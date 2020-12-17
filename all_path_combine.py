# -*- coding: utf-8 -*-

import argparse
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('first', type=str, help='First file path, i.e. part0')

args = parser.parse_args()

first_path = Path(args.first)
parent = first_path.parent
name = first_path.stem

result_file = parent/name

sequence = 0
while True:
    part_path = parent/(name+".part"+str(sequence))
    if part_path.exists() is False:
        break

    with open(result_file, 'a') as result_file_fp, open(part_path) as part_file_fp:
        result_file_fp.write(part_file_fp.read())

    sequence += 1
