#ifndef GRAPHICS_REGS_H_
#define GRAPHICS_REGS_H_

////
//// Command FIFO registers
////

// FIFO_READ: index into command FIFO to be read next 
//
// FIFO_WRITE: index into FIFO to be written next
//
// READ_REQ: Set by server when it is waiting for data from the command FIFO
//
// WRITE_REQ: Set by client when it is waiting for space to write into the command FIFO

#define GRAPHICS_FIFO_READ              0x100
#define GRAPHICS_FIFO_WRITE             0x104
#define GRAPHICS_FIFO_READ_REQ          0x108
#define GRAPHICS_FIFO_WRITE_REQ         0x10C

////
//// Bulk data registers 
////

// BULK_ADDR: Address of data shared between server and client 
//
// BULK_SIZE: Amount of space available at BULK_ADDR
//
// BULK_REQ_SIZE: Set by client. Cleared by server when BULK_SIZE >= BULK_REQ_SIZE
//
// BULK_SIZE_USED: Amount of data read from or written to BULK_ADDR by server while 
//    executing the last command

#define GRAPHICS_BULK_ADDR              0x200
#define GRAPHICS_BULK_SIZE              0x204
#define GRAPHICS_BULK_REQ_SIZE          0x208
#define GRAPHICS_BULK_SIZE_USED         0x20C
#define GRAPHICS_BULK_REQ               0x210

////
//// Asynchonous message registers
////

// ASYNC_COMMAND: Operation to perform on semaphore ASYNC_SEM. 
//    Value is ignored when ASYNC_SEM == KHRN_NO_SEMAPHORE
//
// ASYNC_PID_0: Bits 0-31 of process id containing semaphore ASYNC_SEM
//
// ASYNC_PID_1: Bits 32-63 of process id containing semaphore ASYNC_SEM
//
// ASYNC_SEM: Semaphore to perform operation COMMAND on. Cleared by client
//     when the operation ASYNC_COMMAND has been performed on the semaphore.

#define GRAPHICS_ASYNC_COMMAND          0x300
#define GRAPHICS_ASYNC_PID_0            0x304
#define GRAPHICS_ASYNC_PID_1            0x308
#define GRAPHICS_ASYNC_SEM              0x30C
#define GRAPHICS_ASYNC_REQ              0x310

#define GRAPHICS_RESULT_WRITE           0x400

////
//// Error codes
//// NEN_TODO: Get the server to fill these in
////

// EGL_ERROR: Last EGL error other than EGL_SUCCESS
//
// GL_ERROR: Last GL error other than GL_SUCCESS
//
// VG_ERROR: Last VG error other than VG_SUCCESS

#define GRAPHICS_EGL_ERROR              0x400 // Last EGL error other than EGL_SUCCESS
#define GRAPHICS_GL_ERROR               0x404 // Last GL error other than GL_SUCCESS
#define GRAPHICS_VG_ERROR               0x408 // Last VG error other than VG_SUCCESS

////
//// Static buffers
////

// FIFO: Command FIFO address in IPC block
//
// FIFO_SIZE: Size of command FIFO in bytes
//
// FIFO_LENGTH: Number of uint32's in FIFO
//
// FIFO_WRAP: Partial copy of FIFO starting in memory where the FIFO ends. 
//   Data is copied to this location so that pointers into the FIFO
//   can be returned that are close enough to the end of the FIFO
//   that the client or server may read past the end of the FIFO.
//
// RESULT: Command response address in IPC block
//
// RESULT_SIZE: Size of command response in bytes

#define GRAPHICS_FIFO                   0x1000
#define GRAPHICS_FIFO_SIZE              0x1000
#define GRAPHICS_FIFO_LENGTH            (GRAPHICS_FIFO_SIZE/sizeof(uint32_t))
#define GRAPHICS_FIFO_WRAP              (GRAPHICS_FIFO + GRAPHICS_FIFO_SIZE)  
#define GRAPHICS_FIFO_WRAP_SIZE         (GRAPHICS_FIFO_SIZE)
#define GRAPHICS_RESULT                 (GRAPHICS_FIFO_WRAP + GRAPHICS_FIFO_WRAP_SIZE)
#define GRAPHICS_RESULT_SIZE            0x1000

#endif // GRAPHICS_REGS_H_
