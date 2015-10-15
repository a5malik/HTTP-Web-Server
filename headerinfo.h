#ifndef HEADERINFO_H
#define HEADERINFO_H

#define MAX_FILENAME_SIZE 20000

enum Method{ GET, POST, HEAD };


struct headerinfo
{
	enum Method method_type;
	char version[4];
	char filename[MAX_FILENAME_SIZE];
	int filename_size;
	int filesize;
	char file_found;
};



#endif