/* SPDX-License-Identifier:	GPL-2.0+ */
/*
 * Copyright (C) 2018 MediaTek Incorporation. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#include "sf.h"

#define SF_NAME "sf"

#if defined(MT7622)
/* MT7622 SPI pins to GPIO */
#define SPI_CLK			10
#define SPI_CS			13
#define SPI_MOSI		11
#define SPI_MISO		12
#define SPI_WP			8
#define SPI_HOLD		9

#define GPIO_BASE		0x10211000
#elif defined(MT7629)
/* Leopard SPI pins to GPIO */
#define SPI_CLK			62
#define SPI_CS			63
#define SPI_MOSI		64
#define SPI_MISO		65
#define SPI_WP			66
#define SPI_HOLD		67

#define GPIO_BASE		0x10217000
#else
#error Please specify a platform!
#endif

#define GPIO_SIZE		0x1000

#define GPIO_DIR(x)		(0x10 * (x))
#define GPIO_DOUT(x)		(0x100 + 0x10 * (x))
#define GPIO_DIN(x)		(0x200 + 0x10 * (x))
#define GPIO_MODE(x)		(0x300 + 0x10 * (x))

struct gpio_sts_backup {
	u8 dir_clk;
	u8 dir_cs;
	u8 dir_mosi;
	u8 dir_miso;
	u8 dir_wp;
	u8 dir_hold;
	u8 mode_clk;
	u8 mode_cs;
	u8 mode_mosi;
	u8 mode_miso;
	u8 mode_wp;
	u8 mode_hold;
	u8 mode_spi;
};

static struct proc_dir_entry *sf_entry;

static void __iomem *gpio_base;

static struct mutex lock;

static void *rx_buf;
static u32 rx_bufsz;

static void gpio_set_dir(u32 n, int out)
{
	u32 val;

	val = readl(gpio_base + GPIO_DIR(n / 32));
	if (out)
		val |= 1 << (n % 32);
	else
		val &= ~(1 << (n % 32));
	writel(val, gpio_base + GPIO_DIR(n / 32));
}

static u32 gpio_get_dir(u32 n)
{
	return readl(gpio_base + GPIO_DIR(n / 32)) & (1 << (n % 32));
}

static void gpio_out(u32 n, int high)
{
	u32 val;

	val = readl(gpio_base + GPIO_DOUT(n / 32));
	if (high)
		val |= 1 << (n % 32);
	else
		val &= ~(1 << (n % 32));
	writel(val, gpio_base + GPIO_DOUT(n / 32));
}

static u32 gpio_in(u32 n)
{
	return readl(gpio_base + GPIO_DIN(n / 32)) & (1 << (n % 32));
}

static void gpio_set_mode(u32 n, u32 mode)
{
	u32 val;

	val = readl(gpio_base + GPIO_MODE(n / 8));
	val &= ~(0xff << (4 * (n % 8)));
	val |= mode << (4 * (n % 8));
	writel(val, gpio_base + GPIO_MODE(n / 8));
}

static u32 gpio_get_mode(u32 n)
{
	u32 val;

	val = readl(gpio_base + GPIO_MODE(n / 8));
	val >>= 4 * (n % 8);
	val &= 0xff;

	return val;
}
#if defined(MT7622)
static void enter_spi_gpio_mode(struct gpio_sts_backup *sts)
{
	sts->mode_spi =  gpio_get_mode(2);

	sts->dir_clk = gpio_get_dir(SPI_CLK);
	sts->dir_cs = gpio_get_dir(SPI_CS);
	sts->dir_mosi = gpio_get_dir(SPI_MOSI);
	sts->dir_miso = gpio_get_dir(SPI_MISO);

	sts->dir_wp = gpio_get_dir(SPI_WP);
	sts->mode_wp = gpio_get_mode(45);

	sts->dir_hold = gpio_get_dir(SPI_HOLD);
	sts->mode_hold = gpio_get_mode(46);

	gpio_set_mode(2, 1);
	gpio_set_mode(SPI_CLK, 0);
	gpio_set_mode(SPI_CS, 0);
	gpio_set_mode(SPI_MOSI, 0);
	gpio_set_mode(SPI_MISO, 0);
	gpio_set_mode(SPI_WP, 0);
	gpio_set_mode(SPI_HOLD, 0);

	gpio_set_dir(SPI_CLK, 1);
	gpio_set_dir(SPI_CS, 1);
	gpio_set_dir(SPI_MOSI, 1);
	gpio_set_dir(SPI_MISO, 0);
	gpio_set_dir(SPI_WP, 1);
	gpio_set_dir(SPI_HOLD, 1);
}

