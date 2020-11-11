/***********************************************************************
 * bref : irq test
 * usage: install mod : insmod modName.ko
 *         remove mod : rmmod modName
 *           show mod : lsmod  
 *           show log : dmesg
 * *********************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>  
#include <asm/irq.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/time.h>
#include <linux/ioport.h>//io端口头文件

#include <linux/hrtimer.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/ktime.h>

#include "dmtimer.h"
#include "interrupt.h"
#include "soc_AM335x.h"
#include "gpio_irq_test.h"


#define GPIO_PIN_GPS (0*32+22)

#define GPIO_PIN_NR (1*32+18)

#define GPIO_PIN_LTE (0*32+19)

#define IRQF_TRIGGER_NONE	 0x00000000
#define IRQF_TRIGGER_RISING	 0x00000001
#define IRQF_TRIGGER_FALLING 0x00000002
#define IRQF_TRIGGER_HIGH	 0x00000004
#define IRQF_TRIGGER_LOW	 0x00000008

      
#define TIMER7_BASE SOC_DMTIMER_7_REGS
#define TIMER7_CLKSEL_BASE 0x44E00500

static int gpio_drv_release(struct inode *inode, struct file  *file);
static int gpio_drv_open(struct inode *inode, struct file  *file);
static int gpio_drv_read(struct file *filp, char __user *buf,  size_t count, loff_t *fpos);

static struct file_operations gpio_drv_fops = {
    .owner = THIS_MODULE,
    .open = gpio_drv_open,
    .read = gpio_drv_read,
    .release = gpio_drv_release,    //里面添加free_irq函数,来释放中断服务函数
};

typedef struct _signal_event_s{
	unsigned int ts_GPS; 
	unsigned int ts_LTE; 
	unsigned int ts_NR; 
}signal_event_s;

typedef struct _timerInfo_s {
    int count;
    signal_event_s info[100];
}timerInfo_s;

timerInfo_s timerInfo;

static int gpio_drv_major = 0;
static struct class *gpio_drv_cls;

/*高精度定时器*/
static struct hrtimer hr_timer;
ktime_t ktime;

unsigned long in32(unsigned long addr)
{
	unsigned long *p=(unsigned long *)addr;
	return *p;
}

void out32(unsigned long addr,unsigned long data)
{
	unsigned long *p=(unsigned long *)addr;
	*p=data;
}

enum hrtimer_restart my_hrtimer_callback(struct hrtimer *timer)
{
    // struct timespec ts;
    
    // ts = current_kernel_time();
    // if(timerInfo.count < 10000)
    // {
    //     timerInfo.info[timerInfo.count].ts_GPS = ts;
    //     timerInfo.info[timerInfo.count].ts_LTE = ts;
    //     timerInfo.info[timerInfo.count].ts_NR = ts;
    //     timerInfo.count++;
    // }
    
    hrtimer_start( timer, ktime, HRTIMER_MODE_REL );  // 一直运行下去
    return HRTIMER_RESTART;
}

static int __init my_hrtimer_init(void)
{
    printk( KERN_ALERT "HR Timer module installing\n");

    ktime = ktime_set( 3, 100*1000*1000);  // 3.100ms
    hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    hr_timer.function = &my_hrtimer_callback;
    hrtimer_start( &hr_timer, ktime, HRTIMER_MODE_REL );

    return 0;
}

static void __exit my_hrtimer_exit(void)
{
    int ret;
    ret = hrtimer_cancel(&hr_timer); // 取消定时器执行
    if(ret)
    {
        printk( KERN_ALERT "The timer was still in use...\n");
    }
    printk( KERN_ALERT "HR Timer module uninstalling\n");
    return;
}


int  gpio_drv_release(struct inode *inode, struct file  *file)  //卸载中断
{
    printk("--------^_*  %s-------\n", __FUNCTION__);
    return 0;
}

static int gpio_drv_open(struct inode *inode, struct file  *file)
{      
    printk("--------^_*  %s-------\n", __FUNCTION__);
    return 0;
}

static int gpio_drv_read(struct file *filp, char __user *buf,  size_t count, loff_t *fpos)
{
    int ret;
    // printk("--------^_*  %s-------\n", __FUNCTION__);
    ret = copy_to_user(buf, &timerInfo, count);
	if(ret > 0)
	{
		printk("copy_to_user error\n");
		return -EFAULT;
	}
    timerInfo.count = 0;
    return 0;
} 

struct timespec ts_gps;
struct timespec last_ts;

unsigned int GetTimer7Count(int reset)
{
    unsigned int *var1, value;
    var1 = (unsigned int *)ioremap(TIMER7_BASE+0x3C, sizeof(var1));
    value = *var1;
    if(1 == reset)
    {
        // printk(KERN_INFO"1 == reset");
        *var1 = 0;
    }
    return value;
}

static irqreturn_t  irq_callback_gps (int irq, void *dev_id)       //中断服务函数
{
    if(timerInfo.count < 100)
    {

        timerInfo.info[timerInfo.count].ts_GPS = GetTimer7Count(1);
        timerInfo.count++;
    }

    
    
    //GetTimer7Count();
    // hrtimer_restart(hr_timer);
    return IRQ_HANDLED;
}

static irqreturn_t  irq_callback_nr (int irq, void *dev_id)       //中断服务函数
{
     
    
    timerInfo.info[timerInfo.count].ts_NR = GetTimer7Count(0);
    // hrtimer_restart(hr_timer);
    return IRQ_HANDLED;
}


static irqreturn_t  irq_callback_lte (int irq, void *dev_id)       //中断服务函数
{
    
    timerInfo.info[timerInfo.count].ts_LTE = GetTimer7Count(0);
    // hrtimer_restart(hr_timer);
    return IRQ_HANDLED;
}

