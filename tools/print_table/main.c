/**
 * client-daemon 간 IPC 테스트를 위해 작성한 코드.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

#include <fon/fon.h>
#include <fon/fon_test.h>

int main(int argc, char **argv)
{
	fon_type_t	type		= 33;
	int		port		= -1;
	FonClient	*client	 = NULL;

	if(argc != 2)
	{
		printf("port has been set as FON_IPC_LISTEN_PORT(0x%X)\n", FON_IPC_LISTEN_PORT);
		port = FON_IPC_LISTEN_PORT;
	}
	else
	{
		port = atoi(argv[1]);
	}

	client = fon_client_new(type, port);
	if(client == NULL)
	{
		perror("fon_client_new()");
		exit(EXIT_FAILURE);
	}

	fon_test_table_print(client);

	return 0;
}

