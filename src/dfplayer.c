/*! \copyright 2016-2017 Zorxx Software. All rights reserved.
 *  \file dfplayer.c
 *  \brief DFPlayer serial interface library
 */
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "dfplayer_private.h"
#include "dfplayer.h"

#if defined DEBUG_PRINT
	#define DBG(...) fprintf(stderr, __VA_ARGS__)
#else
	#define DBG(...)
#endif

static void dfplayer_HandleReceivedMessage(dfplayer_context_t *ctxt);
static uint16_t dfplayer_CalculateChecksum(uint8_t *message, uint8_t length);
static int dfplayer_SendMessage(dfplayer_context_t *ctxt, uint8_t command, uint8_t parameter1, uint8_t parameter2,
	bool feedback);

/* ------------------------------------------------------------------------------------------
 * Exported Functions
 */

void *dfplayer_Initialize(void *token, dfplayer_init_info_t *init_info)
{
	dfplayer_context_t *ctxt = NULL;

	ctxt = (dfplayer_context_t *) malloc(sizeof(*ctxt));
	if(NULL == ctxt)
		return NULL;

	memset(ctxt, 0, sizeof(*ctxt));

	ctxt->token = token;
	ctxt->pfnHandleInitialize = init_info->pfnHandleInitialize;
	ctxt->pfnHandleTrackFinished = init_info->pfnHandleTrackFinished;
	ctxt->pfnHandleDeviceState = init_info->pfnHandleDeviceState;
	ctxt->pfnHandleError = init_info->pfnHandleError;
	ctxt->pfnHandleReply = init_info->pfnHandleReply;
	ctxt->pfnSendSerial = init_info->pfnSendSerial;
	ctxt->pfnHandleStatusResponse = init_info->pfnHandleStatusResponse;
	ctxt->pfnHandleVolumeResponse = init_info->pfnHandleVolumeResponse;
	ctxt->pfnHandleEqualizerResponse = init_info->pfnHandleEqualizerResponse;
	ctxt->pfnHandlePlaybackModeResponse = init_info->pfnHandlePlaybackModeResponse;
	ctxt->pfnHandleFileCountResponse = init_info->pfnHandleFileCountResponse;
	ctxt->pfnHandleCurrentTrackResponse = init_info->pfnHandleCurrentTrackResponse;

	return (void *) ctxt;	
}

void dfplayer_HandleSerialChar(void *context, uint8_t c)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	bool done = true; 
	bool update_checksum = true;

	assert(NULL != ctxt);

	switch(ctxt->message_offset)
	{
		case 0: /* Message start */
			if(c == DFPLAYER_MSG_START)
			{
				DBG("%s: start\n", __func__);
				done = false;
				update_checksum = false;
			}
			break;
		case 1: /* Message version */
			if(c == DFPLAYER_MSG_VERSION)
			{
				DBG("%s: version\n", __func__);
				done = false;
			}
			break;
		case 2: /* Data length */
			if(c == DFPLAYER_MSG_DATA_LENGTH)
			{
				DBG("%s: length\n", __func__);
				done = false;
			}
			break;
		case 3: /* Command */
			ctxt->message_command = c;
			DBG("%s: command=%02x\n", __func__, c);
			done = false;
			break;
		case 4: /* Feedback required */
			ctxt->message_feedback = c;
			DBG("%s: feedback=%u\n", __func__, c);
			done = false;
			break;
		case 5: /* Parameter 1 */
			ctxt->message_parameter[0] = c;
			DBG("%s: parameter1=%02x\n", __func__, c);
			done = false;
			break;
		case 6: /* Parameter 2 */
			ctxt->message_parameter[1] = c;
			DBG("%s: parameter2=%02x\n", __func__, c);
			done = false;
			break;
		case 7: /* Checksum (high byte) */
			ctxt->expected_checksum = ((uint16_t) c) << 8;
			DBG("%s: checksum high=%02x\n", __func__, c);
			update_checksum = false;
			done = false;
			break;
		case 8: /* Checksum (low byte) */
			ctxt->expected_checksum |= c; 
			DBG("%s: checksum low=%02x\n", __func__, c);
			update_checksum = false;
			done = false;
			break;
		case 9:
			if(c == DFPLAYER_MSG_END)
			{
				if(ctxt->calculated_checksum == ctxt->expected_checksum)
					dfplayer_HandleReceivedMessage(ctxt);
				else
				{
					DBG("%s: Checksum mismatch (calculated %04x, expected %04x\n",
						__func__, ctxt->calculated_checksum, ctxt->expected_checksum);
				}
			}
			break;
		default:
			break;
	}

	if(!done)
	{
		++(ctxt->message_offset);
		if(update_checksum)
			ctxt->calculated_checksum -= c;
	}
	else
	{
		DBG("%s: restart\n", __func__);
		ctxt->message_offset = 0;
		ctxt->calculated_checksum = 0;
		ctxt->expected_checksum = 0;
		ctxt->message_command = 0;  /* invalid command */
		ctxt->message_feedback = 0;
	}	
} /* dfplayer_HandleSerialChar */

