// sys/mman.h

#pragma once
#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

// Owned by sys/types.h
// TBD 32-bit on x32, 64-bit on x64
typedef long long off64_t;

extern void *mmap(void *__addr, size_t __len, int __prot,
		     int __flags, int __fd, off64_t __offset);

#endif // _SYS_MMAN_H
