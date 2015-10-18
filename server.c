/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
*/
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h>	/* for the waitpid() system call */
#include <signal.h>	/* signal name macros, and the kill() prototype */
#include <sys/time.h>
#include "headerinfo.h"
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_LINE_LENGTH 100
#define HEADER_LENGTH 5000

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void dostuff(int); /* function prototype */
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, pid;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     struct sigaction sa;          // for signal SIGCHLD

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     
     listen(sockfd,5);
     
     clilen = sizeof(cli_addr);
     
     /****** Kill Zombie Processes ******/
     sa.sa_handler = sigchld_handler; // reap all dead processes
     sigemptyset(&sa.sa_mask);
     sa.sa_flags = SA_RESTART;
     if (sigaction(SIGCHLD, &sa, NULL) == -1) {
         perror("sigaction");
         exit(1);
     }
     /*********************************/
     
     while (1) {
         newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
         
         if (newsockfd < 0) 
             error("ERROR on accept");
         
         pid = fork(); //create a new process
         if (pid < 0)
             error("ERROR on fork");
         
         if (pid == 0)  { // fork() returns a value of 0 to the child process
             close(sockfd);
             dostuff(newsockfd);
             exit(0);
         }
         else //returns the process ID of the child process to the parent
             close(newsockfd); // parent doesn't need this 
     } /* end of while */
     return 0; /* we never get here */
}

/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
 
 void processLine(const char* line, int  size, struct headerinfo* hi, char* namebuf)
 {
	 char* pos = line;
	 int sz = 0, i = 0;
	 static char first_line = 't';
	 if(first_line == 't')
	 {
		 if( pos = strstr(line, "GET"))
		 {
			 hi->method_type = GET;
			 pos += 4;
			 while(*pos != ' ')
			 {
				 hi->filename[hi->filename_size++] = *pos++;
			 }
			hi->filename[hi->filename_size] = '\0';
		 }
		  printf("IT REQUESTS FOR FILE: %s \n", hi->filename);
		  if(findFile(hi->filename, &sz, namebuf))
		  {
			  hi->filesize = sz;
			  hi->file_found = 't';
		  }
		  if(pos = strstr(line, "HTTP"))
		  {
			  pos+=5;
			  for( i = 0; i < 3;i++)
				  hi->version[i] = *(pos+i);
			  hi->version[3] = '\0';
			  
		  }
	 }
	 printf("%s\n", line);
	 first_line = 'f';
 }
 
 int findFile(char* filename, int* size, char* namebuf)
 {
	 DIR *dir;
	 int i , n;
	 struct dirent **namelist;
	 char* name, *slash_pos, *current_pos;
	 struct dirent *ent;
	 struct stat st;
	 char buf[PATH_MAX];
	 current_pos = filename+1;
	 if ((dir = opendir(".")) == NULL)
	 {printf("directory is fucked up");}
	  else
	  {
		  while(1)
		  {
			  if((slash_pos = strstr(current_pos, "/")) == NULL)
			  {
				  char found = 'f';
				  //this is the file - read it and put it into a buffer
					  n = scandir(".", &namelist, 0, alphasort);
					  char fname[MAX_FILENAME_SIZE];
					  strcpy(fname, current_pos);
					for (i = 2; i < n; i++) 
					{
					// printf("%s\n",namelist[i]->d_name);
						stat(namelist[i]->d_name, &st);
					if(S_ISREG(st.st_mode))
						{
							if(!strcmp(namelist[i]->d_name, fname))
							{
								printf("file ducking found %s having size %d\n", fname, st.st_size);
								found = 't';
								strcpy(namebuf, fname);
								*size = st.st_size;
								break;
							}
						}
					}
					if(found == 'f')
					{
						printf("no such file ducking found %s \n", fname);
						return 0;
					}
					
					return 1;
			  }
			  else 
			  {
				  //change to subdirectory
				  char dir_name[MAX_FILENAME_SIZE];
				  strncpy(dir_name, current_pos, slash_pos-current_pos);
				  dir_name[slash_pos-current_pos] = '\0';
				  current_pos = slash_pos+1;
				  char found = 'f';
				   n = scandir(".", &namelist, 0, alphasort);
				   for (i = 2; i < n; i++) 
					{
					// printf("%s\n",namelist[i]->d_name);
						stat(namelist[i]->d_name, &st);
					if(S_ISDIR(st.st_mode))
						{
							if(!strcmp(namelist[i]->d_name, dir_name))
							{
								printf("directory ducking found %s\n", dir_name);
								chdir(dir_name);
								found = 't';
								break;
							}
						}
					}
					if(found == 'f')
					{
						printf("no such ducking directory %s\n", dir_name);
						return 0;
					}
			  }
		  }
	  }
	  return 0;
 }
