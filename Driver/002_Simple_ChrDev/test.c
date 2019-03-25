#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEVICE_NAME "/dev/simple_cdev"
#define STRINGS "Hello World"

int main(void)
{
	int fd = 0;
	int i = 0;
	char data[4096] = {0};
	int retval = 0;
	
	fd = open(DEVICE_NAME, O_RDWR);
	
	if (fd == -1)
	{
		perror("error open!\n");
		exit(-1);
	}
	
	printf("open %s successfully!\n", DEVICE_NAME);
	
	//write date
	retval = write(fd, STRINGS, strlen(STRINGS));
	
	if (retval == -1)
	{
		perror("write error!\n");
		exit(-1);
	}
	
	///read date
	retval = read(fd, data, strlen(STRINGS));
	if (retval == -1)
	{
		perror("read error!\n");
		exit(-1);
	}
	
	data[retval] = 0;
	printf("read successfully: %s\n", data);
	
	close(fd);

	return 0;
}
