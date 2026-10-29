// SPDX-License-Identifier: GPL-2.0-only
/*
 * init/noinitramfs.c
 *
 * Copyright (C) 2006, NXP Semiconductors, All Rights Reserved
 * Author: Jean-Paul Saman <jean-paul.saman@nxp.com>
 */
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kdev_t.h>
#include <linux/dirent.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/completion.h>
#include <linux/aw_rpmsg.h>

unsigned long initrd_start, initrd_end;
int initrd_below_start_ok;

phys_addr_t phys_initrd_start;
unsigned long phys_initrd_size;

static const char *buf_start;
static const char *buf_end;

struct ramdisk_file_header {
	u32 ino;
	u32 mode;
	u32 uid;
	u32 gid;
	u32 nlink;
	u32 mtime;
	u32 body_len;
	u32 major;
	u32 minor;
	u32 rdev;
	u32 name_len;
	const char *name;
	const void *data;
};

struct readahead_file {
	struct ramdisk_file_header h;
};

#define N_ALIGN(len) ((((len) + 1) & ~3) + 2)

static const char *parse_header(const char *s, struct ramdisk_file_header *h)
{
	unsigned long parsed[13];
	char buf[9];
	int i;

	if (memcmp(s, "070701", 6)) {
		pr_err("no cpio magic");
		return NULL;
	}

	buf[8] = '\0';
	for (i = 0, s += 6; i < 13; i++, s += 8) {
		memcpy(buf, s, 8);
		parsed[i] = simple_strtoul(buf, NULL, 16);
	}
	h->ino = parsed[0];
	h->mode = parsed[1];
	h->uid = parsed[2];
	h->gid = parsed[3];
	h->nlink = parsed[4];
	h->mtime = parsed[5]; /* breaks in y2106 */
	h->body_len = parsed[6];
	h->major = parsed[7];
	h->minor = parsed[8];
	h->rdev = new_encode_dev(MKDEV(parsed[9], parsed[10]));
	h->name_len = parsed[11];
	h->name = s;
	s += N_ALIGN(h->name_len);
	h->data = (void *)s;

	return (const char *)ALIGN((unsigned long)h->data + h->body_len, 4);
}

static int __readahead_readpage(void *data, struct page *page)
{
	struct readahead_file *f = data;
	unsigned long index = page->index;
	unsigned long copy, offset;
	unsigned long size = f->h.body_len;
	void *p;

	offset = index << PAGE_SHIFT;
	if (unlikely((size >> PAGE_SHIFT) == index))
		copy = size - offset;
	else
		copy = PAGE_SIZE;

	p = kmap_atomic(page);
	memcpy(p, f->h.data + offset, copy);
	kunmap_atomic(p);

	SetPageUptodate(page);
	unlock_page(page);

	return 0;
}

static inline int put_file_to_page_cache(struct readahead_file *f)
{
	unsigned long index = 0;
	unsigned copied = 0;
	unsigned amt;
	unsigned size = f->h.body_len;
	struct page *page;
	struct file *file;

	file = filp_open(f->h.name, O_RDONLY, 0);
	if (IS_ERR(file)) {
		/* file not exist skip.. */
		return PTR_ERR(file);
	}

	do {
		amt = size - copied;
		if (size > PAGE_SIZE)
			amt = PAGE_SIZE;
		page = read_cache_page(file->f_mapping, index, __readahead_readpage, f);
		if (IS_ERR(page))
			return PTR_ERR(page);
		put_page(page);
		copied += amt;
		index++;
	} while(copied < size);

	filp_close(file, NULL);

	return size;
}

void parse_readahead_content(void)
{
	const char *p;
	struct readahead_file f;

	if (!initrd_below_start_ok) {
		pr_warn("not found rootfs readahead data\n");
		return;
	}

	buf_start = (const char *)initrd_start;
	buf_end = (const char *)initrd_end;

	p = buf_start;

	if (!memcmp(p, "070707", 6)) {
		pr_err("incorrect cpio method used: use -H newc option");
		return;
	}

	if (memcmp(p, "070701", 6)) {
		pr_err("no cpio magic");
		return;
	}

	while (p < buf_end) {
		p = parse_header(p, &f.h);
		if (!p)
			break;

		if (!strcmp(f.h.name, "TRAILER!!!"))
			break;

		if (!S_ISREG(f.h.mode))
			continue;

#ifdef DEBUG
		pr_debug("file: %s %u add to page cache\n", f.h.name, f.h.body_len);
#endif

		put_file_to_page_cache(&f);
	}

	free_reserved_area((void *)buf_start, (void *)buf_end, -1, "readahead");
}
