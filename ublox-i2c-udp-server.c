#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <linux/i2c.h>
#include <signal.h>
#include <sys/time.h>

#include "i2c_lib.h"

#define I2C_DEV_1 "/dev/i2c-1" // GPS
#define ADDR_GPS 0x42

int readGPS(int sock,struct sockaddr_in addr)
{
	unsigned char data[2] = {0, 0};
	unsigned char *pdata = data;
	if (i2c_read(I2C_DEV_1, ADDR_GPS, 0xFD, pdata, 2) < 0)
	{
		fprintf(stderr, "%s i2c read err\n", __func__);
		return (-1);
	}

	char msb = data[0];
	char lsb = data[1];

	short bytesAvailable = (short)msb << 8 | lsb;

	if (bytesAvailable <= 0)
		return 0;

	unsigned char *rcvbuffer = (unsigned char *)malloc(bytesAvailable);

	if (!rcvbuffer)
		return -1;

	for (int i = 0; i < bytesAvailable; i++)
	{
		if (i2c_read(I2C_DEV_1, ADDR_GPS, 0xFF, rcvbuffer + i, 1) < 0)
		{
			fprintf(stderr, "%s i2c read err\n", __func__);
			free(rcvbuffer);
			return (-1); // Sensor did not ACK
		}
	}
	
	// udp 送信処理
	if (sendto(sock, rcvbuffer, bytesAvailable, 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		fprintf(stderr, "%s :sendto error %dbyte\n", __func__, bytesAvailable);
		free(rcvbuffer);
		return -1;
	}

	free(rcvbuffer);
	return 1;
}

int endflag = 0;
void SignalHandler(int signo)
{
	endflag = 1;
}
int main(int argc, char *argv[])
{
	signal(SIGUSR1, SignalHandler);
	signal(SIGINT, SignalHandler);
	signal(SIGTERM, SignalHandler);

	int sock;
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	if(argc == 3){
		addr.sin_addr.s_addr = inet_addr(argv[1]);
		addr.sin_port = htons(atoi(argv[2]));
	}else{
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		addr.sin_port = htons(12345);
	}
	sock = socket(AF_INET, SOCK_DGRAM, 0);

	while (!endflag)
	{
		readGPS(sock,addr);
		struct timeval tv_sleep;

		tv_sleep.tv_sec = 0;
		tv_sleep.tv_usec = 10000;
		select(0, NULL, NULL, NULL, &tv_sleep);
	}

	close(sock);
	return 0;
}