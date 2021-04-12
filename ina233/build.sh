#!/bin/bash


#gcc 485.c uart_for_485_LNA.c  -o test_485 




#!/bin/bash

source env.sh

#if [ $# == 1 ];then
	#src=$1
#else
	#src=uart.c
#fi


src="ina233.c"

arm-linux-gnueabihf-gcc -g -o ina233_test  $src