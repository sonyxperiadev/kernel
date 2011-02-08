#ifndef __SOUND_ARM_IPC_H
#define __SOUND_ARM_IPC_H

/******************************************************************************
 FIFO helpers
 *****************************************************************************/

typedef struct
{
	/* ptr to the read/write variables */
	volatile unsigned long *write;
	volatile unsigned long *read;

	/* pointer to the base memory for the fifo */
	volatile void *base;

	/* size of the fifo in terms of entries */
	uint32_t size;

	/* size of each entry */
	uint32_t entry_size;

} IPC_FIFO_T;

static inline void ipc_fifo_reset(IPC_FIFO_T *fifo)
{
	*fifo->write = 0;
	*fifo->read = 0;
}

static inline
void ipc_fifo_setup(IPC_FIFO_T *fifo, volatile unsigned long *write,
			volatile unsigned long *read, volatile void *base_ptr,
			const uint32_t size, const uint32_t entry_size)
{
	fifo->write = write;
	fifo->read = read;
	fifo->size = size;
	fifo->entry_size = entry_size;
	fifo->base = base_ptr;

	ipc_fifo_reset(fifo);
}

static inline
void ipc_fifo_setup_no_reset(IPC_FIFO_T *fifo, volatile unsigned long *write,
				volatile unsigned long *read, volatile void *base_ptr,
				const uint32_t size, const uint32_t entry_size)
{
	fifo->write = write;
	fifo->read = read;
	fifo->size = size;
	fifo->entry_size = entry_size;
	fifo->base = base_ptr;
}

static inline int32_t ipc_fifo_full(const IPC_FIFO_T *fifo)
{
	uint32_t read = *fifo->read;
	uint32_t write = *fifo->write;

	write++;
	write %= fifo->size;

	if (write == read)
		return 1;
	else
		return 0;
}

static inline int32_t ipc_fifo_empty(const IPC_FIFO_T *fifo)
{
	uint32_t read = *fifo->read;
	uint32_t write = *fifo->write;

	if (write == read)
		return 1;
	else
		return 0;
}

static inline void ipc_fifo_write(const IPC_FIFO_T *fifo, const void *entry)
{
	if (!ipc_fifo_full(fifo)) {
		void *fifo_entry = NULL;

		/* inc the write ptr */
		uint32_t write = *fifo->write;

		/* dest */
		fifo_entry = (void *)((int)fifo->base + (fifo->entry_size * write));

		memcpy(fifo_entry, entry, fifo->entry_size);
		write++;
		write %= fifo->size;
		mb();
		*fifo->write = write;
	}
}

static inline void ipc_fifo_read(const IPC_FIFO_T *fifo, void *entry)
{
	if (!ipc_fifo_empty(fifo)) {
		/* read ptr.. */
		uint32_t read = *fifo->read;

		/* dest */
		void *fifo_entry = (void *)((int)fifo->base + (fifo->entry_size * read));

		memcpy(entry, fifo_entry, fifo->entry_size);

		/* inc the read ptr */
		read++;
		read %= fifo->size;
		mb();
		*fifo->read = read;
	}
}


#endif /* __SOUND_ARM_IPC_H */
