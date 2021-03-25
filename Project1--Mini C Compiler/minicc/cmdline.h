#ifndef CMDLINE_H
#define CMDLINE_H

extern char **targv, **arglim;
extern char *outfile;
extern char *infile[];
extern int verbose;

char * get_filename();
int    get_lineno();

#endif
