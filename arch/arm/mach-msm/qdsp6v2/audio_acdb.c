/* Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/android_pmem.h>
#include "audio_acdb.h"


#define MAX_NETWORKS		9
#define NUM_ACTIVE_NETWORKS	6
#define VOCPROC_STREAM_OFFSET	NUM_ACTIVE_NETWORKS
#define VOCPROC_VOL_OFFSET	(NUM_ACTIVE_NETWORKS * 2)
#define NUM_VOCPROC_CAL_TYPES	(NUM_ACTIVE_NETWORKS * 3)
#define NUM_AUDPROC_CAL_TYPES	3
#define ACDB_BLOCK_SIZE		4096
#define NUM_VOCPROC_BLOCKS	18

enum {
	RX_CAL,
	TX_CAL,
	MAX_AUDPROC_TYPES
};

struct acdb_data {
	struct mutex		acdb_mutex;

	/* AudProc Cal */
	struct acdb_cal_block	audproc_cal[MAX_AUDPROC_TYPES];
	struct acdb_cal_block	audstrm_cal[MAX_AUDPROC_TYPES];
	struct acdb_cal_block	audvol_cal[MAX_AUDPROC_TYPES];

	/* VocProc Cal */
	struct acdb_cal_block	vocproc_cal[MAX_NETWORKS];
	struct acdb_cal_block	vocstrm_cal[MAX_NETWORKS];
	struct acdb_cal_block	vocvol_cal[MAX_NETWORKS];
	uint32_t		vocproc_cal_size;
	uint32_t		vocstrm_cal_size;
	uint32_t		vocvol_cal_size;

	/* Sidetone Cal */
	struct sidetone_cal	sidetone_cal;

	/* PMEM information */
	int			pmem_fd;
	unsigned long		paddr;
	unsigned long		kvaddr;
	unsigned long		pmem_len;
	struct file		*file;

};

static struct acdb_data		acdb_data;


void get_audproc_buffer_data(struct audproc_buffer_data *cal_buffers)
{
	int i;
	pr_debug("%s\n", __func__);

	if (cal_buffers == NULL) {
		pr_err("ACDB=> NULL pointer sent to %s\n", __func__);
		goto done;
	}

	for (i = 0; i < NUM_AUDPROC_BUFFERS; i++) {
		cal_buffers->phys_addr[i] = (uint32_t)
			(acdb_data.paddr +
			(NUM_VOCPROC_BLOCKS + i) * ACDB_BLOCK_SIZE);
		cal_buffers->buf_size[i] = ACDB_BLOCK_SIZE;
	}
done:
	return;
}

void store_audproc_cal(int32_t path, struct cal_block *cal_block)
{
	pr_debug("%s, path = %d\n", __func__, path);

	if (cal_block->cal_offset > acdb_data.pmem_len) {
		pr_err("%s: offset %d is > pmem_len %ld\n",
			__func__, cal_block->cal_offset,
			acdb_data.pmem_len);
		goto done;
	}
	if (path >= MAX_AUDPROC_TYPES) {
		pr_err("ACDB=> Bad path sent to %s, path: %d\n",
			__func__, path);
		goto done;
	}

	mutex_lock(&acdb_data.acdb_mutex);

	acdb_data.audproc_cal[path].cal_kvaddr =
		cal_block->cal_offset + acdb_data.kvaddr;
	acdb_data.audproc_cal[path].cal_paddr =
		cal_block->cal_offset + acdb_data.paddr;
	acdb_data.audproc_cal[path].cal_size =
		cal_block->cal_size;

	mutex_unlock(&acdb_data.acdb_mutex);
done:
	return;
}

void get_audproc_cal(int32_t path, struct acdb_cal_block *cal_block)
{
	pr_debug("%s, path = %d\n", __func__, path);

	if (cal_block == NULL) {
		pr_err("ACDB=> NULL pointer sent to %s\n", __func__);
		goto done;
	}
	if (path >= MAX_AUDPROC_TYPES) {
		pr_err("ACDB=> Bad path sent to %s, path: %d\n",
			__func__, path);
		goto done;
	}

	mutex_lock(&acdb_data.acdb_mutex);

	cal_block->cal_kvaddr = acdb_data.audproc_cal[path].cal_kvaddr;
	cal_block->cal_paddr = acdb_data.audproc_cal[path].cal_paddr;
	cal_block->cal_size = acdb_data.audproc_cal[path].cal_size;

	mutex_unlock(&acdb_data.acdb_mutex);
done:
	return;
}

