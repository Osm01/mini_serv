#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct	s_users
{
    int	id;
	char msg[10000];
}				t_users;


fd_set	ActiveFDs , ReadFDs, WriteFDs;
char	BufferWrite[100000];
char	BufferRead[1024];
t_users	users[1024];
int		MaxFDs = 0;
int		ids = 0;

void	print_error(char *msg)
{
	write(2, msg, strlen(msg));
	write(2, "\n", strlen("\n"));
	exit(1);
}

void	broadcast_message(int sender)
{
	for (int fd = 0; fd <= MaxFDs ; fd ++)
	{
		if (FD_ISSET(fd, &WriteFDs) && fd != sender)
			send(fd, BufferWrite, strlen(BufferWrite), 0);
	}
}

int main(int arc, char **argv) {
	if (arc != 2)
		print_error("Wrong number of arguments");
	int sockfd, connfd;
	socklen_t	len;
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
		print_error("Fatal error");
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(argv[1])); 
  
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		print_error("Fatal error");
	if (listen(sockfd, 10) != 0)
		print_error("Fatal error");

	FD_ZERO(&ActiveFDs);
	FD_SET(sockfd, &ActiveFDs);
	MaxFDs = sockfd;

	// char	*buffer = malloc(1000000);
    while (1)
    {
		ReadFDs = WriteFDs = ActiveFDs;
		if (select(MaxFDs + 1, &ReadFDs, &WriteFDs, NULL, NULL) == -1)
			print_error("Fatal error");
		for (int fd = 0; fd <= MaxFDs ;  fd ++)
		{
			if (FD_ISSET(fd, &ReadFDs))
			{
				if (fd == sockfd)
				{
					// new connection
					len = sizeof(cli);
					connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
					if (connfd < 0)
   						print_error("Fatal error");
					MaxFDs = (connfd > MaxFDs) ? connfd : MaxFDs;
					users[connfd].id = ids ++;
					FD_SET(connfd, &ActiveFDs);
					sprintf(BufferWrite, "server: client %d just arrived\n", users[connfd].id);
					broadcast_message(connfd);
				}
				else
				{
					int ByteRead = recv(fd, BufferRead, sizeof(BufferRead) - 1, 0);
					if (ByteRead == -1)
						print_error("Fatal error");
					if (ByteRead == 0)
					{
						// discoonect
						sprintf(BufferWrite, "server: client %d just left\n", users[fd].id);
						broadcast_message(fd);
						FD_CLR(fd, &ActiveFDs);
						close (fd);
						break;
					}
					else
					{
						BufferRead[ByteRead] = 0;
						for (int i = 0, j = strlen(users[fd].msg); i < ByteRead  && y < sizeof(users[fd].msg) ; i++ , j ++)
						{
							users[fd].msg[j] = BufferRead[i];
							if (BufferRead[i] == '\n')
							{
								users[fd].msg[j] = 0;
								sprintf(BufferWrite, "client %d: %s\n", users[fd].id, users[fd].msg);
								broadcast_message(fd);
								bzero(&users[fd].msg, strlen(users[fd].msg));
								j = -1;
							}
						}
						//fprintf(stderr, "the buffer contain : (%d) (%s)\n", users[fd].id, users[fd].msg);
						
					}
				}
			}
		}
    }
	return (1);
}
