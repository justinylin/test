#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DEVICE_NODE "/dev/globalvar"

int main(int argc, char **argv)
{
	int fd,num;
	
	fd=open(DEVICE_NODE, O_RDWR);
	
	if(fd != -1){
		read(fd,&num,sizeof(int));
		printf("The globalvar is %d\n",num);
		printf("Please input the num written to globalvar\n");
		scanf("%d",&num);
		write(fd,&num,sizeof(int));
		read(fd,&num,sizeof(int));
		printf("The globalvar is %d\n",num);
		close(fd);
	}else{
		printf("Device open failure\n");
	}
}
