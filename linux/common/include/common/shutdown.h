// Module that manages application shutdown

#ifndef _SHUTDOWN_H_
#define _SHUTDOWN_H_

// initialize module
void Shutdown_init(void);
// blocks process until a trigger
void Shutdown_wait(void);
// unblocks process that is waiting
void Shutdown_trigger(void);
// cleanup module
void Shutdown_cleanup(void);

#endif
