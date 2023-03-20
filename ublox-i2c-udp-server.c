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

const unsigned char UBLOX_INIT[] = {
	0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x64, 0x00, 0x01, 0x00, 0x01, 0x00, 0x7A, 0x12,				//(10Hz)
	0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x2B, // GxGLL off
	0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x39, // GxGSV off
	0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x47, // GxVTG off
	0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x24, // GxGGA off
	0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x32, // GxGSA off
};

int gpsinit()
{
	int sendbuffer_cnt = 0;
	char sendbuffer[512] = {0};

	if (i2c_write(I2C_DEV_1, ADDR_GPS, 0xFF, UBLOX_INIT, sizeof(UBLOX_INIT)) < 0)
	{
		return (-1);
	}
	/**
  i2c write len : 8
	\xb5\x62\x06\x08\x00\x00\x0e\x30


	factoryReset
	  i2c write len : 21
	  \xb5\x62\x06\x09\x0d\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\x17\x8a

	hardReset
	  i2c write len : 12
	  \xb5\x62\x06\x04\x04\x00\xff\xff\x00\x00\x0c\x5d

	setI2COutput(COM_TYPE_NMEA | COM_TYPE_UBX);
	  i2c write len : 28
	  \xb5\x62\x06\x00\x14\x00\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\x00\x03\x00\x00\x00\x00\x00\x1a\x2b

	*/

#if 0 
  // i2cbugin
  sendbuffer_cnt = 8;
  memcpy(sendbuffer, "\xb5\x62\x06\x08\x00\x00\x0e\x30", sendbuffer_cnt);

  if(i2c_write(I2C_DEV_1,ADDR_GPS, 0xFF, sendbuffer, sendbuffer_cnt)<0){
		return (-1);
	}

  // factoryReset
  sendbuffer_cnt = 21;
  memcpy(sendbuffer, "\xb5\x62\x06\x09\x0d\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\xff\x17\x8a", sendbuffer_cnt);

  if(i2c_write(I2C_DEV_1,ADDR_GPS, 0xFF, sendbuffer, sendbuffer_cnt)<0){
		return (-1);
	}

  // hardReset
  sendbuffer_cnt = 12;
  memcpy(sendbuffer, "\xb5\x62\x06\x04\x04\x00\xff\xff\x00\x00\x0c\x5d", sendbuffer_cnt);

  if(i2c_write(I2C_DEV_1,ADDR_GPS, 0xFF, sendbuffer, sendbuffer_cnt)<0){
		return (-1);
	}

  sleep(5);

  // setI2COutput(COM_TYPE_NMEA | COM_TYPE_UBX);
  sendbuffer_cnt = 28;
  memcpy(sendbuffer, "\xb5\x62\x06\x00\x14\x00\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\x00\x03\x00\x00\x00\x00\x00\x1a\x2b", sendbuffer_cnt);

  if(i2c_write(I2C_DEV_1,ADDR_GPS, 0xFF, sendbuffer, sendbuffer_cnt)<0){
		return (-1);
	}
#endif

	//  sendbuffer_cnt = 28;
	// memcpy(sendbuffer, "\xb5\x62\x06\x00\x14\x00\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\x00\x03\x00\x00\x00\x00\x00\x1a\x2b", sendbuffer_cnt);
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
	// gpsinit();

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