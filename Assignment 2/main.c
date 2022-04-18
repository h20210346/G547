/* DISK ON FILE(DOF) BLOCK DEVICE DRIVER Driver */
#include <linux/string.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/genhd.h> 
#include <linux/blkdev.h> 
#include <linux/hdreg.h> 
#include <linux/errno.h>
#include <linux/vmalloc.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>


#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

#define SECTOR_SIZE 512
#define MBR_SIZE SECTOR_SIZE
#define MBR_DISK_SIGNATURE_OFFSET 440
#define MBR_DISK_SIGNATURE_SIZE 4
#define PARTITION_TABLE_OFFSET 446
#define PARTITION_ENTRY_SIZE 16 
#define PARTITION_TABLE_SIZE 64 
#define MBR_SIGNATURE_OFFSET 510
#define MBR_SIGNATURE_SIZE 2
#define MBR_SIGNATURE 0xAA55
#define BR_SIZE SECTOR_SIZE
#define BR_SIGNATURE_OFFSET 510
#define BR_SIGNATURE_SIZE 2
#define BR_SIGNATURE 0xAA55

#define DOF_FIRST_MINOR 0
#define DOF_MINOR_CNT 16

#define DOF_SECTOR_SIZE 512
#define DOF_DEVICE_SIZE 1024 /* sectors */
/* So, total device size = 1024 * 512 bytes = 512 KiB ( Since 1 sector = 512 Bytes) */

/* Array where the disk stores its data */
static u8 *dev_data;

/* Major number to be reserved */
static u_int dof_major = 0;  




//--------------------Partition Table Entry struture---------------------------
//Struture that stores the entried of the partition table in the DOS partition
//table format.(16 Bytes)
//-----------------------------------------------------------------------------
typedef struct
{
	unsigned char boot_type; // 0x00 - Inactive; 0x80 - Active (Bootable)
	unsigned char start_head;
	unsigned char start_sec:6;
	unsigned char start_cyl_hi:2;
	unsigned char start_cyl;
	unsigned char part_type;
	unsigned char end_head;
	unsigned char end_sec:6;
	unsigned char end_cyl_hi:2;
	unsigned char end_cyl;
	unsigned int abs_start_sec;
	unsigned int sec_in_part;
} PartEntry;

// The are four partitions in the partition table in MBR 
typedef PartEntry PartTable[4]; 

//--------------------Partitions dof1 and dof2---------------------------
//Defining the dof1 and dof2 partition table entries
//-----------------------------------------------------------------------------
static PartTable def_part_table =
{
	{
		boot_type: 0x00,
		start_head: 0x00,
		start_sec: 0x2,
		start_cyl: 0x00,
		part_type: 0x83,
		end_head: 0x00,
		end_sec: 0x20,
		end_cyl: 0x09,
		abs_start_sec: 0x00000001,
		sec_in_part: 0x0000013F
	},
	{
		boot_type: 0x00,
		start_head: 0x00,
		start_sec: 0x1,
		start_cyl: 0x14,
		part_type: 0x83,
		end_head: 0x00,
		end_sec: 0x20,
		end_cyl: 0x1F,
		abs_start_sec: 0x00000280,
		sec_in_part: 0x00000180
	},
         
};

//------------------copy_mbr()-----------------------------------------
//FUnction : This function is used set up the MBR of the dof inside the
//           dofdevice_init() function
//Input argument : (1) Array where the dof stores its data 
//Output argument : void
//-------------------------------------------------------------------------
static void copy_mbr(u8 *disk)
{
	loff_t pos = 446;
	memset(disk, 0x0, MBR_SIZE);
	*(unsigned long *)(disk + MBR_DISK_SIGNATURE_OFFSET) = 0x36E5756D;
	memcpy(disk + PARTITION_TABLE_OFFSET, &def_part_table, PARTITION_TABLE_SIZE);
	*(unsigned short *)(disk + MBR_SIGNATURE_OFFSET) = MBR_SIGNATURE;


       

	//Declaring file pointer 
	struct file *dof_file;

	//Opening file 
	dof_file = filp_open("/etc/sachin.txt",  O_RDWR, 666);

       kernel_write(dof_file,&def_part_table,PARTITION_TABLE_SIZE, &pos );

       //close the file
	filp_close(dof_file,NULL); 
        
}


//------------------dofdevice_init()-----------------------------------------
//FUnction : This function is used to initialize the Disk On File(DOF)
//Input argument : void
//Output argument : Size of the file which is treted as a block disk device
//                  (in terms of number of sectors)
//-------------------------------------------------------------------------
int dofdevice_init(void)
{
	dev_data = vmalloc(DOF_DEVICE_SIZE * DOF_SECTOR_SIZE);
	if (dev_data == NULL)
		return -ENOMEM;
	/* Setup its partition table */
	copy_mbr(dev_data);
	return DOF_DEVICE_SIZE;
}

//------------------dofdevice_cleanup()-----------------------------------------
//FUnction : It calls the vfree() function and passes it dev_data as the 
//           argument. This frees the memory space occupied by the dof
//Input argument : void
//Output argument : void
//-----------------------------------------------------------------------------

