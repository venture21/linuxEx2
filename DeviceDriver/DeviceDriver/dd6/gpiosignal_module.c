#include <linux/fs.h>			// open(),read(),write(),close() 커널함수
#include <linux/cdev.h>			// register_chardev_region(), cdev_init()
#include <linux/module.h>	
#include <linux/io.h>			// ioremap(), iounmap() 커널함수
#include <linux/gpio.h>			// request_gpio(), gpio_set_value()
#include <linux/uaccess.h>		// copy_from_user(), copy_to_user()
#include <linux/interrupt.h>	// gpio_to_irq(), request_irq()
#include <linux/timer.h>		// init_timer(), add_timer(), del_timer()
#include <linux/signal.h>		// signal사용
#include <asm/siginfo.h>		// siginfo 구조체 사용을 위해
#include <linux/sched/signal.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("heejin Park");
MODULE_DESCRIPTION("Raspi 3 GPIO IRQ LED DRIVER");

#define GPIO_MAJOR	200
#define GPIO_MINOR	0
#define GPIO_LED	18
#define GPIO_SW		24
#define GPIO_DEVICE	"gpioled"
#define BLK_SIZE	100

static char msg[BLK_SIZE]={0};

static int gpio_open(struct inode *, struct file*);
static ssize_t gpio_read(struct file*, char *, size_t, loff_t *);
static ssize_t gpio_write(struct file*, const char *, size_t, loff_t *);
static int gpio_close(struct inode *, struct file *);

struct cdev gpio_cdev;
static int switch_irq;
static struct timer_list timer;	//타이머처리를 위한 구조체
static struct task_struct *task;	//태스크를 위한 구조체
pid_t pid;
char pid_valid;
static struct file_operations gpio_fops = {
    .owner = THIS_MODULE,
    .read  = gpio_read,
    .write = gpio_write,
    .open  = gpio_open,
    .release = gpio_close,
};


static void timer_func(unsigned long data)
{
	gpio_set_value(GPIO_LED, data);
	if(data)
		timer.data = 0;
	else
		timer.data = 1;
	timer.expires=jiffies + (1*HZ);
	add_timer(&timer);
}

// 인터럽트 서비스 루틴
static irqreturn_t isr_func(int irq, void *data)
{
	// IRQ발생 && LED가 OFF일때
	static int count;
	static struct siginfo sinfo;
	memset(&sinfo, 0, sizeof(struct siginfo));
	if(irq == switch_irq && !gpio_get_value(GPIO_LED))
	{
		gpio_set_value(GPIO_LED, 1);
		sinfo.si_signo = SIGIO;
		sinfo.si_code = SI_USER;
		
		task = pid_task(find_vpid(pid),PIDTYPE_PID);
		if(task!=NULL)
		//if(pid_valid)
		{
			send_sig_info(SIGIO,&sinfo,task);
		}
		else
		{
			printk("Error: i don't know user pid\n");
		}
	}
	else	//IRQ발생 && LED가ON 일때
		gpio_set_value(GPIO_LED, 0);

	printk(KERN_INFO "Called isr_func():%d\n",count);
	count++;
	return IRQ_HANDLED;
}


int initModule(void)
{
	dev_t devno;
	unsigned int count;
	int err;

	printk(KERN_INFO "gpioirq_module : initModule()\n");

	devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
	
	// 1.커널에 char device를 등록한다.
	register_chrdev_region(devno, 1, GPIO_DEVICE);

	// 2. 문자 디바이스를 위한 구조체를 초기화 한다.
    cdev_init(&gpio_cdev,&gpio_fops);

    gpio_cdev.owner = THIS_MODULE;
    count = 1;

	// 3. 문자디바이스를 추가
    err = cdev_add(&gpio_cdev,devno,count);
    if(err<0)
    {
        printk(KERN_INFO "Error : cdev_add()\n");
        return -1;
    }
	printk(KERN_INFO "'mknod /dev/%s c %d 0'\n", GPIO_DEVICE, GPIO_MAJOR);
    printk(KERN_INFO "'chmod 666 /dev/%s'\n", GPIO_DEVICE);

	// GPIO_LED핀에 대한 요청과 설정
    err = gpio_request(GPIO_LED, "LED");
    if(err==-EBUSY)
    {
        printk(KERN_INFO "Error gpio_request\n");
        return -1;
    }

    gpio_direction_output(GPIO_LED, 0);

    // GPIO_SW핀에 대한 요청과 설정
    err = gpio_request(GPIO_SW, "SWITCH");
    if(err==-EBUSY)
    {
        printk(KERN_INFO "Error gpio_request\n");
        return -1;
    }
	
	// GPIO 인터럽트 번호 리턴
	
	switch_irq = gpio_to_irq(GPIO_SW);
	err=request_irq(switch_irq, isr_func, IRQF_TRIGGER_FALLING, "switch", NULL);
	if(err)
	{
		printk(KERN_INFO "Erro request_irq\n");
		return -1;
	}

	return 0;
}