void dostuff (int sock)
{
	
	/*
	objective: 
	1)read line by line the http headers untill you get \r\n\r\n
	2)store each line into fixed size buffer. 
	3)process the buffer to get header field information, store that into a struct . 
	4)build response by pasting headers info and then opening the file required. 
	*/
	
	struct headerinfo hi;
	hi.filename[0] = '\0';
	hi.filename_size  = 0;
	hi.file_found = 'f';
	char name_buf[MAX_FILENAME_SIZE];
	int n, fd;
	char c;
	char* buffer = (char*) malloc(MAX_LINE_LENGTH);
	size_t size_multiplier = 1;
	int count = 0;
	//char end[4] = {'\r', '\n', '\r', '\n', '\0'};
	char prev_cr = 'f'; //back to back carriage returns
	memset((void *)buffer, 0, size_multiplier * MAX_LINE_LENGTH);
	while((n=read(sock, &c, 1)) == 1)
	{
		if(c == '\n')
		{
			buffer[count] = '\0';
			if(count == 1 && buffer[0] == '\r')
			{
				break;
			}
			else
			{
				prev_cr = 'f';
				processLine(buffer, count, &hi, name_buf);
			}
			memset((void *)buffer, 0, size_multiplier * MAX_LINE_LENGTH);
			count = 0;
			continue;
		}
		buffer[count++] = c;
		if(count == size_multiplier * MAX_LINE_LENGTH-1)
			buffer = (char*)realloc(buffer, ++size_multiplier * MAX_LINE_LENGTH);
	}
	printf("out of loop");
	if(n < 0)
		 error("ERROR reading from socket");
	 
	  char str[HEADER_LENGTH];
	 if((fd = open(name_buf, O_RDONLY)) == -1)
	 { 
		sprintf ( str, "HTTP/%s 404 Not Found\r\nContent-type: text/html\r\nContent-length: 135\r\n\r\n<html><head><title>Not Found</title></head><body>Sorry, the object you requested was not found.</body><html>", hi.version );
		printf(str);
		write(sock, str, strlen(str));
	 }
	 else
	 {
		 sprintf(str , "HTTP/%s 200 OK\r\nContent-length: %d\r\n\r\n", hi.version, hi.filesize);
		printf(str);
		 write(sock, str, strlen(str));
		 while((n=read(fd,&c, 1) == 1))
		 {
			 if (n < 0) error("ERROR reading from file");	
			 if((n = write(sock,&c, 1)) == 1)
				 continue;
			 else
				 error("ERROR reading from file");
		 }
	 }
}
	
	//
	
   // int n;
   // char c;
   // char buffer[MAX_LINE_LENGTH] = {0};
   // struct timeval tv;
   // fd_set active_fd;
   // int select_return;
   // tv.tv_sec = 10;
   // tv.tv_usec = 0;
   // printf("Here is the message: ");
	// do{
		// FD_ZERO(&active_fd);
		// FD_SET(sock, &active_fd);
		// if((select_return = select(sock+1, &active_fd, NULL, NULL, &tv)) < 0)
			// error("ERROR reading from socket");
		// else if( select_return > 0)
		// {
			// int n = read(sock, buffer, MAX_LINE_LENGTH-1);
			// buffer[n] = '\0';
			// if(n < 0)
				// if (n < 0) error("ERROR reading from socket");
			// if(n > 0)
				// printf("%s", buffer);
			
		// }
	// }while(select_return != 0);
	
	// printf("\n");
   // n = write(sock,"I got your message",18);
   // if (n < 0) error("ERROR writing to socket");
   