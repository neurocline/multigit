// xplat.h
// Simple cross-platform layer

#pragma once

// Cross-platform helpers
int internal_mkstemp(char* template); // linux mkstemp clone
unsigned long internal_write(int fd, void* buf, unsigned long size); // linux write clone

// See http://man7.org/linux/man-pages/man2/access.2.html
int internal_access_R_OK(const char *pathname); // linux access clone hardwired to mode=R_OK

// See http://man7.org/linux/man-pages/man2/mkdir.2.html
// See http://pubs.opengroup.org/onlinepubs/009696699/basedefs/sys/types.h.html
// See http://pubs.opengroup.org/onlinepubs/7908799/xsh/sysstat.h.html
int internal_mkdir(const char *pathname, int mode); // linux mkdir clone

// Synthesized function
int internal_is_dir(const char* pathname);