void store_audstrm_cal(int32_t path, struct cal_block *cal_block)
{
	pr_debug("%s, path = %d\n", __func__, path);

	if (cal_block->cal_offset > acdb_data.pmem_len) {
		pr_err("%s: offset %d is > pmem_len %ld\n",
			__func__, cal_block->cal_offset,
			acdb_data.pmem_len);
		goto done;
	}
	if (path >= MAX_AUDPROC_TYPES) {
		pr_err("ACDB=> Bad path sent to %s, path: %d\n",
			__func__, path);
		goto done;
	}

	mutex_lock(&acdb_data.acdb_mutex);

	acdb_data.audstrm_cal[path].cal_kvaddr =
		cal_block->cal_offset + acdb_data.kvaddr;
	acdb_data.audstrm_cal[path].cal_paddr =
		cal_block->cal_offset + acdb_data.paddr;
	acdb_data.audstrm_cal[path].cal_size =
		cal_block->cal_size;

	mutex_unlock(&acdb_data.acdb_mutex);
done:
	return;
}

void get_audstrm_cal(int32_t path, struct acdb_cal_block *cal_block)
{
	pr_debug("%s, path = %d\n", __func__, path);

	if (cal_block == NULL) {
		pr_err("ACDB=> NULL pointer sent to %s\n", __func__);
		goto done;
	}
	if (path >= MAX_AUDPROC_TYPES) {
		pr_err("ACDB=> Bad path sent to %s, path: %d\n",
			__func__, path);
		goto done;
	}

	mutex_lock(&acdb_data.acdb_mutex);

	cal_block->cal_kvaddr = acdb_data.audstrm_cal[path].cal_kvaddr;
	cal_block->cal_paddr = acdb_data.audstrm_cal[path].cal_paddr;
	cal_block->cal_size = acdb_data.audstrm_cal[path].cal_size;

	mutex_unlock(&acdb_data.acdb_mutex);
done:
	return;
}

void store_audvol_cal(int32_t path, struct cal_block *cal_block)
{
	pr_debug("%s, path = %d\n", __func__, path);

	if (cal_block->cal_offset > acdb_data.pmem_len) {
		pr_err("%s: offset %d is > pmem_len %ld\n",
			__func__, cal_block->cal_offset,
			acdb_data.pmem_len);
		goto done;
	}
	if (path >= MAX_AUDPROC_TYPES) {
		pr_err("ACDB=> Bad path sent to %s, path: %d\n",
			__func__, path);
		goto done;
	}

	mutex_lock(&acdb_data.acdb_mutex);

	acdb_data.audvol_cal[path].cal_kvaddr =
		cal_block->cal_offset + acdb_data.kvaddr;
	acdb_data.audvol_cal[path].cal_paddr =
		cal_block->cal_offset + acdb_data.paddr;
	acdb_data.audvol_cal[path].cal_size =
		cal_block->cal_size;

	mutex_unlock(&acdb_data.acdb_mutex);
done:
	return;
}

void get_audvol_cal(int32_t path, struct acdb_cal_block *cal_block)
{
	pr_debug("%s, path = %d\n", __func__, path);

	if (cal_block == NULL) {
		pr_err("ACDB=> NULL pointer sent to %s\n", __func__);
		goto done;
	}
	if (path > MAX_AUDPROC_TYPES) {
		pr_err("ACDB=> Bad path sent to %s, path: %d\n",
			__func__, path);
		goto done;
	}

	mutex_lock(&acdb_data.acdb_mutex);

	cal_block->cal_kvaddr = acdb_data.audvol_cal[path].cal_kvaddr;
	cal_block->cal_paddr = acdb_data.audvol_cal[path].cal_paddr;
	cal_block->cal_size = acdb_data.audvol_cal[path].cal_size;

	mutex_unlock(&acdb_data.acdb_mutex);
done:
	return;
}


