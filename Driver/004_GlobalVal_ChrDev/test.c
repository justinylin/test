#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define DEVICE_NODE "/dev/globalvar"
#define PAGE_SIZE 4096
#define READ_OPCODE "r"
#define WRITE_OPCODE "w"

int main(int argc, char **argv)
{
	int fd = -1;
	int count = 0;
	char *str = NULL;

	if (argc != 3)
	{
		printf("Usage:\n");
		printf("%s r <count>\n", argv[0]);
		printf("%s w <strings>\n", argv[0]);
		return -1;
	}
	else
	{
		if (0 == strncmp(argv[1], READ_OPCODE, strlen(READ_OPCODE)))
		{
			/* read operation */
			count = (int)strtol(argv[2], NULL, 10);
			if (count <= 0 || count > PAGE_SIZE)
			{
				printf("read count %d out of range, should be 1~%d\n", count, PAGE_SIZE);
				return -1;
			}
		}
		else if (0 == strncmp(argv[1], WRITE_OPCODE, strlen(WRITE_OPCODE)))
		{
			/* write operation */
			count = strlen(argv[2]);
			if (count <= 0 || count > PAGE_SIZE)
			{
				printf("write string too long\n");
				return -1;
			}
			else
			{
				str = argv[2];
			}
		}
		else
		{
			printf("wrong opcode %s, must be %s or %s\n", argv[1], READ_OPCODE, WRITE_OPCODE);
			return -1;
		}
	}
	
	fd = open(DEVICE_NODE, O_RDWR);
	if (-1 == fd)
	{
		perror("open /dev/globalvar failed");
		return -1;
	}

	if (0 == strncmp(argv[1], READ_OPCODE, strlen(READ_OPCODE)))
	{
		/* read operation */
		str = (char*)malloc(count + 1);
		if (NULL == str)
		{
			perror("buffer malloc failed");
			close(fd);
			return -1;
		}

		/* make sure the last byte is '\0' */
		memset(str, 0, count + 1);

		read(fd, str, count);

		printf("read out: %s\n", str);

		free(str);
	}
	else if (0 == strncmp(argv[1], WRITE_OPCODE, strlen(WRITE_OPCODE)))
	{
		/* write operation */
		write(fd, str, count);
	}

	close(fd);

	return 0;

}
