#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>	//	proc filesystem
#include <linux/io.h>

MODULE_LICENSE("Dual BSD/GPL");

#define CPDEBUG_PROCFS_NAME "cpdebug"

static int procfile_read(char *page, char **start, 
	off_t offset, int count, int *eof, void *data) ;

static int procfile_write(struct file *file, const char *buffer, 
	unsigned long count, void *data) ;

static struct proc_dir_entry *g_cpdebug_procfile = 0 ;

static u32 g_cpdebug_boot = 0 ;
static u32 g_cpdebug_dtcm = 0 ;
static u32 g_cpdebug_ipc  = 0 ;

#define CPDEBUG_BOOT_ADDRESS	0x3AC00000
#define CPDEBUG_BOOT_PSIZE	0x40

#define CPDEBUG_DTCM_ADDRESS	0x3AD00000
#define CPDEBUG_DTCM_PSIZE	0x40

#define MAIN_ADDRESS_OFFSET 0x30
#define INIT_ADDRESS_OFFSET 0x34

#define IPC_BASE	0x81E00000
#define IPC_SIZE	0x00200000

static int __init cpdebug_init( void )
{
	g_cpdebug_boot = (u32)ioremap_nocache( CPDEBUG_BOOT_ADDRESS, CPDEBUG_BOOT_PSIZE ) ;
	if( !g_cpdebug_boot )
	{
		printk(KERN_ALERT "Error: could not map cpdebug boot memory\n");
		return -ENOMEM;
	}

	g_cpdebug_dtcm = (u32)ioremap_nocache( CPDEBUG_DTCM_ADDRESS, CPDEBUG_DTCM_PSIZE ) ;
	if( !g_cpdebug_dtcm )
	{
		printk(KERN_ALERT "Error: could not map cpdebug DTCM\n");
		return -ENOMEM;
	}

	g_cpdebug_ipc = (u32)ioremap_nocache( IPC_BASE, IPC_SIZE ) ;
	if( !g_cpdebug_ipc )
	{
		printk(KERN_ALERT "Error: could not map IPC memory\n");
		return -ENOMEM;
	}

	g_cpdebug_procfile = create_proc_entry( CPDEBUG_PROCFS_NAME, S_IRWXU | S_IRWXG | S_IRWXO, NULL );

	if (g_cpdebug_procfile == NULL) {
		remove_proc_entry( CPDEBUG_PROCFS_NAME, NULL); 
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", CPDEBUG_PROCFS_NAME );
		return -ENOMEM;
	}

	g_cpdebug_procfile->read_proc  = procfile_read;
	g_cpdebug_procfile->write_proc = procfile_write;
	g_cpdebug_procfile->mode 	     = S_IRWXU | S_IRWXG | S_IRWXO ; 

	return 0 ;
}

static void __exit cpdebug_exit( void )
{
	if( g_cpdebug_boot )
	{
		iounmap( (volatile void*)g_cpdebug_boot ) ;
		g_cpdebug_boot = 0 ;
	}

	if( g_cpdebug_dtcm )
	{
		iounmap( (volatile void*)g_cpdebug_dtcm ) ;
		g_cpdebug_dtcm = 0 ;
	}

	if( g_cpdebug_ipc )
	{
		iounmap( (volatile void*)g_cpdebug_ipc ) ;
		g_cpdebug_ipc = 0 ;
	}

	if (g_cpdebug_procfile)
		remove_proc_entry( CPDEBUG_PROCFS_NAME, NULL); 

	printk( KERN_ALERT "cpdebug_exit: called\n" ) ;
}

static void safe_strncat( char *dst, const char *src, int len )
{
	int l_dst = strlen( dst ) ;
	int l_src = strlen( src ) ;
	if( l_dst + l_src < len )
	{
		strncat( dst, src, len - l_dst - l_src ) ;
	}
}

static int procfile_read(char *page, char **start, off_t offset, int count, int *eof, void *data)
{
	char buf[80] ;
	u16  idx ;

	*page = 0 ;

	for( idx=0; idx<CPDEBUG_BOOT_PSIZE; idx+=4 )
	{
		sprintf( buf, "CPDEBUG_BOOT_ADDRESS + %x = %x\n", idx, *(u32*)(g_cpdebug_boot+idx) ) ;
		safe_strncat( page, buf, count ) ;
	}

	for( idx=0; idx<CPDEBUG_DTCM_PSIZE; idx+=4 )
	{
		sprintf( buf, "CPDEBUG_DTCM_ADDRESS + %x = %x\n", idx, *(u32*)(g_cpdebug_dtcm+idx) ) ;
		safe_strncat( page, buf, count ) ;
	}

	for( idx=0; idx</* IPC_SIZE */CPDEBUG_DTCM_PSIZE; idx+=4 )
	{
		sprintf( buf, "IPC_BASE + %x = %x\n", idx, *(u32*)(g_cpdebug_ipc+idx) ) ;
		safe_strncat( page, buf, count ) ;
	}

	*eof = 1 ;

	return 1+strlen(page);
}

static int procfile_write(struct file *file, const char *buffer, unsigned long count,
		   void *data)
{
	static int cpdebug_started = 0 ;

	if( !cpdebug_started )
	{
		cpdebug_started = 1 ;

		/* 
		 *	clear first (9) 32-bit words in shared memory 
		memset( (void*)g_cpdebug_ipc, 0, IPC_SIZE ) ;
		 */

		/*
		 *	start the COMMS processor
		 */
		*(u32*)(g_cpdebug_dtcm + INIT_ADDRESS_OFFSET) = *(u32*)(g_cpdebug_dtcm + MAIN_ADDRESS_OFFSET) ;

		printk( KERN_ALERT "Modem started\n" ) ;
	}

	else
	{
		printk( KERN_ALERT "error - cpdebug already started\n" ) ;
	}

	return count ;
}


module_init( cpdebug_init ) ;
module_exit( cpdebug_exit ) ;