int dfplayer_Play(void *context)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_PLAY, 0, 0, true);
}

int dfplayer_Pause(void *context)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_PAUSE, 0, 0, true);
}

int dfplayer_NextTrack(void *context)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_NEXT_TRACK, 0, 0, true);
}

int dfplayer_PreviousTrack(void *context)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_PREVIOUS_TRACK, 0, 0, true);
}

int dfplayer_SetTrack(void *context, uint16_t track_number)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;

	if(track_number > DFPLAYER_TRACK_MAX)
	{
		DBG("%s: Tack specified too high (%u, %u max)\n",
			__func__, track_number, DFPLAYER_TRACK_MAX);
	}

	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_SET_TRACK, track_number >> 8,
		track_number & 0xFF, true);
}

int dfplayer_SetFolder(void *context, uint8_t folder_number)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;

	if(folder_number > DFPLAYER_TRACK_MAX)
	{
		DBG("%s: Folder specified too high (%u, %u max)\n",
			__func__, folder_number, DFPLAYER_FOLDER_MAX);
	}

	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_SET_FOLDER, 0, folder_number, true);
}

int dfplayer_SetPlaybackSource(void *context, uint16_t device)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_SET_PLAYBACK_SOURCE, device >> 8,
		device & 0xFF, true);
}

int dfplayer_EnableRepeatPlayback(void *context, bool enable)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_REPEAT, 0, (enable) ? 1 : 0, true);
}

int dfplayer_SetEqualizer(void *context, dfplayerEqualizer_e mode)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_SET_EQUALIZER, 0, (uint8_t) mode, true);
}

int dfplayer_SetPlaybackMode(void *context, dfplayerPlaybackMode_e mode)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_SET_PLAYBACK_MODE, 0, (uint8_t) mode, true);
}

int dfplayer_VolumeUp(void *context)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_VOLUME_UP, 0, 0, true);
}

int dfplayer_VolumeDown(void *context)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_VOLUME_DOWN, 0, 0, true);
}

int dfplayer_VolumeSet(void *context, uint8_t volume)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	if(volume > DFPLAYER_VOL_MAX)
	{
		DBG("%s: Volume specified too high (%u, %u max)\n", __func__, volume, DFPLAYER_VOL_MAX);
		return -1;
	}
	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_VOLUME_SET, 0, volume, true);
}

int dfplayer_SetStandbyMode(void *context, bool enable)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	return dfplayer_SendMessage(ctxt,
		(enable) ? DFPLAYER_CMD_POWER_MODE_STANDBY : DFPLAYER_CMD_POWER_MODE_NORMAL,
		0, 0, true);
}

int dfplayer_Reset(void *context)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_RESET, 0, 0, true);
}

int dfplayer_QueryStatus(void *context)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_QUERY_STATUS, 0, 0, true);
}

int dfplayer_QueryVolume(void *context)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_QUERY_VOLUME, 0, 0, true);
}

int dfplayer_QueryEqualizer(void *context)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_QUERY_EQUALIZER, 0, 0, true);
}

int dfplayer_QueryPlaybackMode(void *context)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	return dfplayer_SendMessage(ctxt, DFPLAYER_CMD_QUERY_PLAYBACK_MODE, 0, 0, true);
}

int dfplayer_QueryFileCount(void *context, uint8_t device)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	uint8_t command;
	switch(device)
	{
		case DFPLAYER_DEVICE_TFCARD: command = DFPLAYER_CMD_QUERY_TFCARD_FILES; break;
		case DFPLAYER_DEVICE_UDISK: command = DFPLAYER_CMD_QUERY_UDISK_FILES; break;
		case DFPLAYER_DEVICE_FLASH: command = DFPLAYER_CMD_QUERY_FLASH_FILES; break;
		default: return -1;
	}
	return dfplayer_SendMessage(ctxt, command, 0, 0, true);
}

