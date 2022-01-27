/* SPDX-License-Identifier:	GPL-2.0+ */
/*
 * Copyright (C) 2018 MediaTek Incorporation. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <sf/sf.h>

static int do_spi_xfer(uint32_t tx_num, const void *tx_buf, uint32_t rx_num,
		       char **prx_buf)
{
	int fd;
	struct sf_op *op;
	char *rx_buf;
	int len;

	/* Write request */
	fd = open("/proc/sf", O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open /proc/sf for write: %d\n", fd);
		return -1;
	}

	len = sizeof(struct sf_op) + tx_num;
	op = calloc(len, 1);
	if (!op) {
		perror("Failed to alloc memory for tx\n");
		goto failed;
	}

	op->tx_num = tx_num;
	op->rx_num = rx_num;
	memcpy(op->tx_buf, tx_buf, tx_num);

	if (len != write(fd, op, len)) {
		fprintf(stderr, "Failed to write to /proc/sf: %d\n", len);
		goto failed;
	}

	close(fd);

	/* Read result */
	fd = open("/proc/sf", O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open /proc/sf for read: %d\n", fd);
		return -1;
	}

	rx_buf = malloc(op->rx_num);
	if (!rx_buf) {
		perror("Failed to alloc memory for rx\n");
		goto failed;
	}

	if (op->rx_num != read(fd, rx_buf, op->rx_num)) {
		fprintf(stderr, "Failed to read from /proc/sf: %d\n", op->rx_num);
		goto failed;
	}

	close(fd);

	if (prx_buf)
		*prx_buf = rx_buf;

	return 0;

failed:
	if (fd > 0)
		close(fd);

	return -1;
}

static void read_jedec_id(void)
{
	int i;
	char *buf;

	if (do_spi_xfer(1, "\x9f", 3, &buf)) {
		fprintf(stderr, "Failed to read JEDEC ID\n");
		return;
	}

	printf("JEDEC ID: ");
	for (i = 0; i < 3; i++)
		printf("%02x ", buf[i]);
	printf("\n");
	free(buf);
}

static void read_unique_id(void)
{
	int i;
	char *buf;

	if (do_spi_xfer(5, "\x4b\0\0\0\0", 8, &buf)) {
		fprintf(stderr, "Failed to read Unique ID\n");
		return;
	}

	printf("Unique ID: ");
	for (i = 0; i < 8; i++)
		printf("%02x ", buf[i]);
	printf("\n");
	free(buf);
}

static void read_sfdp(void)
{
	int i, j;
	char *buf, *b;
	uint32_t *p;

	if (do_spi_xfer(5, "\x5a\0\0\0\0", 0x100, &buf)) {
		fprintf(stderr, "Failed to read SFDP\n");
		return;
	}

	printf("SFDP:\n");
	p = (uint32_t *) buf;
	for (i = 0; i < 0x10; i++) {
		printf("%08x: %08x %08x %08x %08x     ", i * 0x10,
			p[i * 4], p[i * 4 + 1], p[i * 4 + 2], p[i * 4] + 3);
		b = (char *) &p[i * 4];
		for (j = 0; j < 0x10; j++) {
			if (b[j] <= 0x20 || b[j] >= 0x7f)
				printf(".");
			else
				printf("%c", b[j]);
		}
		printf("\n");
	}
	free(buf);
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		printf("Usage: sf [jedecid | uid | sfdp]\n");
		return 1;
	}

	if (!strcmp(argv[1], "jedecid"))
		read_jedec_id();
	else if (!strcmp(argv[1], "uid"))
		read_unique_id();
	else if (!strcmp(argv[1], "sfdp"))
		read_sfdp();

	return 0;
}
