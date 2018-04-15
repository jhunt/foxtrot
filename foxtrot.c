#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

#define FOXTROT_VERSION "1.0.0"

static int
foxtrot_init(void)
{
	printk(KERN_INFO "foxtrot v" FOXTROT_VERSION " starting up...\n");
	return 0;
}

static void
foxtrot_exit(void)
{
	printk(KERN_INFO "foxtrot v" FOXTROT_VERSION " shutting down...\n");
}

module_init(foxtrot_init);
module_exit(foxtrot_exit);