int dfplayer_QueryCurrentTrack(void *context, uint8_t device)
{
	dfplayer_context_t *ctxt = (dfplayer_context_t *) context;
	uint8_t command;
	switch(device)
	{
		case DFPLAYER_DEVICE_TFCARD: command = DFPLAYER_CMD_QUERY_TFCARD_TRACK; break;
		case DFPLAYER_DEVICE_UDISK: command = DFPLAYER_CMD_QUERY_UDISK_TRACK; break;
		case DFPLAYER_DEVICE_FLASH: command = DFPLAYER_CMD_QUERY_FLASH_TRACK; break;
		default: return -1;
	}
	return dfplayer_SendMessage(ctxt, command, 0, 0, true);
}

/* ------------------------------------------------------------------------------------------
 * Private Helper Functions
 */

static uint16_t dfplayer_CalculateChecksum(uint8_t *buffer, uint8_t length)
{
	uint16_t checksum = 0;
	uint8_t idx;

	for(idx = 0; idx < length; ++idx)
		checksum -= buffer[idx];
	return checksum;
}

static int dfplayer_SendMessage(dfplayer_context_t *ctxt, uint8_t command, uint8_t parameter1, uint8_t parameter2,
	bool feedback)
{
	uint8_t message[DFPLAYER_MSG_LENGTH];
	uint16_t checksum;

	if(ctxt->pfnSendSerial == NULL)
	{
		DBG("%s: No serial function handler specified\n", __func__);
		return -1;
	}

	message[0] = DFPLAYER_MSG_START;
	message[1] = DFPLAYER_MSG_VERSION;
	message[2] = DFPLAYER_MSG_DATA_LENGTH;
	message[3] = command;
	message[4] = (feedback) ? 1 : 0;
	message[5] = parameter1;
	message[6] = parameter2;
	checksum = dfplayer_CalculateChecksum(&message[1], 6); /* checksum skips start byte */
	message[7] = checksum >> 8;
	message[8] = checksum & 0xFF;
	message[9] = DFPLAYER_MSG_END;

	return ctxt->pfnSendSerial(ctxt, ctxt->token, message, DFPLAYER_MSG_LENGTH);
}