void Timer7_config(void)
{
    volatile unsigned int *var1 = 0;
    volatile unsigned int *var2 = 0;
    volatile unsigned int *var3 = 0;
    volatile unsigned int *var4 = 0;
    volatile unsigned int *var5 = 0;


    var1 = (volatile unsigned int *)ioremap(0x44E00000+0x7C, sizeof(var1));
    writel(0x00000002, var1);
    printk("0x44E00000 + 0x7C = %x\n", *var1);
    var2 = (volatile unsigned int *)ioremap(TIMER7_BASE + 0x10, sizeof(var2));
    writel(0x00000002, var2);
    printk("TIMER7_BASE + 0x10 = %x\n", *var2);
    var3 = (volatile unsigned int *)ioremap(TIMER7_BASE + 0x40, sizeof(var3));
    writel(0x00000000, var3);
    printk("TIMER7_BASE + 0x40 = %x\n", *var3);
    var4 = (volatile unsigned int *)ioremap(TIMER7_BASE + 0x38, sizeof(var4));
    writel(0x00000003, var4);
    printk("TIMER7_BASE + 0x38 = %x\n", *var4);
    var5 = (volatile unsigned int *)ioremap(TIMER7_CLKSEL_BASE + 0x04, sizeof(var5));
    writel(0x00000001, var5);
    printk("TIMER7_CLKSEL_BASE + 0x04 = %x\n", *var5);
    
    // writel(0x00000002, 0x44E00000 + 0x7C);//CM_PER->M_PER_TIMER7_CLKCTRL,开启定时器
    // printk("OCP Configuration Register = %x\n", readl(TIMER7_BASE + 0x10));
    // writel(0x00000002, TIMER7_BASE + 0x10);//DMTIMER7->Timer OCP Configuration Register,定时器自由运行
    // writel(0x00000000, TIMER7_BASE + 0x40);//DMTIMER7->Timer Load Register, timer counter register start value after overflow
    // writel(0x00000003, TIMER7_BASE + 0x38);//DMTIMER7->Timer Control Register，Auto-Reload,& Start Timer

    // //时钟选择，默认为高速晶振
    // writel(0x00000001, TIMER7_CLKSEL_BASE + 0x04);//CM_DPLL->CLKSEL_TIMER7_CLK,时钟源选择，默认复位即为选贼高速晶振
}



int gpio_interrupt_start(int pin, irq_handler_t handler,char *name)
{
    int ret = 0;
    ret = gpio_request(pin, name);  
    if(ret < 0)
    {
        printk(KERN_ERR"gpio_irq_test: Failed to request the %s",name); 
        return ret;
    }else
    {
        printk(KERN_INFO"gpio_irq_test: success to request the %s!",name); 

    }
    
    ret = gpio_direction_input(pin);
    if(ret < 0)
    {
        printk(KERN_ERR"gpio_irq_test: fail to set gpio_direction_input %s\n",name);
        return ret;
    }else
    {
        printk(KERN_INFO"gpio_irq_test: success to set gpio_direction_input %s\n",name);
    }
    
    
    

    //参数1--中断号码
    //获取中断号码的方法: 
    //1,外部中断IRQ_EINT(x)  
    //2, 头文件 #include <mach/irqs.h>  #include <plat/irqs.h>
	//参数2--中断的处理方法
	//参数3--中断触发方式:  高电平，低电平，上升沿，下降沿，双边沿
	//参数4--中断的描述-自定义字符串
	//参数5--传递个参数2的任意数据
	//返回值0表示正确
    ret = request_irq(gpio_to_irq(pin), handler, IRQF_TRIGGER_RISING , name, 0);
    if(ret < 0)
    {
        printk(KERN_ERR"gpio_irq_test: fail to set request_irq %s\n",name);
        return ret;
    }else
    {
        printk(KERN_INFO"gpio_irq_test: success to set request_irq %s\n",name);
    }
}



static int gpio_drv_init(void)
{
    int ret;
    // 1， 申请主设备号--动态申请主设备号
	gpio_drv_major = register_chrdev(0, "gpio_drv_dev", &gpio_drv_fops);
 
	// 2, 创建设备节点
	gpio_drv_cls = class_create(THIS_MODULE, "gpio_drv_cls");
	device_create(gpio_drv_cls, NULL,  MKDEV(gpio_drv_major, 0),  NULL, "gpio_drv_cls0");

    Timer7_config();

    ret = gpio_interrupt_start(GPIO_PIN_NR, irq_callback_nr, "GPIO_PIN_NR");
    ret = gpio_interrupt_start(GPIO_PIN_GPS, irq_callback_gps, "GPIO_PIN_GPS");  
    ret = gpio_interrupt_start(GPIO_PIN_LTE, irq_callback_lte, "GPIO_PIN_lte");
    


    
    // my_hrtimer_init();
    return 0;
}

static void gpio_drv_exit(void)
{
    gpio_free(GPIO_PIN_GPS); 
    free_irq(gpio_to_irq(GPIO_PIN_GPS), 0);

    gpio_free(GPIO_PIN_NR); 
    free_irq(gpio_to_irq(GPIO_PIN_NR), 0);

    gpio_free(GPIO_PIN_LTE); 
    free_irq(gpio_to_irq(GPIO_PIN_LTE), 0);

    device_destroy(gpio_drv_cls, MKDEV(gpio_drv_major, 0)); 
    class_destroy(gpio_drv_cls); 
    my_hrtimer_exit();
    return ;
}



module_init(gpio_drv_init);
module_exit(gpio_drv_exit);
MODULE_LICENSE("GPL");