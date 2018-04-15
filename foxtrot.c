#include <linux/module.h>
#include <linux/init.h>

#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>

MODULE_LICENSE("Dual BSD/GPL");

#define FOXTROT_VERSION "1.0.0"

struct foxtrot_dev {
	spinlock_t lock;                /* For mutual exclusion */
	struct request_queue *queue;    /* The device request queue */
	struct gendisk *gd;             /* The gendisk structure */
};

static int nsectors = 1024;
static int hardsect_size = 512;

/*
 * We can tweak our hardware sector size, but the kernel talks to us
 * in terms of small sectors, always.
 */
#define KERNEL_SECTOR_SHIFT	9
#define KERNEL_SECTOR_SIZE	(1<<KERNEL_SECTOR_SHIFT)

static int                 foxtrot_major = 0;
static struct foxtrot_dev *foxtrot_dev = NULL;

static int
foxtrot_open(struct block_device *bdev, fmode_t mode)
{
	/* FIXME: noop! */
	return 0;
}

static void
foxtrot_release(struct gendisk *disk, fmode_t mode)
{
	/* FIXME: noop! */
}

static int
foxtrot_ioctl (struct block_device *bdev, fmode_t mode,
               unsigned int cmd, unsigned long arg)
{
	/* FIXME: noop! */
	return -ENOTTY;
}

static struct block_device_operations foxtrot_ops = {
	.owner           = THIS_MODULE,
	.open            = foxtrot_open,
	.release         = foxtrot_release,
	.ioctl           = foxtrot_ioctl
};

static void
foxtrot_request(struct request_queue *q)
{
	struct request *req;
	int ret;

	req = blk_fetch_request(q);
	while (req) {
		printk (KERN_NOTICE "foxtrot: req dir %d sec %ld, nr %d\n",
		                    rq_data_dir(req), blk_rq_pos(req), blk_rq_cur_sectors(req));
		ret = 0;

		if(!__blk_end_request_cur(req, ret)){
			req = blk_fetch_request(q);
		}
	}
}

static int
foxtrot_init(void)
{
	printk(KERN_INFO "foxtrot v" FOXTROT_VERSION " starting up...\n");

	foxtrot_major = register_blkdev(foxtrot_major, "foxtrot");
	if (foxtrot_major <= 0) {
		printk(KERN_WARNING "foxtrot: unable to get a major number\n");
		return -EBUSY;
	}

	foxtrot_dev = kmalloc(sizeof(*foxtrot_dev), GFP_KERNEL);
	if (foxtrot_dev == NULL)
		goto unregister_and_fail;

	spin_lock_init(&foxtrot_dev->lock);
	foxtrot_dev->queue = blk_init_queue(foxtrot_request, &foxtrot_dev->lock);
	if (foxtrot_dev->queue == NULL)
		goto unregister_and_fail;

	foxtrot_dev->queue->queuedata = foxtrot_dev;
	foxtrot_dev->gd = alloc_disk(8192);
	if (foxtrot_dev->gd == NULL) {
		printk(KERN_NOTICE "foxtrot: disk device allocation failure\n");
		goto unregister_and_fail;
	}
	foxtrot_dev->gd->major = foxtrot_major;
	foxtrot_dev->gd->first_minor = 1;
	foxtrot_dev->gd->fops = &foxtrot_ops;
	foxtrot_dev->gd->queue = foxtrot_dev->queue;
	foxtrot_dev->gd->private_data = foxtrot_dev;
	snprintf(foxtrot_dev->gd->disk_name, 32, "foxtrot");
	set_capacity(foxtrot_dev->gd, nsectors*(hardsect_size/KERNEL_SECTOR_SIZE));
	add_disk(foxtrot_dev->gd);

	return 0;

unregister_and_fail:
	unregister_blkdev(foxtrot_major, "foxtrot");
	return -ENOMEM;
}

static void
foxtrot_exit(void)
{
	printk(KERN_INFO "foxtrot v" FOXTROT_VERSION " shutting down...\n");

	blk_cleanup_queue(foxtrot_dev->queue);
	kfree(foxtrot_dev);

	if (foxtrot_dev->gd != NULL) {
		del_gendisk(foxtrot_dev->gd);
		put_disk(foxtrot_dev->gd);
	}

	unregister_blkdev(foxtrot_major, "foxtrot");
}

module_init(foxtrot_init);
module_exit(foxtrot_exit);
