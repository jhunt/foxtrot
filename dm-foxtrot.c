#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/bio.h>
#include <linux/device-mapper.h>


struct dm_foxtrot_target {
	struct dm_dev *dev;
	sector_t start;
};

/* map virtual blocks onto physical blocks */
static int
dm_foxtrot_map(struct dm_target *ti, struct bio *bio)
{
	struct dm_foxtrot_target *dm_foxtrot;

	dm_foxtrot = (struct dm_foxtrot_target *) ti->private;
	bio->bi_bdev = dm_foxtrot->dev->bdev;
	/*
	if ((bio->bi_rw & WRITE) == WRITE)
		printk(KERN_CRIT "dm_foxtrot_map : bio is a write request...\n");
	else
		printk(KERN_CRIT "dm_foxtrot_map : bio is a read request...\n");
	 */
	submit_bio(bio->bi_rw,bio);
	return DM_MAPIO_SUBMITTED;
}


/* dm-foxtrot device constructor */
static int
dm_foxtrot_ctr(struct dm_target *ti,unsigned int argc,char **argv)
{
	struct dm_foxtrot_target *dm_foxtrot;
	unsigned long long start;

	if (argc != 2) {
		printk(KERN_CRIT "Invalid no.of arguments.\n");
		ti->error = "Invalid argument count";
		return -EINVAL;
	}

	dm_foxtrot = kmalloc(sizeof(struct dm_foxtrot_target), GFP_KERNEL);
	if (dm_foxtrot == NULL) {
		printk(KERN_CRIT "Mdt is null\n");
		ti->error = "dm-dm_foxtrot: Cannot allocate linear context";
		return -ENOMEM;
	}

	if (sscanf(argv[1], "%llu", &start) != 1) {
		ti->error = "dm-dm_foxtrot: Invalid device sector";
		goto bad;
	}

	dm_foxtrot->start=(sector_t)start;

	if (dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &dm_foxtrot->dev)) {
		ti->error = "dm-dm_foxtrot: Device lookup failed";
		goto bad;
	}

	ti->private = dm_foxtrot;
	return 0;

bad:
	kfree(dm_foxtrot);
	return -EINVAL;
}


static void
dm_foxtrot_dtr(struct dm_target *ti)
{
	struct dm_foxtrot_target *dm_foxtrot;

	dm_foxtrot = (struct dm_foxtrot_target *) ti->private;
	dm_put_device(ti, dm_foxtrot->dev);
	kfree(dm_foxtrot);
}

static struct target_type dm_foxtrot = {
	.name = "foxtrot",
	.version = {1,0,0},
	.module = THIS_MODULE,
	.ctr = dm_foxtrot_ctr,
	.dtr = dm_foxtrot_dtr,
	.map = dm_foxtrot_map,
};

static int
dm_foxtrot_init(void)
{
	int result;

	result = dm_register_target(&dm_foxtrot);
	if (result < 0)
		printk(KERN_CRIT "Error in registering target\n");

	return 0;
}

static void
dm_foxtrot_exit(void)
{
	dm_unregister_target(&dm_foxtrot);
}

module_init(dm_foxtrot_init);
module_exit(dm_foxtrot_exit);

MODULE_LICENSE("Dual MIT/GPL");