static int gpio_open(struct inode *inod, struct file *fil)
{
    // open()이 호출될때마다 모듈의 사용 카운터를 증가시킨다.
    try_module_get(THIS_MODULE);

    printk(KERN_INFO "GPIO Device opened()\n");
    return 0;
}


static int gpio_close(struct inode *inod, struct file *fil)
{
    // close()가 호출될때마때다 모듈의 사용 카운터를 감소시킨다.
    module_put(THIS_MODULE);
 	del_timer_sync(&timer);
    printk(KERN_INFO "GPIO Device closed()\n");
    return 0;
}


static ssize_t gpio_read(struct file *inode, char *buff, size_t len, loff_t *off)
{
    int count;
    if(gpio_get_value(GPIO_LED))
        msg[0]='1';
    else
        msg[0]='0';
    strcat(msg, " from kernel");

    // 커널의 msg문자열을 사용자영역(buff의번지)으로 복사한다.
    count = copy_to_user(buff, msg, strlen(msg)+1);

    printk(KERN_INFO "GPIO Device read:%s\n",msg);
    return count;
}


static ssize_t gpio_write(struct file *inode, const char *buff, size_t len, loff_t *off)
{
    short count;
	char *cmd, *str;
	char *sep =":";
	char *endptr, *pidstr;
	//pid_t pid;
    memset(msg, 0 , BLK_SIZE);

    // 사용자영역(buff의 번지)에서 msg 배열로 데이터를 복사한다.
    count = copy_from_user(msg,buff,len);
	str = kstrdup(msg, GFP_KERNEL);
	cmd = strsep(&str,sep);
	pidstr = strsep(&str, sep);
	cmd[1]='\0';
	printk("Command : %s, Pid : %s\n", cmd, pidstr);
	

	if(!strcmp(cmd,"0"))
	{
		del_timer_sync(&timer);
	}
	else
	{
		init_timer(&timer);
		timer.function = timer_func;
		timer.data = 1L; 	//timer_func으로 전달하는 인자값
		timer.expires = jiffies + (1*HZ);	//타이머 시간값
		add_timer(&timer);
		printk("add timer\n");
	}

	if(!strcmp(cmd,"end"))
	{
		pid_valid=0;	
	}
	pid_valid=1;	
	
    gpio_set_value(GPIO_LED, (!strcmp(msg,"0"))?0:1);
    printk(KERN_INFO "GPIO Device write : %s\n", msg);

	// 시그널 발생시 보낼 PID값을 등록
	pid = simple_strtol(pidstr, &endptr, 10);
	//pid =simple_strtol(pidstr, NULL,10);
	printk("pid=%d\n",pid);
	if(endptr !=NULL) {
		task = pid_task(find_vpid(pid),PIDTYPE_PID);
		if(task==NULL)
		{
			printk("Error : Can't find PID from user application\n");
			return 0;
		}
	}
    return count;
}


void cleanupModule(void)
{
	dev_t devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
	//del_timer_sync(&timer);
	
    // 1. 문자 디바이스의 등록을 해제한다.
    unregister_chrdev_region(devno,1);

    // 2. 문자 디바이스의 구조체를 삭제한다.
    cdev_del(&gpio_cdev);

	// 등록된 switch_irq의 인터럽트 해제
	free_irq(switch_irq, NULL);

	gpio_set_value(GPIO_LED, 0);
	gpio_free(GPIO_LED);
	gpio_free(GPIO_SW);

	printk(KERN_INFO "Cleanup gpioirq_module\n");

}





module_init(initModule);
module_exit(cleanupModule);