static void leave_spi_gpio_mode(struct gpio_sts_backup *sts)
{
	gpio_set_dir(SPI_CLK, sts->dir_clk);
	gpio_set_dir(SPI_CS, sts->dir_cs);
	gpio_set_dir(SPI_MOSI, sts->dir_mosi);
	gpio_set_dir(SPI_MISO, sts->dir_miso);
	gpio_set_dir(SPI_WP, sts->dir_wp);
	gpio_set_dir(SPI_HOLD, sts->dir_hold);

	gpio_set_mode(45, sts->mode_wp);
	gpio_set_mode(46, sts->mode_hold);
	gpio_set_mode(2, sts->mode_spi);
}
#elif defined(MT7629)
static void enter_spi_gpio_mode(struct gpio_sts_backup *sts)
{
	sts->dir_clk = gpio_get_dir(SPI_CLK);
	sts->mode_clk = gpio_get_mode(SPI_CLK);

	sts->dir_cs = gpio_get_dir(SPI_CS);
	sts->mode_cs = gpio_get_mode(SPI_CS);

	sts->dir_mosi = gpio_get_dir(SPI_MOSI);
	sts->mode_mosi = gpio_get_mode(SPI_MOSI);

	sts->dir_miso = gpio_get_dir(SPI_MISO);
	sts->mode_miso = gpio_get_mode(SPI_MISO);

	sts->dir_wp = gpio_get_dir(SPI_WP);
	sts->mode_wp = gpio_get_mode(SPI_WP);

	sts->dir_hold = gpio_get_dir(SPI_HOLD);
	sts->mode_hold = gpio_get_mode(SPI_HOLD);

	gpio_set_mode(SPI_CLK, 0);
	gpio_set_mode(SPI_CS, 0);
	gpio_set_mode(SPI_MOSI, 0);
	gpio_set_mode(SPI_MISO, 0);
	gpio_set_mode(SPI_WP, 0);
	gpio_set_mode(SPI_HOLD, 0);

	gpio_set_dir(SPI_CLK, 1);
	gpio_set_dir(SPI_CS, 1);
	gpio_set_dir(SPI_MOSI, 1);
	gpio_set_dir(SPI_MISO, 0);
	gpio_set_dir(SPI_WP, 1);
	gpio_set_dir(SPI_HOLD, 1);
}

static void leave_spi_gpio_mode(struct gpio_sts_backup *sts)
{
	gpio_set_dir(SPI_CLK, sts->dir_clk);
	gpio_set_dir(SPI_CS, sts->dir_cs);
	gpio_set_dir(SPI_MOSI, sts->dir_mosi);
	gpio_set_dir(SPI_MISO, sts->dir_miso);
	gpio_set_dir(SPI_WP, sts->dir_wp);
	gpio_set_dir(SPI_HOLD, sts->dir_hold);

	gpio_set_mode(SPI_CLK, sts->mode_clk);
	gpio_set_mode(SPI_CS, sts->mode_cs);
	gpio_set_mode(SPI_MOSI, sts->mode_mosi);
	gpio_set_mode(SPI_MISO, sts->mode_miso);
	gpio_set_mode(SPI_WP, sts->mode_wp);
	gpio_set_mode(SPI_HOLD, sts->mode_hold);
}
#endif

static void spi_gpio_chip_select(int active)
{
	gpio_out(SPI_CS, 1);

	if (active)
	{
		/* /WP and /HOLD to high level */
		gpio_out(SPI_WP, 1);
		gpio_out(SPI_HOLD, 1);
		/* Mode 0 */
		gpio_out(SPI_CLK, 0);
		gpio_out(SPI_CS, 0);
	}
}

static void spi_gpio_tx_byte(u8 data)
{
	u32 i;

	for (i = 0; i < 8; i++)
	{
		if (data & 0x80)
			gpio_out(SPI_MOSI, 1);
		else
			gpio_out(SPI_MOSI, 0);
		gpio_out(SPI_CLK, 1);
		gpio_out(SPI_CLK, 0);
		data <<= 1;
	}
}

