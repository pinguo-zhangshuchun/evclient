#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <comm.h>
#include <netinet/in.h>
#include <protocol.h>
#include <string.h>

int simulate_master_device(int32 token);
static void print_ip(uint32 ip);
static int usage(int argc, char **argv);
static int prepare_tcp_client(char *ip, int port);

void main(int argc, char **argv)
{
	int uid = usage(argc, argv);
	if (uid < 0) {
		printf("Usage:):\n");
		printf("  %s  token\n", argv[0]);
		return;
	}

	int token = atoi(argv[1]);
	token = htonl(token);
	int fd = simulate_master_device(token);
	if (fd < 0) {
		printf("Failed connect to slave device\n");
	}

	uint32 buffer[4];
	buffer[0] = htonl(0xa1);
	buffer[1] = htonl(8);

	if (16 != send(fd, buffer, 16, 0)) {
		printf("Failed send get request.\n");
	}

	if (16 != recv(fd, buffer, 16, 0)) {
		printf("Failed recv get response.\n");
	}

	int w = ntohl(buffer[2]);
	int h = ntohl(buffer[3]);
	printf("w=%d,h=%d\n", w, h);

	buffer[0] = htonl(0xa2);
	buffer[2] = htonl(990);
	buffer[3] = htonl(991);
	if (16 != send(fd, buffer, 16, 0)) {
		printf("Failed send CMD_SET\n");
	}

	close(fd);
}

static void print_ip(uint32 ip)
{
    printf("ip:%d.%d.%d.%d\n", ip & 0xff, 
            (ip>>8) & 0xff, 
            (ip>>16) & 0xff,
            (ip>>24) & 0xff);
}

static int usage(int argc, char **argv) {
    if (2 != argc) {
        return -1;
    }

    return 0;
}

int simulate_master_device(int token)
{
    int fd = prepare_tcp_client("127.0.0.1", DEFAULT_PORT);
    if (fd < 0) {
        LogE("Failed prepare tcp client socket");
        return -1;
    }

    msgheader_t header;
    memset(&header, 0, sizeof(header));
    header.message_type = REQ_CONNECT;
    header.device_type = DEVICE_MASTER;
    header.token = token;

    if (send(fd, &header, sizeof(header), 0) != sizeof(header)) {
        LogE("Failed send msgheader");
        close(fd);
        return -1;
    }

    memset(&header, 0, sizeof(header));
    if (recv(fd, &header, sizeof(header), 0) != sizeof(header)) {
        LogE("Failed recv msgheader");
        close(fd);
        return -1;
    }


    printf("token=%d\n", ntohl(header.token));
    printf("port:%d\n", ntohs(header.port));
    print_ip(header.ip);
    close(fd);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr) );
    addr.sin_addr.s_addr = header.ip;

    fd = prepare_tcp_client(inet_ntoa(addr.sin_addr), ntohs(header.port));
    if (fd < 0) {
        LogE("Failed create socket to connect slave device");
        return -1;
    }

	return fd;
}

static int prepare_tcp_client(char *ip, int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        LogE("Failed create socket\n");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        LogE("Failed connect to server");
        close(fd);
        return -1;
    }

    return fd;
}
