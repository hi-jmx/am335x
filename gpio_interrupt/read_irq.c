
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/input.h>
#include <unistd.h>


unsigned int gpsCounter[10] = {0};
unsigned int nrCounter[10] = {0};
unsigned int lteCounter[10] = {0};


 
typedef struct _signal_event_s{
	unsigned int ts_GPS; 
	unsigned int ts_LTE; 
	unsigned int ts_NR; 
}signal_event_s;

typedef struct _timerInfo_s {
    int count;
    signal_event_s info[100];
}timerInfo_s;
 
int main(int argc, char *argv[])
{
	int  ret;
	timerInfo_s data;
	int index = 0;

	unsigned int dalte_gps = 0;
	unsigned int dalte_nr = 0;
	unsigned int dalte_lte = 0;


	unsigned int sum_gps = 0;
	unsigned int sum_nr = 0;
	unsigned int sum_lte = 0;

	unsigned int max_gps = 0;
	unsigned int max_nr = 0;
	unsigned int max_lte = 0;

	unsigned int min_gps = 0;
	unsigned int min_nr = 0;
	unsigned int min_lte = 0;

	int dalte = 0;

	//直接将驱动模块当做文件来操作
	int fd = open("/dev/gpio_drv_cls0", O_RDWR);
	if(fd < 0)
	{
		perror("open");
		exit(1);
	}
	int i = 0,j = 0;
	
    while(1)
	{
		ret = read(fd, &data, sizeof(timerInfo_s));
		if(ret < 0)
		{
			perror("read");
			exit(1);
		}
		//解析包
        printf("count %d\n", data.count);
		if(data.count == 1)
		{
			i = index%10;
			gpsCounter[i] = data.info[0].ts_GPS;
			nrCounter[i] = data.info[0].ts_NR;
			lteCounter[i] = data.info[0].ts_LTE;
			index++;
		}
		if(0 == i && index > 1)
		{
			max_gps = 0;
			min_gps = 0;
			sum_gps = 0;

			max_nr = 0;
			min_nr = 0;
			sum_nr = 0;

			max_lte = 0;
			min_lte = 0;
			sum_lte = 0;			


			for(j = 0;j< 10;j++)
			{
				if(max_gps < gpsCounter[i])
				{
					max_gps = gpsCounter[i];
				}
				if(min_gps > gpsCounter[i])
				{
					min_gps = gpsCounter[i];
				}
				sum_gps += gpsCounter[i];

				if(max_nr < nrCounter[i])
				{
					max_nr = nrCounter[i];
				}
				if(min_nr > nrCounter[i])
				{
					min_nr = nrCounter[i];
				}
				sum_nr += nrCounter[i];


				if(max_lte < lteCounter[i])
				{
					max_lte = lteCounter[i];
				}
				if(min_lte > lteCounter[i])
				{
					min_lte = lteCounter[i];
				}
				sum_lte += lteCounter[i];

			}
			dalte_gps = (sum_gps-min_gps-max_gps)/ 8 ;
			dalte_nr = (sum_nr-min_nr-max_nr)/ 8 ;
			dalte_lte = (sum_lte-min_lte-max_lte)/ 8 ;

			dalte = dalte_gps / 100000; // 1us

			printf("gps %d nr %d lte %d  nr %d lte %d\n",dalte_gps,dalte_nr,dalte_lte,(dalte_gps-dalte_nr)/dalte,(dalte_gps-dalte_lte)/dalte);
		}

        usleep(1000*100); // 100ms
	}
	close(fd);
}