void store_vocproc_cal(int32_t len, struct cal_block *cal_blocks)
{
	int i;
	pr_debug("%s\n", __func__);

	if (len > MAX_NETWORKS) {
		pr_err("%s: Calibration sent for %d networks, only %d are "
			"supported!\n", __func__, len, MAX_NETWORKS);
		goto done;
	}


	mutex_lock(&acdb_data.acdb_mutex);

	for (i = 0; i < len; i++) {
		if (cal_blocks[i].cal_offset > acdb_data.pmem_len) {
			pr_err("%s: offset %d is > pmem_len %ld\n",
				__func__, cal_blocks[i].cal_offset,
				acdb_data.pmem_len);
			acdb_data.vocproc_cal[i].cal_size = 0;
		} else {
			acdb_data.vocproc_cal[i].cal_size =
				cal_blocks[i].cal_size;
			acdb_data.vocproc_cal[i].cal_paddr =
				cal_blocks[i].cal_offset +
				acdb_data.paddr;
			acdb_data.vocproc_cal[i].cal_kvaddr =
				cal_blocks[i].cal_offset +
				acdb_data.kvaddr;
		}
	}
	acdb_data.vocproc_cal_size = len;
	mutex_unlock(&acdb_data.acdb_mutex);
done:
	return;
}

void get_vocproc_cal(struct acdb_cal_data *cal_data)
{
	pr_debug("%s\n", __func__);

	if (cal_data == NULL) {
		pr_err("ACDB=> NULL pointer sent to %s\n", __func__);
		goto done;
	}

	mutex_lock(&acdb_data.acdb_mutex);

	cal_data->num_cal_blocks = acdb_data.vocproc_cal_size;
	cal_data->cal_blocks = &acdb_data.vocproc_cal[0];

	mutex_unlock(&acdb_data.acdb_mutex);
done:
	return;
}

void store_vocstrm_cal(int32_t len, struct cal_block *cal_blocks)
{
	int i;
	pr_debug("%s\n", __func__);

	if (len > MAX_NETWORKS) {
		pr_err("%s: Calibration sent for %d networks, only %d are "
			"supported!\n", __func__, len, MAX_NETWORKS);
		goto done;
	}

	mutex_lock(&acdb_data.acdb_mutex);

	for (i = 0; i < len; i++) {
		if (cal_blocks[i].cal_offset > acdb_data.pmem_len) {
			pr_err("%s: offset %d is > pmem_len %ld\n",
				__func__, cal_blocks[i].cal_offset,
				acdb_data.pmem_len);
			acdb_data.vocstrm_cal[i].cal_size = 0;
		} else {
			acdb_data.vocstrm_cal[i].cal_size =
				cal_blocks[i].cal_size;
			acdb_data.vocstrm_cal[i].cal_paddr =
				cal_blocks[i].cal_offset +
				acdb_data.paddr;
			acdb_data.vocstrm_cal[i].cal_kvaddr =
				cal_blocks[i].cal_offset +
				acdb_data.kvaddr;
		}
	}
	acdb_data.vocstrm_cal_size = len;
	mutex_unlock(&acdb_data.acdb_mutex);
done:
	return;
}

void get_vocstrm_cal(struct acdb_cal_data *cal_data)
{
	pr_debug("%s\n", __func__);

	if (cal_data == NULL) {
		pr_err("ACDB=> NULL pointer sent to %s\n", __func__);
		goto done;
	}

	mutex_lock(&acdb_data.acdb_mutex);

	cal_data->num_cal_blocks = acdb_data.vocstrm_cal_size;
	cal_data->cal_blocks = &acdb_data.vocstrm_cal[0];

	mutex_unlock(&acdb_data.acdb_mutex);
done:
	return;
}

void store_vocvol_cal(int32_t len, struct cal_block *cal_blocks)
{
	int i;
	pr_debug("%s\n", __func__);

	if (len > MAX_NETWORKS) {
		pr_err("%s: Calibration sent for %d networks, only %d are "
			"supported!\n", __func__, len, MAX_NETWORKS);
		goto done;
	}

	mutex_lock(&acdb_data.acdb_mutex);

	for (i = 0; i < len; i++) {
		if (cal_blocks[i].cal_offset > acdb_data.pmem_len) {
			pr_err("%s: offset %d is > pmem_len %ld\n",
				__func__, cal_blocks[i].cal_offset,
				acdb_data.pmem_len);
			acdb_data.vocvol_cal[i].cal_size = 0;
		} else {
			acdb_data.vocvol_cal[i].cal_size =
				cal_blocks[i].cal_size;
			acdb_data.vocvol_cal[i].cal_paddr =
				cal_blocks[i].cal_offset +
				acdb_data.paddr;
			acdb_data.vocvol_cal[i].cal_kvaddr =
				cal_blocks[i].cal_offset +
				acdb_data.kvaddr;
		}
	}
	acdb_data.vocvol_cal_size = len;
	mutex_unlock(&acdb_data.acdb_mutex);
done:
	return;
}