void dofdevice_cleanup(void)
{
	vfree(dev_data);
}

//------------------dofdevice_writer()-----------------------------------------
//Function : Writes data from a memory location into the dof
//Input argument : (1) Sector offset 
//                 (2) pointer to the data which isto be written
//                 (3) number of sectors to write
//Output argument : void
//-----------------------------------------------------------------------------
void dofdevice_write(sector_t sector_off, u8 *buffer, unsigned int sectors)
{

	memcpy(dev_data + sector_off * DOF_SECTOR_SIZE, buffer,
		sectors * DOF_SECTOR_SIZE);
    
	
	//Declaring file pointer 
	struct file *dof_file;

	//Opening file 
	dof_file = filp_open("/etc/sachin.txt",  O_RDWR, 666);

 	kernel_write(dof_file,buffer,sectors * DOF_SECTOR_SIZE, sector_off*DOF_SECTOR_SIZE );

	//close the file
	filp_close(dof_file,NULL); 
}

//------------------dofdevice_read()-----------------------------------------
//FUnction : Reads data from dof and writes it into a memory location
//Input argument : (1) Sector offset 
//                 (2) pointer to the address where the data from thedisk is 
//                     to be written 
//                 (3) number of sectors to read from
//Output argument : void
//-----------------------------------------------------------------------------

void dofdevice_read(sector_t sector_off, u8 *buffer, unsigned int sectors)
{
	memcpy(buffer, dev_data + sector_off * DOF_SECTOR_SIZE,
		sectors * DOF_SECTOR_SIZE);

	//Declaring file pointer 
	struct file *dof_file;	

	//open the file
        dof_file = filp_open("/etc/sachin.txt",  O_RDWR, 666);

	kernel_read(dof_file, sector_off*DOF_SECTOR_SIZE,buffer, sectors * DOF_SECTOR_SIZE);

 	//close the file
	filp_close(dof_file,NULL);
        
}


//----------------------------------------------------------- 
// The internal structure representation of our Device dof
//-----------------------------------------------------------
static struct dof_device
{
	/* Size is the size of the device (in sectors) */
	unsigned int size;
	/* For exclusive access to our request queue */
	spinlock_t lock;
	/* Our request queue */
	struct request_queue *dof_queue;
	/* This is kernel's representation of an individual disk device */
	struct gendisk *dof_disk;
} dof_dev;


//----------------------dof_open()---------------------
//FUnction used to open the disk of file. 
//----------------------------------------------------

static int dof_open(struct block_device *bdev, fmode_t mode)
{
	unsigned unit = iminor(bdev->bd_inode);

	printk(KERN_INFO "dof: Device is opened\n");
	printk(KERN_INFO "dof: Inode number is %d\n", unit);

	if (unit > DOF_MINOR_CNT)
		return -ENODEV;
	return 0;
}

//----------------------dof_close()---------------------
//Function used to close the disk of file. 
//----------------------------------------------------

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
static int dof_close(struct gendisk *disk, fmode_t mode)
{
	printk(KERN_INFO "dof: Device is closed\n");
	return 0;
}
#else
static void dof_close(struct gendisk *disk, fmode_t mode)
{
	printk(KERN_INFO "dof: Device is closed\n");
}
#endif

//----------------------dof_getgeo()---------------------
//Defines the geometry of the dof andd used in file operations
//----------------------------------------------------

static int dof_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	geo->heads = 1;
	geo->cylinders = 32;
	geo->sectors = 32;
	geo->start = 0;
	return 0;
}

//-----------------------dof_transfer()-----------------------
//FUnction that does the Actual Data transfer
//-----------------------------------------------------------
static int dof_transfer(struct request *req)
{

	int dir = rq_data_dir(req);
	sector_t start_sector = blk_rq_pos(req);
	unsigned int sector_cnt = blk_rq_sectors(req);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0))
#define BV_PAGE(bv) ((bv)->bv_page)
#define BV_OFFSET(bv) ((bv)->bv_offset)
#define BV_LEN(bv) ((bv)->bv_len)
	struct bio_vec *bv;
#else
#define BV_PAGE(bv) ((bv).bv_page)
#define BV_OFFSET(bv) ((bv).bv_offset)
#define BV_LEN(bv) ((bv).bv_len)
	struct bio_vec bv;