static void spi_gpio_tx(const u8 *data, u32 size)
{
	u32 i;

	for (i = 0; i < size; i++)
		spi_gpio_tx_byte(data[i]);
}

static u8 spi_gpio_rx_byte(void)
{
	u32 i;
	u8 data = 0;

	gpio_out(SPI_MOSI, 0);

	for (i = 0; i < 8; i++)
	{
		data <<= 1;
		gpio_out( SPI_CLK, 1);
		if (gpio_in(SPI_MISO))
			data |= 1;
		gpio_out(SPI_CLK, 0);
	}

	return data;
}

static void spi_gpio_rx(u8 *data, u32 size)
{
	u32 i;

	for (i = 0; i < size; i++)
		data[i] = spi_gpio_rx_byte();
}

/* Used for procfs only */
static int do_spi_xfer(u32 tx_num, const void __user *tx_buf, u32 rx_num)
{
	u8 buf[0x100];
	u32 c, n;
	int err = 0;

	rx_bufsz = 0;

	spi_gpio_chip_select(1);

	c = 0;
	while (c < tx_num) {
		n = min(tx_num - c, sizeof(buf));

		err = copy_from_user(buf, tx_buf + c, n);
		if (err)
			goto done;

		spi_gpio_tx(buf, n);
		c += n;
	}

	if (rx_buf)
		vfree(rx_buf);

	rx_buf = vmalloc(rx_num);
	if (!rx_buf)
		goto done;

	spi_gpio_rx(rx_buf, rx_num);
	rx_bufsz = rx_num;

done:
	spi_gpio_chip_select(0);

	return err;
}

/* Standalone API */
void sf_do_spi_xfer(u32 tx_num, const void *tx_buf, u32 rx_num,
			  void *rx_buf)
{
	struct gpio_sts_backup sts;

	preempt_disable();
	enter_spi_gpio_mode(&sts);
	spi_gpio_chip_select(1);
	spi_gpio_tx(tx_buf, tx_num);
	spi_gpio_rx(rx_buf, rx_num);
	spi_gpio_chip_select(0);
	leave_spi_gpio_mode(&sts);
	preempt_enable();
}
EXPORT_SYMBOL(sf_do_spi_xfer);

static int sf_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int sf_write(struct file *file, const char __user *buf, size_t size,
		    loff_t *ppos)
{
	struct sf_op op;
	struct gpio_sts_backup sts;
	long err;

	if (*ppos)
		return -EINVAL;

	if (size < sizeof(struct sf_op))
		return -EINVAL;

	err = copy_from_user(&op, buf, sizeof(struct sf_op));
	if (err)
		return err;

	pr_debug("tx_num = %u, rx_num = %u\n", op.tx_num, op.rx_num);

	if (size < sizeof(struct sf_op) + op.tx_num)
		return -EINVAL;

	mutex_lock(&lock);
	preempt_disable();
	enter_spi_gpio_mode(&sts);
	err = do_spi_xfer(op.tx_num, buf + sizeof(struct sf_op), op.rx_num);
	leave_spi_gpio_mode(&sts);
	preempt_enable();
	mutex_unlock(&lock);

	if (err) {
		pr_err("user data i/o failed\n");
		return err;
	}

	return size;
}

static ssize_t sf_read(struct file *file, char __user *buf, size_t size,
		       loff_t *ppos)
{
	long err;

	if (*ppos >= rx_bufsz)
		return -EINVAL;

	if ((*ppos + size) > rx_bufsz)
		size = rx_bufsz - *ppos;

	mutex_lock(&lock);
	err = copy_to_user(buf, rx_buf + *ppos, size);
	mutex_unlock(&lock);

	if (err)
		return err;

	*ppos += size;

	return size;
}

int sf_release(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations sf_fops =
{
	.open  = sf_open,
	.read = sf_read,
	.write = sf_write,
	.release = sf_release,
};

static int __init sf_init(void)
{
	mutex_init(&lock);

	gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);

	sf_entry = proc_create(SF_NAME, 0600, NULL, &sf_fops);

	if (!sf_entry)
		pr_err("failed to create proc entry " SF_NAME);

	return 0;
}

static void __exit sf_exit(void)
{
	if (sf_entry)
		proc_remove(sf_entry);

	if (gpio_base)
		iounmap(gpio_base);

	if (rx_buf)
		vfree(rx_buf);
}

module_init(sf_init);
module_exit(sf_exit);

MODULE_LICENSE("GPL");
