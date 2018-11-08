// unistd.h

#pragma once
#ifndef _UNISTD_H
#define _UNISTD_H

typedef unsigned int uid_t;

uid_t getuid(void);

int gethostname(char *name, size_t len);

int close(int fd);

#endif // _UNISTD_H