void get_vocvol_cal(struct acdb_cal_data *cal_data)
{
	pr_debug("%s\n", __func__);

	if (cal_data == NULL) {
		pr_err("ACDB=> NULL pointer sent to %s\n", __func__);
		goto done;
	}

	mutex_lock(&acdb_data.acdb_mutex);

	cal_data->num_cal_blocks = acdb_data.vocvol_cal_size;
	cal_data->cal_blocks = &acdb_data.vocvol_cal[0];

	mutex_unlock(&acdb_data.acdb_mutex);
done:
	return;
}

void store_sidetone_cal(struct sidetone_cal *cal_data)
{
	pr_debug("%s\n", __func__);

	mutex_lock(&acdb_data.acdb_mutex);

	acdb_data.sidetone_cal.enable = cal_data->enable;
	acdb_data.sidetone_cal.gain = cal_data->gain;

	mutex_unlock(&acdb_data.acdb_mutex);
}


void get_sidetone_cal(struct sidetone_cal *cal_data)
{
	pr_debug("%s\n", __func__);

	if (cal_data == NULL) {
		pr_err("ACDB=> NULL pointer sent to %s\n", __func__);
		goto done;
	}

	mutex_lock(&acdb_data.acdb_mutex);

	cal_data->enable = acdb_data.sidetone_cal.enable;
	cal_data->gain = acdb_data.sidetone_cal.gain;

	mutex_unlock(&acdb_data.acdb_mutex);
done:
	return;
}

static int acdb_open(struct inode *inode, struct file *f)
{
	s32 result = 0;
	pr_info("%s\n", __func__);

	if (acdb_data.pmem_fd) {
		pr_err("%s: ACDB opened but PMEM allocated, freeing PMEM!\n",
			__func__);
		put_pmem_file(acdb_data.file);
		acdb_data.pmem_fd = 0;
	}

	return result;
}