static void dfplayer_HandleReceivedMessage(dfplayer_context_t *ctxt)
{
	DBG("%s: command=%02x, feedback=%u, parameter1=%02x, parameter2=%02x\n", __func__,
		ctxt->message_command, ctxt->message_feedback, ctxt->message_parameter[0],
		ctxt->message_parameter[1]);

	switch(ctxt->message_command)
	{
		case DFPLAYER_CMD_UDISK_FINISH:
			if(ctxt->pfnHandleTrackFinished != NULL)
			{
				uint16_t track_number = ((uint16_t) ctxt->message_parameter[0]) << 8
					| ctxt->message_parameter[1];
				ctxt->pfnHandleTrackFinished(ctxt, ctxt->token, track_number, DFPLAYER_DEVICE_UDISK);
			}
			break;

		case DFPLAYER_CMD_TFCARD_FINISH:
			if(ctxt->pfnHandleTrackFinished != NULL)
			{
				uint16_t track_number = ((uint16_t) ctxt->message_parameter[0]) << 8
					| ctxt->message_parameter[1];
				ctxt->pfnHandleTrackFinished(ctxt, ctxt->token, track_number, DFPLAYER_DEVICE_TFCARD);
			}
			break;

		case DFPLAYER_CMD_FLASH_FINISH:
			if(ctxt->pfnHandleTrackFinished != NULL)
			{
				uint16_t track_number = ((uint16_t) ctxt->message_parameter[0]) << 8
					| ctxt->message_parameter[1];
				ctxt->pfnHandleTrackFinished(ctxt, ctxt->token, track_number, DFPLAYER_DEVICE_FLASH);
			}
			break;
			
		case DFPLAYER_CMD_INITIALIZE:
			if(ctxt->pfnHandleInitialize != NULL)
			{
				uint16_t devices_online = ((uint16_t) ctxt->message_parameter[0]) << 8
					| ctxt->message_parameter[1];
				ctxt->pfnHandleInitialize(ctxt, ctxt->token, devices_online);
			}
			break;

		case DFPLAYER_CMD_DEVICE_PUSH_IN:
			if(ctxt->pfnHandleDeviceState != NULL)
			{
				uint16_t device = ((uint16_t) ctxt->message_parameter[0]) << 8
					| ctxt->message_parameter[1];
				ctxt->pfnHandleDeviceState(ctxt, ctxt->token, device, true);
			}
			break;

		case DFPLAYER_CMD_DEVICE_PULL_OUT:
			if(ctxt->pfnHandleDeviceState != NULL)
			{
				uint16_t device = ((uint16_t) ctxt->message_parameter[0]) << 8
					| ctxt->message_parameter[1];
				ctxt->pfnHandleDeviceState(ctxt, ctxt->token, device, false);
			}
			break;

		case DFPLAYER_CMD_ERROR_REPORT:
			if(ctxt->pfnHandleError != NULL)
			{
				uint16_t error = ((uint16_t) ctxt->message_parameter[0]) << 8
					| ctxt->message_parameter[1];
				ctxt->pfnHandleError(ctxt, ctxt->token, (dfplayerError_e) error);
			}
			break;

		case DFPLAYER_CMD_REPLY:
			if(ctxt->pfnHandleReply != NULL)
			{
				ctxt->pfnHandleReply(ctxt, ctxt->token);
			}
			break;

		case DFPLAYER_CMD_QUERY_STATUS:
			if(ctxt->pfnHandleStatusResponse != NULL)
			{
				bool playing = (ctxt->message_parameter[0]) ? true : false; 
				ctxt->pfnHandleStatusResponse(ctxt, ctxt->token, playing); 
			}
			break;

		case DFPLAYER_CMD_QUERY_VOLUME:
			if(ctxt->pfnHandleVolumeResponse != NULL)
			{
				uint16_t volume = ((uint16_t) ctxt->message_parameter[0]) << 8
					| ctxt->message_parameter[1];
				ctxt->pfnHandleVolumeResponse(ctxt, ctxt->token, volume);
			} 
			break;

		case DFPLAYER_CMD_QUERY_EQUALIZER:
			if(ctxt->pfnHandleEqualizerResponse != NULL)
			{
				dfplayerEqualizer_e mode = (dfplayerEqualizer_e) ctxt->message_parameter[0];
				ctxt->pfnHandleEqualizerResponse(ctxt, ctxt->token, mode);
			}
			break;

		case DFPLAYER_CMD_QUERY_PLAYBACK_MODE:
			if(ctxt->pfnHandlePlaybackModeResponse != NULL)
			{
				dfplayerPlaybackMode_e mode = (dfplayerEqualizer_e) ctxt->message_parameter[0];
				ctxt->pfnHandlePlaybackModeResponse(ctxt, ctxt->token, mode);
			}
			break;

		case DFPLAYER_CMD_QUERY_TFCARD_FILES:
		case DFPLAYER_CMD_QUERY_UDISK_FILES:
		case DFPLAYER_CMD_QUERY_FLASH_FILES:
			if(ctxt->pfnHandleFileCountResponse != NULL)
			{
				uint8_t device = 0;
				switch(ctxt->message_command)
				{
					case DFPLAYER_CMD_QUERY_TFCARD_FILES: device = DFPLAYER_DEVICE_TFCARD; break;
					case DFPLAYER_CMD_QUERY_UDISK_FILES: device = DFPLAYER_DEVICE_UDISK; break;
					case DFPLAYER_CMD_QUERY_FLASH_FILES: device = DFPLAYER_DEVICE_FLASH; break;
				}
				uint16_t count = ((uint16_t) ctxt->message_parameter[0]) << 8
					| ctxt->message_parameter[1];
				ctxt->pfnHandleFileCountResponse(ctxt, ctxt->token, device, count);
			}
			break;
		case DFPLAYER_CMD_QUERY_TFCARD_TRACK:
		case DFPLAYER_CMD_QUERY_UDISK_TRACK:
		case DFPLAYER_CMD_QUERY_FLASH_TRACK:
			if(ctxt->pfnHandleCurrentTrackResponse != NULL)
			{
				uint8_t device = 0;
				switch(ctxt->message_command)
				{
					case DFPLAYER_CMD_QUERY_TFCARD_TRACK: device = DFPLAYER_DEVICE_TFCARD; break;
					case DFPLAYER_CMD_QUERY_UDISK_TRACK: device = DFPLAYER_DEVICE_UDISK; break;
					case DFPLAYER_CMD_QUERY_FLASH_TRACK: device = DFPLAYER_DEVICE_FLASH; break;
				}
				uint16_t track = ((uint16_t) ctxt->message_parameter[0]) << 8
					| ctxt->message_parameter[1];
				ctxt->pfnHandleCurrentTrackResponse(ctxt, ctxt->token, device, track);
			}
			break;

		case DFPLAYER_CMD_QUERY_VERSION: /* TODO */
		default:
			DBG("%s: Unknown message command (%02x)\n", __func__, ctxt->message_command);
			break;
	}
}
