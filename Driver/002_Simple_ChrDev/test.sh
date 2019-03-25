#!/bin/bash

gcc test.c -o test

make clean all

insmod simple_chrdev.ko

mknod /dev/simple_cdev c 224 0

./test

rmmod simple_chrdev

rm /dev/simple_cdev

make clean

rm test
