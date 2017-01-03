/* \file main.c
 * \brief Example application for communicating with the dfplayer mini MP3 module
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <termios.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "dfplayer.h"

static int OpenSerial(const char *port, unsigned int speed, unsigned int parity);
static bool SerialReceive(void *dfplayer, int fd);

static void dfplayer_HandleInitialize(void *context, void *token, uint16_t devices_online);
static void dfplayer_HandleTrackFinished(void *context, void *token, uint16_t track_number, uint16_t device);
static void dfplayer_HandleDeviceState(void *context, void *token, uint16_t device, bool inserted);
static void dfplayer_HandleError(void *context, void *token, dfplayerError_e error);
static void dfplayer_HandleReply(void *context, void *token);
static int dfplayer_SerialSend(void *context, void *token, uint8_t *data, uint32_t bytes);

typedef struct app_info_s
{
	int fd;
} app_info_t;

int main(int argc, char *argv[])
{
	const char *portname;
	int fd;
	bool done = false;
	void *dfplayer;
	dfplayer_init_info_t init_info;
	app_info_t *app_info;

	if(argc < 2)
	{
		fprintf(stderr, "%s [port]\n", argv[0]);
		return -1;
	}
	portname = argv[1];

	fd = OpenSerial(portname, B9600, 0);
	if(fd < 0)
	{
        	fprintf(stderr, "Error opening serial port '%s': %d (%s)\n",
			portname, errno, strerror(errno));
		return -1;
	}

	app_info = (app_info_t *) malloc(sizeof(*app_info));
	if(NULL == app_info)
	{
		fprintf(stderr, "Failed to allocate app info structure\n");
		return -1;
	}
	app_info->fd = fd;

	init_info.pfnHandleInitialize = dfplayer_HandleInitialize;
	init_info.pfnHandleTrackFinished = dfplayer_HandleTrackFinished;
	init_info.pfnHandleDeviceState = dfplayer_HandleDeviceState;
	init_info.pfnHandleError = dfplayer_HandleError;
	init_info.pfnHandleReply = dfplayer_HandleReply;
	init_info.pfnSendSerial = dfplayer_SerialSend; 
	dfplayer = dfplayer_Initialize((void *) app_info, &init_info);
	if(NULL == dfplayer)
	{
		fprintf(stderr, "Failed to initialize dfplayer\n");
		return -1;
	}

	while(!done)	
	{
		done = SerialReceive(dfplayer, fd);
	}

	printf("Done\n");
	close(fd);

	return 0;
}

/* -------------------------------------------------------------------------------------------
 * Serial Port 
 */

static bool SerialReceive(void *dfplayer, int fd)
{
	uint8_t data;
	bool done = false;
	fd_set fdRead;
	struct timeval timeout;

	FD_ZERO(&fdRead);
	FD_SET(fd, &fdRead);
	timeout.tv_sec = 0;
	timeout.tv_usec = 100000L;
	if(select(fd + 1, &fdRead, NULL, NULL, &timeout) > 0)
	{
		int result = read(fd, &data, sizeof(data));
		if(result > 0)
			dfplayer_HandleSerialChar(dfplayer, data);
		else if(result < 0)
			done = true;
	}

	return done;
}

static int OpenSerial(const char *port, unsigned int speed, unsigned int parity)
{
        struct termios tty;
	int result;
	int fd;

	fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);

        memset(&tty, 0, sizeof(tty));
        result = tcgetattr(fd, &tty);
        if(result != 0)
        {
                fprintf(stderr, "%s: Error from tcgetattr %d (%s)\n",
			__func__, errno, strerror(errno));
                return result;
        }
        cfsetospeed(&tty, speed);
        cfsetispeed(&tty, speed);

        tty.c_iflag &= ~IGNBRK;
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);

        tty.c_lflag = 0;
        tty.c_oflag = 0;
        tty.c_cc[VMIN]  = 1;
        tty.c_cc[VTIME] = 0;

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
        tty.c_cflag |= (CLOCAL | CREAD);
        tty.c_cflag &= ~(PARENB | PARODD);
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if(tcsetattr(fd, TCSANOW, &tty) != 0)
        {
                fprintf(stderr, "%s: Error from tcsetattr %d (%s)\n",
			__func__, errno, strerror(errno));
                return result;
        }
        return fd;
}

/* -------------------------------------------------------------------------------------------
 * DFPlayer Event Handlers 
 */

static void dfplayer_HandleInitialize(void *context, void *token, uint16_t devices_online)
{
	fprintf(stderr, "%s: Initialized (%04x device online)\n", __func__, devices_online);
}

static void dfplayer_HandleReply(void *context, void *token)
{
	fprintf(stderr, "%s: Reply\n", __func__);
}

static void dfplayer_HandleTrackFinished(void *context, void *token, uint16_t track_number, uint16_t device)
{
	fprintf(stderr, "%s: Track %u finished (device %04x)\n", __func__, track_number, device);
}

static void dfplayer_HandleDeviceState(void *context, void *token, uint16_t device, bool inserted)
{
	fprintf(stderr, "%s: Device %04x %s\n", __func__, device, (inserted) ? "inserted" : "removed");
}

static void dfplayer_HandleError(void *context, void *token, dfplayerError_e error)
{
	fprintf(stderr, "%s: Error %d\n", __func__, error);
}

static int dfplayer_SerialSend(void *context, void *token, uint8_t *data, uint32_t bytes)
{
	app_info_t *info = (app_info_t *) token;	
	uint8_t idx;

	printf("Send: ");
	for(idx = 0; idx < bytes; ++idx)
		printf("%s%02x", (idx == 0) ? "" : (idx % 16) ? " " : "\n", data[idx]);
	printf("\n");

	return (write(info->fd, data, bytes) == bytes) ? 0 : -1;
}