static int acdb_ioctl(struct inode *inode, struct file *f,
		unsigned int cmd, unsigned long arg)
{
	s32			result = 0;
	s32			audproc_path;
	s32			size;
	struct cal_block	data[MAX_NETWORKS];
	pr_debug("%s\n", __func__);

	switch (cmd) {
	case AUDIO_REGISTER_PMEM: {
		struct msm_audio_pmem_info info;
		struct audproc_buffer_data buffer;

		pr_debug("AUDIO_REGISTER_PMEM\n");
		if (copy_from_user(&info, (void *)arg, sizeof(info)))
			return -EFAULT;

		result = get_pmem_file(info.fd, &acdb_data.paddr,
					&acdb_data.kvaddr, &acdb_data.pmem_len,
					&acdb_data.file);
		if (result == 0)
			acdb_data.pmem_fd = info.fd;
		else{
			pr_err("%s: Could not register PMEM!!!\n", __func__);
			goto done;
		}

		pr_debug("AUDIO_REGISTER_PMEM done! paddr = 0x%lx, "
			"kvaddr = 0x%lx, len = x%lx\n", acdb_data.paddr,
			acdb_data.kvaddr, acdb_data.pmem_len);
		get_audproc_buffer_data(&buffer);
		result = adm_memory_map_regions(buffer.phys_addr, 0,
				buffer.buf_size,
				NUM_AUDPROC_BUFFERS);
		if (result < 0)
			pr_err("Audcal mmap did not work!\n");
		goto done;
	}
	case AUDIO_DEREGISTER_PMEM: {

		struct audproc_buffer_data buffer;

		get_audproc_buffer_data(&buffer);

		pr_debug("AUDIO_DEREGISTER_PMEM\n");

		result = adm_memory_unmap_regions(buffer.phys_addr,
				buffer.buf_size, NUM_AUDPROC_BUFFERS);
		if (result < 0)
			pr_err("Audcal unmap did not work!\n");
		if (acdb_data.pmem_fd) {
			put_pmem_file(acdb_data.file);
			acdb_data.pmem_fd = 0;
		}
		goto done;
	}
	}

	if (copy_from_user(&size, (void *) arg, sizeof(size))) {

		result = -EFAULT;
		goto done;
	}

	if (size <= 0) {
		pr_err("%s: Invalid size sent to driver: %d\n",
			__func__, size);
		result = -EFAULT;
		goto done;
	}

	if (copy_from_user(data, (void *)(arg + sizeof(size)), size)) {

		pr_err("%s: fail to copy table size %d\n", __func__, size);
		result = -EFAULT;
		goto done;
	}

	if (data == NULL) {
		pr_err("%s: NULL pointer sent to driver!\n", __func__);
		result = -EFAULT;
		goto done;
	}

	switch (cmd) {
	case AUDIO_SET_AUDPROC_TX_CAL:
		audproc_path = TX_CAL;
		if (size > sizeof(struct cal_block))
			pr_err("%s: More Audproc Cal then expected, "
				"size received: %d\n", __func__, size);
		store_audproc_cal(audproc_path, data);
		break;
	case AUDIO_SET_AUDPROC_RX_CAL:
		audproc_path = RX_CAL;
		if (size > sizeof(struct cal_block))
			pr_err("%s: More Audproc Cal then expected, "
				"size received: %d\n", __func__, size);
		store_audproc_cal(audproc_path, data);
		break;
	case AUDIO_SET_AUDPROC_TX_STREAM_CAL:
		audproc_path = TX_CAL;
		if (size > sizeof(struct cal_block))
			pr_err("%s: More Audproc Cal then expected, "
				"size received: %d\n", __func__, size);
		store_audstrm_cal(audproc_path, data);
		break;
	case AUDIO_SET_AUDPROC_RX_STREAM_CAL:
		audproc_path = RX_CAL;
		if (size > sizeof(struct cal_block))
			pr_err("%s: More Audproc Cal then expected, "
				"size received: %d\n", __func__, size);
		store_audstrm_cal(audproc_path, data);
		break;
	case AUDIO_SET_AUDPROC_TX_VOL_CAL:
		audproc_path = TX_CAL;
		if (size > sizeof(struct cal_block))
			pr_err("%s: More Audproc Cal then expected, "
				"size received: %d\n", __func__, size);
		store_audvol_cal(audproc_path, data);
	case AUDIO_SET_AUDPROC_RX_VOL_CAL:
		audproc_path = RX_CAL;
		if (size > sizeof(struct cal_block))
			pr_err("%s: More Audproc Cal then expected, "
				"size received: %d\n", __func__, size);
		store_audvol_cal(audproc_path, data);
		break;
	case AUDIO_SET_VOCPROC_CAL:
		store_vocproc_cal(size / sizeof(struct cal_block), data);
		break;
	case AUDIO_SET_VOCPROC_STREAM_CAL:
		store_vocstrm_cal(size / sizeof(struct cal_block), data);
		break;
	case AUDIO_SET_VOCPROC_VOL_CAL:
		store_vocvol_cal(size / sizeof(struct cal_block), data);
		break;
	case AUDIO_SET_SIDETONE_CAL:
		if (size > sizeof(struct sidetone_cal))
			pr_err("%s: More sidetone cal then expected, "
				"size received: %d\n", __func__, size);
		store_sidetone_cal((struct sidetone_cal *)data);
		break;
	default:
		pr_err("ACDB=> ACDB ioctl not found!\n");
	}

done:
	return result;
}

static int acdb_release(struct inode *inode, struct file *f)
{
	s32 result = 0;
	struct audproc_buffer_data buffer;
	pr_info("%s\n", __func__);


	get_audproc_buffer_data(&buffer);

	result = adm_memory_unmap_regions(buffer.phys_addr,
			buffer.buf_size, NUM_AUDPROC_BUFFERS);
	if (result < 0)
		pr_err("Audcal unmap did not work!\n");
	if (acdb_data.pmem_fd) {
		put_pmem_file(acdb_data.file);
		acdb_data.pmem_fd = 0;
	}
	return result;
}

static const struct file_operations acdb_fops = {
	.owner = THIS_MODULE,
	.open = acdb_open,
	.release = acdb_release,
	.ioctl = acdb_ioctl,
};

struct miscdevice acdb_misc = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "msm_acdb",
	.fops	= &acdb_fops,
};

static int __init acdb_init(void)
{
	pr_info("%s\n", __func__);
	memset(&acdb_data, 0, sizeof(acdb_data));
	mutex_init(&acdb_data.acdb_mutex);
	return misc_register(&acdb_misc);
}

static void __exit acdb_exit(void)
{
}

module_init(acdb_init);
module_exit(acdb_exit);

MODULE_DESCRIPTION("MSM 8x60 Audio ACDB driver");
MODULE_LICENSE("GPL v2");
