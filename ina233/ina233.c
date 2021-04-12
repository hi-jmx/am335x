#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <endian.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/i2c.h>   
#include <linux/i2c-dev.h>
       

#define D0_CONFIG_OPERATOMG_MODE          0xD0
#define D1_SHUNT_VOLTAGE_MEAS             0xD1        
#define D2_CONFIG_WARNING                 0xD2


#define D0_MFR_ADC_CONFIG_SHUNT_VOLTAGE_CONTINUOUS   0x4125
#define D2_MFR_ALERT_MASK 0xFB





#define INA_ADDRESS         0x40

int main( int argc, char *argv[] )
{
	float value = {0.0};
	int ret = -1;
	int fd = 0;
	int i = 0;
	
	ret = open("/dev/i2c-1", O_RDWR);
	if(ret < 0)
	{
		perror("open i2c-1");
		printf("ret %d",ret);
		return ret;
	}
	
	fd = ret;

	int addr = INA_ADDRESS; /* The I2C address */

	if (ioctl(fd, I2C_SLAVE, addr) < 0) 
	{
		/* ERROR HANDLING; you can check errno to see what went wrong */
		perror("ioctl error");
        ret = -1;
		return ret;
	}

	struct i2c_rdwr_ioctl_data ltc_data;
 	ltc_data.nmsgs=2;
 	ltc_data.msgs=(struct i2c_msg*)malloc(ltc_data.nmsgs*sizeof(struct i2c_msg));
	if(!ltc_data.msgs)
	{
		perror("malloc error");
        ret = -1;
		return ret;
	}

    ioctl(fd,I2C_TIMEOUT, 1); /* 设置超时 */
    ioctl(fd,I2C_RETRIES, 2); /* 设置重试次数 */


	unsigned char tmp_buff[3] = {0};
	ltc_data.nmsgs  = 1;	
	(ltc_data.msgs[0]).len = 3;//buf的长度
	(ltc_data.msgs[0]).flags = 0;//write
	(ltc_data.msgs[0]).addr = INA_ADDRESS;//设备地址
	(ltc_data.msgs[0]).buf = tmp_buff;
	(ltc_data.msgs[0]).buf[0] = D0_CONFIG_OPERATOMG_MODE;//写的地址
	(ltc_data.msgs[0]).buf[1] = (D0_MFR_ADC_CONFIG_SHUNT_VOLTAGE_CONTINUOUS & 0x00ff);//写的value lsb
    (ltc_data.msgs[0]).buf[2] = ((D0_MFR_ADC_CONFIG_SHUNT_VOLTAGE_CONTINUOUS >> 8) & 0xff);//写的value msb

	ret= ioctl(fd, I2C_RDWR, (unsigned long)&ltc_data);  
	if(ret < 0) {  
			perror("writedata error");  
            ret = -1;
			goto fr;  
	} 
	usleep(100000);

    unsigned char tmp_buff1[2] = {0};
    ltc_data.nmsgs  = 1;	
	(ltc_data.msgs[0]).len =2;//buf的长度
	(ltc_data.msgs[0]).flags = 0;//write
	(ltc_data.msgs[0]).addr = INA_ADDRESS;//设备地址
	(ltc_data.msgs[0]).buf = tmp_buff1;
	(ltc_data.msgs[0]).buf[0] = D2_CONFIG_WARNING;//写的地址
	(ltc_data.msgs[0]).buf[1] = D2_MFR_ALERT_MASK;//写的value 

	ret= ioctl(fd, I2C_RDWR, (unsigned long)&ltc_data);  
	if(ret < 0) {  
			perror("writedata error");  
            ret = -1;
			goto fr;  
	} 
	usleep(100000);



    unsigned char tmp_buff2[2] = {0};
    ltc_data.nmsgs  = 1;	
	(ltc_data.msgs[0]).len =2;//buf的长度
	(ltc_data.msgs[0]).flags = 0;//write
	(ltc_data.msgs[0]).addr = INA_ADDRESS;//设备地址
	(ltc_data.msgs[0]).buf = tmp_buff2;
	(ltc_data.msgs[0]).buf[0] = D2_CONFIG_WARNING;//写的地址
	(ltc_data.msgs[0]).buf[1] = D2_MFR_ALERT_MASK;//写的value 

	ret= ioctl(fd, I2C_RDWR, (unsigned long)&ltc_data);  
	if(ret < 0) {  
			perror("writedata error");  
            ret = -1;
			goto fr;  
	} 
	usleep(100000);

    unsigned char databuf[2] = {0};
    while(1)
    {
        ltc_data.nmsgs= 2;  
		ltc_data.msgs[0].len= 1;
		(ltc_data.msgs[0]).flags = 0;  
		ltc_data.msgs[0].addr= INA_ADDRESS;  
		ltc_data.msgs[0].buf[0]= D1_SHUNT_VOLTAGE_MEAS;  //address of read

		ltc_data.msgs[1].len= 2;  
		ltc_data.msgs[1].addr= INA_ADDRESS;  
		ltc_data.msgs[1].flags= 1;     /* read */  
		(ltc_data.msgs[1]).buf = databuf;
		ltc_data.msgs[1].buf[0]= 0; // read buffer0
		ltc_data.msgs[1].buf[1]= 0; // read buffer1

		ret= ioctl(fd, I2C_RDWR, (unsigned long)&ltc_data);  
		if(ret < 0) {  
				perror("readerror");  
				goto fr;
		} 

        printf("buf[0] %d buf[1] %d\n",ltc_data.msgs[1].buf[0], ltc_data.msgs[1].buf[1]);

        sleep(2);
    }







fr:
	free(ltc_data.msgs);

    close( fd );
    return ret;
}