#endif
	struct req_iterator iter;

	sector_t sector_offset;
	unsigned int sectors;
	u8 *buffer;

	int ret = 0;

	sector_offset = 0;
	rq_for_each_segment(bv, req, iter)
	{
		buffer = page_address(BV_PAGE(bv)) + BV_OFFSET(bv);
		if (BV_LEN(bv) % DOF_SECTOR_SIZE != 0)
		{
			printk(KERN_ERR "dof: Should never happen: "
				"bio size (%d) is not a multiple of DOF_SECTOR_SIZE (%d).\n"
				"This may lead to data truncation.\n",
				BV_LEN(bv), DOF_SECTOR_SIZE);
			ret = -EIO;
		}
		sectors = BV_LEN(bv) / DOF_SECTOR_SIZE;
		printk(KERN_DEBUG "dof: Start Sector: %llu, Sector Offset: %llu; Buffer: %p; Length: %u sectors\n",
			(unsigned long long)(start_sector), (unsigned long long)(sector_offset), buffer, sectors);
		if (dir == WRITE) /* Write to the device */
		{
			dofdevice_write(start_sector + sector_offset, buffer, sectors);
		}
		else /* Read from the device */
		{
			dofdevice_read(start_sector + sector_offset, buffer, sectors);
		}
		sector_offset += sectors;
	}
	if (sector_offset != sector_cnt)
	{
		printk(KERN_ERR "dof: bio info doesn't match with the request info");
		ret = -EIO;
	}

	return ret;
}
	
//----------------------dof_request()-------------------------
// Represents a block I/O request for us to execute
//-------------------------------------------------------------
static void dof_request(struct request_queue *q)
{
	struct request *req;
	int ret;

	/* Gets the current request from the dispatch queue */
	while ((req = blk_fetch_request(q)) != NULL)
	{
		ret = dof_transfer(req);
		__blk_end_request_all(req, ret);
	}
}

//------------------------------------------------------------------------
// These are the file operations that performed on the dof block device
//------------------------------------------------------------------------
static struct block_device_operations dof_fops =
{
	.owner = THIS_MODULE,
	.open = dof_open,
	.release = dof_close,
	.getgeo = dof_getgeo,
};
	
//--------------------------dof_init()-----------------------------------------
// This is the registration and initialization section of the dof block device
// driver
//------------------------------------------------------------------------------
static int __init dof_init(void)
{

	int ret;

	/* Set up our DOF Device */
	if ((ret = dofdevice_init()) < 0)
	{
		return ret;
	}
	dof_dev.size = ret;

	/* Get Registered */
	dof_major = register_blkdev(dof_major, "dof");
	if (dof_major <= 0)
	{
		printk(KERN_ERR "dof: Unable to get Major Number\n");
		dofdevice_cleanup();
		return -EBUSY;
	}

	/* Get a request queue (here queue is created) */
	spin_lock_init(&dof_dev.lock);
	dof_dev.dof_queue = blk_init_queue(dof_request, &dof_dev.lock);
	if (dof_dev.dof_queue == NULL)
	{
		printk(KERN_ERR "dof: blk_init_queue failure\n");
		unregister_blkdev(dof_major, "dof");
		dofdevice_cleanup();
		return -ENOMEM;
	}
	
	/*
	 * Add the gendisk structure
	 * By using this memory allocation is involved, 
	 * the minor number we need to pass bcz the device 
	 * will support this much partitions 
	 */
	dof_dev.dof_disk = alloc_disk(DOF_MINOR_CNT);
	if (!dof_dev.dof_disk)
	{
		printk(KERN_ERR "dof: alloc_disk failure\n");
		blk_cleanup_queue(dof_dev.dof_queue);
		unregister_blkdev(dof_major, "dof");
		dofdevice_cleanup();
		return -ENOMEM;
	}

 	/* Setting the major number */
	dof_dev.dof_disk->major = dof_major;
  	/* Setting the first mior number */
	dof_dev.dof_disk->first_minor = DOF_FIRST_MINOR;
 	/* Initializing the device operations */
	dof_dev.dof_disk->fops = &dof_fops;
 	/* Driver-specific own internal data */
	dof_dev.dof_disk->private_data = &dof_dev;
	dof_dev.dof_disk->queue = dof_dev.dof_queue;
	/*
	 * You do not want partition information to show up in 
	 * cat /proc/partitions set this flags
	 */
	//dof_dev.dof_disk->flags = GENHD_FL_SUPPRESS_PARTITION_INFO;
	sprintf(dof_dev.dof_disk->disk_name, "dof");
	/* Setting the capacity of the device in its gendisk structure */
	set_capacity(dof_dev.dof_disk, dof_dev.size);

	/* Adding the disk to the system */
	add_disk(dof_dev.dof_disk);
	/* Now the disk is "live" */
	printk(KERN_INFO "dof: DOF Block driver initialised (%d sectors; %d bytes)\n",
		dof_dev.size, dof_dev.size * DOF_SECTOR_SIZE);

	return 0;
}



//-------------------------------dof_cleanup()--------------------------------
// This is the unregistration and uninitialization section of the dof block
// device driver
//----------------------------------------------------------------------------
static void __exit dof_cleanup(void)
{       


	del_gendisk(dof_dev.dof_disk);
	put_disk(dof_dev.dof_disk);
	blk_cleanup_queue(dof_dev.dof_queue);
	unregister_blkdev(dof_major, "dof");
	dofdevice_cleanup();
}

//----------Initialisation------ 
module_init(dof_init);

//-------Exiting-------
module_exit(dof_cleanup);

//Module Information 
MODULE_LICENSE("GPL");


