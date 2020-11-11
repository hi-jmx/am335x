#!/bin/bash


#gcc 485.c uart_for_485_LNA.c  -o test_485 




#!/bin/bash

source env.sh

#if [ $# == 1 ];then
	#src=$1
#else
	#src=uart.c
#fi


#src_recog="recognize.c hz_log.c hz_uart.c"
#arm-linux-gnueabihf-gcc -g -o recog  $src_recog -lpthread -l rt -DPLATFORM_AM335X

src_spi="read_irq.c"
arm-linux-gnueabihf-gcc -g -Wall -o read_irq  $src_spi   




