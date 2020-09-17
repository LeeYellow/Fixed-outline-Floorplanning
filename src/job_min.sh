#!/bin/bash
./main ../testcase/n100.hardblocks ../testcase/n100.pl ../testcase/n100.nets ../output/output100_01.floorplan 0.1
./main ../testcase/n100.hardblocks ../testcase/n100.pl ../testcase/n100.nets ../output/output100_015.floorplan 0.15
./main ../testcase/n200.hardblocks ../testcase/n200.pl ../testcase/n200.nets ../output/output200_01.floorplan 0.1
./main ../testcase/n200.hardblocks ../testcase/n200.pl ../testcase/n200.nets ../output/output200_015.floorplan 0.15
./main ../testcase/n300.hardblocks ../testcase/n300.pl ../testcase/n300.nets ../output/output300_01.floorplan 0.1
./main ../testcase/n300.hardblocks ../testcase/n300.pl ../testcase/n300.nets ../output/output300_015.floorplan 0.15

echo "finish"
