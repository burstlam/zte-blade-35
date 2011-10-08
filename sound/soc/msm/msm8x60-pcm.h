/*
 * Copyright (C) 2008 Google, Inc.
 * Copyright (C) 2008 HTC Corporation
 * Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can find it at http://www.fsf.org.
 */

#ifndef _MSM_PCM_H
#define _MSM_PCM_H
#include <mach/qdsp6v2/apr_audio.h>
#include <mach/qdsp6v2/q6asm.h>


/* Support unconventional sample rates 12000, 24000 as well */
#define USE_RATE                \
			(SNDRV_PCM_RATE_8000_48000 | SNDRV_PCM_RATE_KNOT)

extern int copy_count;

struct buffer {
	void *data;
	unsigned size;
	unsigned used;
	unsigned addr;
};

struct buffer_rec {
	void *data;
	unsigned int size;
	unsigned int read;
	unsigned int addr;
};

struct audio_locks {
	wait_queue_head_t read_wait;
	wait_queue_head_t write_wait;
	wait_queue_head_t wait;
	wait_queue_head_t eos_wait;
	wait_queue_head_t enable_wait;
};

extern struct audio_locks the_locks;

struct msm_audio {
	struct snd_pcm_substream *substream;
	unsigned int pcm_size;
	unsigned int pcm_count;
	unsigned int pcm_irq_pos;       /* IRQ position */
	uint16_t source; /* Encoding source bit mask */

	struct audio_client *audio_client;

	uint16_t session_id;

	uint32_t samp_rate;
	uint32_t channel_mode;
	uint32_t dsp_cnt;

	uint32_t device_events; /* device events interested in */
	int abort; /* set when error, like sample rate mismatch */

	int enabled;
	int eos_ack;
	int cmd_ack;
	atomic_t start;
	atomic_t out_count;
	atomic_t in_count;
	int periods;
};



/* platform data */
extern struct snd_soc_platform msm_soc_platform;
extern struct snd_soc_dai msm_dais[2];
extern struct snd_soc_codec_device soc_codec_dev_msm;

#endif /*_MSM_PCM_H*/
