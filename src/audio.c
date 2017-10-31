#include "audio.h"
#include "dma.h"
#include "timer.h"


// Mixer adapted from: http://deku.gbadev.org

// The mixing buffer being DMA'd to the audio chip
s8 mixBuffers[2][AUDIO_SAMPLES_PER_FRAME];

// Current mixing state
SOUND_VARS audio_vars;

// Channels to mix
SOUND_CHANNEL audio_channels[AUDIO_MAX_CHANNELS];

void audio_init() {
	// Clear mixbuffer
	dma3_fill(mixBuffers, 0, sizeof(mixBuffers)/4, DMA_NOW|DMA_32);

	// Init mixing vars
	audio_vars.mixBufferBase = &mixBuffers[0][0];
	audio_vars.mixBufferSize = AUDIO_SAMPLES_PER_FRAME;
	audio_vars.activeBuffer = 1;

	// Init channels
	for (u32 i = 0; i < AUDIO_MAX_CHANNELS; i++) {
		audio_channels[i].data			= 0;
		audio_channels[i].pos			= 0;
		audio_channels[i].stride		= 0;
		audio_channels[i].vol			= 0;
		audio_channels[i].length		= 0;
		audio_channels[i].loopLength	= 0;
	}

	// Init sound chip
	REG_SOUNDBIAS = 0x4200;
	REG_SOUNDCNT_H = 	SDS_DMG100|SDS_A100|SDS_B100|
						SDS_AR|SDS_AL|SDS_ATMR0|SDS_ARESET;
	REG_SOUNDCNT_X = SOUND_ENABLE;

	//Formula for playback frequency is: 0xFFFF-round(cpuFreq/playbackFreq)
	// samples/seconds = 16777216(c/s)/18157(hz)
	REG_TM0D = AUDIO_TM0_START;
	REG_TM0CNT = TIMER_ENABLE;

	// Start the dma
	dma1_fifo_repeat(REG_FIFO_A, audio_vars.mixBufferBase);
}

void audio_vsync() {
	// buffer 1 just got over
	if(audio_vars.activeBuffer == 1) { // Start playing buffer 0
		dma1_fifo_repeat(REG_FIFO_A, audio_vars.mixBufferBase);
		// Set the current buffer pointer to the start of buffer 1
		audio_vars.curMixBuffer = audio_vars.mixBufferBase + audio_vars.mixBufferSize;
		audio_vars.activeBuffer = 0;
	} else { // buffer 0 just got over
		// DMA points to buffer 1 already, so don't bother stopping and resetting it
		// Set the current buffer pointer to the start of buffer 0
		audio_vars.curMixBuffer = audio_vars.mixBufferBase;
		audio_vars.activeBuffer = 1;
	}
}

void audio_mix() {
	// Temporary double size accumulation buffer
	s16 tempBuffer[AUDIO_SAMPLES_PER_FRAME];
	dma3_fill(tempBuffer, 0, audio_vars.mixBufferSize*sizeof(s16)/4, DMA_NOW|DMA_32);

	for (u32 c = 0; c < AUDIO_MAX_CHANNELS; c++) {
		SOUND_CHANNEL* channel = &audio_channels[c];
		// If channel is playing
		if ( channel->data ) {
			// For each sample required next frame
			for(u32 i = 0; i < audio_vars.mixBufferSize; i++) {
				tempBuffer[i] += channel->data[channel->pos>>10] * channel->vol;
				channel->pos += channel->stride;

				if(channel->pos >= channel->length) {
					if(channel->loopLength == 0) {
						channel->data = 0; // disable channel
						break;
					} else {
						while(channel->pos >= channel->length) {
							channel->pos -= channel->loopLength;
						}
					}
				}
			}
		}
	}

	// now downsample 16bit->8bit into the mix buffer
	for(u32 i = 0; i < audio_vars.mixBufferSize; i++) {
		// >>6 to divide off the volume, >>2 to divide by 4 channels to prevent overflow.
		audio_vars.curMixBuffer[i] = tempBuffer[i]>>8;
	}
}


void audio_play(u32 channel, s8 *data, u32 length, u32 freq, u32 vol, u32 loop) {
	// Disable channel
	audio_channels[channel].data = 0;

	// Reset play position
	audio_channels[channel].pos	= 0;

	// Setup length (20.12 fixed point)
	audio_channels[channel].length	= length<<10;

	// Setup sample stride based on mixing/recorded sample rate
	// Assumed to be the same for now
	audio_channels[channel].stride	= (freq<<10)/AUDIO_SAMPLE_RATE;

	// Setup volume (1.6 fixed-point)
	audio_channels[channel].vol	= vol<<6;

	// Setup loop state
	audio_channels[channel].loopLength	= (loop)?length<<10:0;

	// Renable channel
	audio_channels[channel].data = data;
}

#if 0
// Unused square soundwave test
static void square_sound1() {
	REG_TM0D = -0x8000;
	REG_TM0CNT = FREQ_1024|TIMER_ENABLE;
	REG_TM1CNT = CASCADE|TIMER_ENABLE;

	REG_SOUNDCNT_X = SOUND_ENABLE;
	REG_SOUNDCNT_L = SDMG_LSQR1|SDMG_RSQR1|SOUNDCNT_L_SET_VOL(7, 7);
	REG_SOUNDCNT_H = SDS_DMG100;

	REG_SOUND1CNT_L = SOUND1CNT_L_SWEEP_NUM(6)|SWEEP_UP|SWEEP_39_1_MS;
	REG_SOUND1CNT_H	= DUTY_PATTERN_50|ENVELOPE_DOWN
						|SOUND1CNT_H_ENVELOPE_STEP_TIME(7)
						|SOUND1CNT_H_ENVOLOPE_INIT_VOL(15);
	REG_SOUND1CNT_X	= SOUND1CNT_X_SND_FREQ(1024)|SND_RESET;
}
#endif
