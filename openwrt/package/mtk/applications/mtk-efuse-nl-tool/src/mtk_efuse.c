/* SPDX-License-Identifier:	GPL-2.0+ */
/*
 * Copyright (C) 2021 MediaTek Incorporation. All Rights Reserved.
 *
 * Author: Alvin Kuo <Alvin.Kuo@mediatek.com>
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>

#include "mtk_efuse.h"

static struct nl_sock *sock;
static struct nl_cache *cache;
static struct genl_family *family;

static void print_usage(void)
{
	printf("Usage:\n");
	printf(" mtk-efuse-tool [CMD] [HASH_NUMBER] [PKEY_HASH_FILE]\n");
	printf(" [CMD] are:\n");
	printf("    wh       write hash [PKEY_HASH_FILE] into the one\n");
	printf("             [HASH_NUMBER] of four efuse hash fields\n");
	printf("    rh       read hash from one [HASH_NUMBER] of four\n");
	printf("             efuse hash fields\n");
	printf("    lh       lock one [HASH_NUMBER] of four efuse hash\n");
	printf("             fields\n");
	printf("    cl       check if lock one [HASH_NUMBER] of four efuse\n");
	printf("             hash fields\n");
	printf("    dh       disable one [HASH_NUMBER] of four efuse hash\n");
	printf("             fields\n");
	printf("    cd       check if disable one [HASH_NUMBER] of four\n");
	printf("             efuse hash fields\n");
	printf("    es       enable secure boot of BROM\n");
	printf("    cs       check if enable secure boot of BROM\n");
	printf("    dj       disable JTAG\n");
	printf("    cj       check if disable JTAG\n");
	printf("    db       disable BROM CMD\n");
	printf("    cb       check if disable BROM CMD\n");
	printf("    ea       enable anti-rollback of BROM\n");
	printf("    ca       check if enable anti-rollback of BROM\n");
	printf("    ua       update eFuse anti-rollback version\n");
	printf(" [HASH_NUMBER] are:\n");
	printf("    0        PUBK0_HASH\n");
	printf("    1        PUBK1_HASH\n");
	printf("    2        PUBK2_HASH\n");
	printf("    3        PUBK3_HASH\n");
	printf(" Example:\n");
	printf("  @For secure boot:\n");
	printf("    mtk-efuse-tool wh 0 bl2.img.signkeyhash\n");
	printf("    mtk-efuse-tool rh 0\n");
	printf("    mtk-efuse-tool lh 0\n");
	printf("    mtk-efuse-tool es\n");
	printf("    mtk-efuse-tool dj\n");
	printf("    mtk-efuse-tool db\n");
	printf("  @For anti-rollback:\n");
	printf("    mtk-efuse-tool ea\n");
	printf("    mtk-efuse-tool ua\n");
}

static int verify_parameters(int argc,
			     char *argv[])
{
	char *cmd;
	uint32_t pubk_hash_num;

	if (argc < 2) {
		fprintf(stderr, "missing cmd\n");
		return -EINVAL;
	}

	cmd = argv[1];
	if (!strncmp(cmd, "wh", 2)) {
		if (argc < 4) {
			fprintf(stderr, "too few parameters\n");
			return -EINVAL;
		}

		if (!isdigit(*argv[2])) {
			fprintf(stderr, "hash number must be a numeric\n");
			return -EINVAL;
		}

		pubk_hash_num = strtoul(argv[2], NULL, 10);
		if (pubk_hash_num > 3) {
			fprintf(stderr, "hash number out of range\n");
			return -EINVAL;
		}
	} else if (!strncmp(cmd, "rh", 2) ||
		   !strncmp(cmd, "lh", 2) ||
		   !strncmp(cmd, "cl", 2) ||
		   !strncmp(cmd, "dh", 2) ||
		   !strncmp(cmd, "cd", 2) ) {
		if (argc < 3) {
			fprintf(stderr, "too few parameters\n");
			return -EINVAL;
		}

		if (!isdigit(*argv[2])) {
			fprintf(stderr, "hash number must be a numeric\n");
			return -EINVAL;
		}

		pubk_hash_num = strtoul(argv[2], NULL, 10);
		if (pubk_hash_num > 3) {
			fprintf(stderr, "hash number out of range\n");
			return -EINVAL;
		}
	}

	return 0;
}

static int mtk_efuse_read_pubk(char *file,
			       uint8_t *buffer,
			       uint32_t buffer_size)
{
	FILE *fp;
	long len;

	/* verify parameters */
	if (!file || !buffer || buffer_size < MTK_EFUSE_FIELD_SBC_PUBK_HASH_LEN)
		return -EINVAL;

	/* open file */
	fp = fopen(file, "rb");
	if (!fp) {
		fprintf(stderr, "unable to open file\n");
		return -1;
	}

	/* get file length */
	if (fseek(fp, 0, SEEK_END)) {
		fprintf(stderr, "unable to seek to end of file\n");
		return -ferror(fp);
	}
	len = ftell(fp);
	if (len < 0) {
		fprintf(stderr, "unable to get the size of file\n");
		return -ferror(fp);
	}
	rewind(fp);

	/* check file length */
	if (len < MTK_EFUSE_FIELD_SBC_PUBK_HASH_LEN) {
		fprintf(stderr, "file size less than public key hash length\n");
		return -1;
	}

	/* read public key hash */
	memset(buffer, 0x0, buffer_size);
	len = fread(buffer, 1, MTK_EFUSE_FIELD_SBC_PUBK_HASH_LEN, fp);
	if (len != MTK_EFUSE_FIELD_SBC_PUBK_HASH_LEN)
		return -ferror(fp);

	return 0;
}

static void mtk_efuse_nl_free(void)
{
	if (family)
		nl_object_put((struct nl_object *)family);
	if (cache)
		nl_cache_free(cache);
	if (sock)
		nl_socket_free(sock);

	sock = NULL;
	cache = NULL;
	family = NULL;
}

int mtk_efuse_nl_init(void)
{
	int ret;

	sock = NULL;
	cache = NULL;
	family = NULL;

	/* allocate a new netlink socket */
	sock = nl_socket_alloc();
	if (!sock) {
		fprintf(stderr, "failed to create socket\n");
		goto error;
	}

	/* connect a generic netlink socket*/
	ret = genl_connect(sock);
	if (ret <0) {
		fprintf(stderr, "failed to connect a generic netlink socket\n");
		goto error;
	}

	/* allocate a new controller cache */
	ret = genl_ctrl_alloc_cache(sock, &cache);
	if (ret < 0) {
		fprintf(stderr, "failed to allocate a controller cache\n");
		goto error;
	}

	/* search controller cache for a family name match */
	family = genl_ctrl_search_by_name(cache, MTK_EFUSE_NL_GENL_NAME);
	if (!family) {
		fprintf(stderr,"netlink family (%s) not found\n",
			MTK_EFUSE_NL_GENL_NAME);
		goto error;
	}

	return 0;

error:
	mtk_efuse_nl_free();
	return -EINVAL;
}

static int construct_attrs(struct nl_msg *msg,
			   void *arg)
{
	uint32_t idx;
        struct mtk_efuse_data_t *data = arg;
	struct genlmsghdr *genlh = nlmsg_data(nlmsg_hdr(msg));

	/* put efuse field into netlink msg */
	nla_put_u32(msg,
		    MTK_EFUSE_NL_ATTR_TYPE_EFUSE_FIELD,
		    data->efuse_field);

	if (genlh->cmd == MTK_EFUSE_NL_CMD_WRITE) {
		/* put value into netlink msg */
		nla_put_u32(msg,
			    MTK_EFUSE_NL_ATTR_TYPE_EFUSE_VALUE,
			    data->efuse_value);
	}

	if (genlh->cmd == MTK_EFUSE_NL_CMD_WRITE_PUBK_HASH) {
		/* put public key hash into netlink msg */
		for (idx = 0; idx < MTK_EFUSE_PUBK_HASH_INDEX_MAX; idx++) {
			nla_put_u32(msg,
				    MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH0 + idx,
				    data->pubk_hash[idx]);
		}
	}

        return 0;
}

static int spilt_attrs(struct nl_msg *msg,
		       void *arg)
{
	int ret;
	uint32_t idx;
        struct mtk_efuse_data_t *data = arg;
	struct genlmsghdr *genlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *nl_attrs[MTK_EFUSE_NL_ATTR_TYPE_MAX + 1];

	/* parse netlink msg */
	ret = nla_parse(nl_attrs, MTK_EFUSE_NL_ATTR_TYPE_MAX,
			genlmsg_attrdata(genlh, 0), genlmsg_attrlen(genlh, 0),
			NULL);
	if (ret < 0) {
		fprintf(stderr,"netlink msg parse fail (%d)\n", ret);
		goto done;
	}

	if (genlh->cmd == MTK_EFUSE_NL_CMD_READ) {
		/* get value */
		data->efuse_value =
			nla_get_u32(nl_attrs[MTK_EFUSE_NL_ATTR_TYPE_EFUSE_VALUE]);
	}

	if (genlh->cmd == MTK_EFUSE_NL_CMD_READ_PUBK_HASH) {
		/* get public key hash */
		for (idx =0; idx < MTK_EFUSE_PUBK_HASH_INDEX_MAX; idx++) {
			data->pubk_hash[idx] =
				nla_get_u32(nl_attrs[MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH0 + idx]);
		}
	}

	return 0;

done:
	return NL_SKIP;
}

static int wait_handler(struct nl_msg *msg,
			void *arg)
{
        int *finished = arg;

        *finished = 1;
        return NL_STOP;
}

static int mtk_efuse_nl_request(int cmd,
				int (*spilt)(struct nl_msg *, void *),
				int (*construct)(struct nl_msg *, void *),
				void *arg)
{
        int ret = -1;
        int finished;
        int flags = 0;
        struct nl_msg *msg;
        struct nl_cb *callback = NULL;

        /* allocate an netllink message buffer */
	msg = nlmsg_alloc();
	if (!msg) {
		fprintf(stderr, "failed to allocate netlink message\n");
		return -ENOMEM;
	}

	/* add generic netlink headers to netlink message */
	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ,
		    genl_family_get_id(family), 0, flags, cmd, 0);

	/* fill attaribute of netlink message via construct function */
	if (construct) {
		ret = construct(msg, arg);
		if (ret < 0) {
			fprintf(stderr, "construct attribute fail\n");
			goto error;
		}
	}

	/* allocate a new callback handler */
	callback = nl_cb_alloc(NL_CB_CUSTOM);
	if (!callback) {
		fprintf(stderr, "failed to allocate callback handler\n");
		goto error;
	}

	/* send netlink message */
	ret = nl_send_auto_complete(sock, msg);
	if (ret < 0) {
		fprintf(stderr, "send netlink msg fail (%d)\n", ret);
		goto error;
	}

	finished = 0;
	if (spilt) {
		nl_cb_set(callback, NL_CB_VALID, NL_CB_CUSTOM,
			  spilt, arg);
		nl_cb_set(callback, NL_CB_FINISH, NL_CB_CUSTOM,
			  wait_handler, &finished);
	} else {
		nl_cb_set(callback, NL_CB_ACK, NL_CB_CUSTOM,
			  wait_handler, &finished);
	}

	/* receive message from kernel request */
	ret = nl_recvmsgs(sock, callback);
	if (ret < 0) {
		fprintf(stderr, "receive msg fail (%d)\n", ret);
		goto error;
	}

	/* wait until an ACK is received for the latest not yet acknowledge*/
	if (!finished)
		ret = nl_wait_for_ack(sock);
		if (ret < 0) {
			fprintf(stderr, "wait ack fail (%d)\n", ret);
			goto error;
		}

error:
	if (callback)
		nl_cb_put(callback);

	if (msg)
		nlmsg_free(msg);

        return ret;
}

int main(int argc, char *argv[])
{
	char *cmd;
	int ret = 0;
	uint32_t idx;
	char *pubk_hash_file;
	uint32_t pubk_hash_num;
	struct mtk_efuse_data_t data = { 0 };

	ret = verify_parameters(argc, argv);
	if (ret) {
		print_usage();
		goto error;
	}

	ret = mtk_efuse_nl_init();
	if (ret) {
		fprintf(stderr,
			"netlink init fail(%d)\n", ret);
		goto error;
	}

	cmd = argv[1];
	if (!strncmp(cmd, "wh", 2)) {
		/* get efuse field of public key hash */
		pubk_hash_num = strtoul(argv[2], NULL, 10);
		data.efuse_field =
			MTK_EFUSE_FIELD_SBC_PUBK0_HASH + pubk_hash_num;

		/* get public key hash */
		pubk_hash_file = argv[3];
		ret = mtk_efuse_read_pubk(pubk_hash_file,
					  (uint8_t *)data.pubk_hash,
					  sizeof(data.pubk_hash));
		if (ret) {
			fprintf(stderr,
				"read public key from \"%s\" fail (%d)\n",
				pubk_hash_file, ret);
			goto error;
		}

		/* send netlink request */
		ret = mtk_efuse_nl_request(MTK_EFUSE_NL_CMD_WRITE_PUBK_HASH,
					   NULL,
					   construct_attrs,
					   &data);
		if (ret) {
			fprintf(stderr,
				"netlink request fail (%d) for '%s'\n",
				ret, cmd);
			goto error;
		}
	} else if (!strncmp(cmd, "rh", 2)) {
		/* get efuse field of public key hash */
		pubk_hash_num = strtoul(argv[2], NULL, 10);
		data.efuse_field =
			MTK_EFUSE_FIELD_SBC_PUBK0_HASH + pubk_hash_num;

		/* send netlink request */
		ret = mtk_efuse_nl_request(MTK_EFUSE_NL_CMD_READ_PUBK_HASH,
					   spilt_attrs,
					   construct_attrs,
					   &data);
		if (ret) {
			fprintf(stderr,
				"netlink request fail (%d) for '%s'\n",
				ret, cmd);
			goto error;
		}

		/* print public key hash */
		printf("PUBK%d_HASH :", pubk_hash_num);
		for (idx = 0; idx < MTK_EFUSE_FIELD_SBC_PUBK_HASH_LEN; idx++) {
			if (idx % 16 == 0)
				printf("\n%08x ", idx);
			if (idx % 8 == 0)
				printf(" ");
			printf("%02x ", ((uint8_t *)(data.pubk_hash))[idx]);
		}
		printf("\n");
	} else if (!strncmp(cmd, "lh", 2)) {
		/* get efuse field of public key lock */
		pubk_hash_num = strtoul(argv[2], NULL, 10);
		data.efuse_field =
			MTK_EFUSE_FIELD_SBC_PUBK0_HASH_LOCK + pubk_hash_num;

		/* fill in efuse value */
		data.efuse_value = 0x00000001;

		/* send netlink request */
		ret = mtk_efuse_nl_request(MTK_EFUSE_NL_CMD_WRITE,
					   NULL,
					   construct_attrs,
					   &data);
		if (ret) {
			fprintf(stderr,
				"netlink request fail (%d) for '%s'\n",
				ret, cmd);
			goto error;
		}
	} else if (!strncmp(cmd, "cl", 2)) {
		/* get efuse field of public key lock */
		pubk_hash_num = strtoul(argv[2], NULL, 10);
		data.efuse_field =
			MTK_EFUSE_FIELD_SBC_PUBK0_HASH_LOCK + pubk_hash_num;

		/* send netlink request */
		ret = mtk_efuse_nl_request(MTK_EFUSE_NL_CMD_READ,
					   spilt_attrs,
					   construct_attrs,
					   &data);
		if (ret) {
			fprintf(stderr,
				"netlink request fail (%d) for '%s'\n",
				ret, cmd);
			goto error;
		}

		/* print result */
		printf("PUBK%d_HASH : ", pubk_hash_num);
		if (data.efuse_value == 0x00000001)
			printf("lock\n");
		else
			printf("unlock\n");
	} else if (!strncmp(cmd, "dh", 2)) {
		/* get efuse field of public key disable */
		pubk_hash_num = strtoul(argv[2], NULL, 10);
		data.efuse_field =
			MTK_EFUSE_FIELD_SBC_PUBK0_HASH_DIS + pubk_hash_num;

		/* fill in efuse value */
		data.efuse_value = 0x00000001;

		/* send netlink request */
		ret = mtk_efuse_nl_request(MTK_EFUSE_NL_CMD_WRITE,
					   NULL,
					   construct_attrs,
					   &data);
		if (ret) {
			fprintf(stderr,
				"netlink request fail (%d) for '%s'\n",
				ret, cmd);
			goto error;
		}
	} else if (!strncmp(cmd, "cd", 2)) {
		/* get efuse field of public key disable */
		pubk_hash_num = strtoul(argv[2], NULL, 10);
		data.efuse_field =
			MTK_EFUSE_FIELD_SBC_PUBK0_HASH_DIS + pubk_hash_num;

		/* send netlink request */
		ret = mtk_efuse_nl_request(MTK_EFUSE_NL_CMD_READ,
					   spilt_attrs,
					   construct_attrs,
					   &data);
		if (ret) {
			fprintf(stderr,
				"netlink request fail (%d) for '%s'\n",
				ret, cmd);
			goto error;
		}

		/* print result */
		printf("PUBK%d_HASH : ", pubk_hash_num);
		if (data.efuse_value == 0x00000001)
			printf("disable\n");
		else
			printf("enable\n");
	} else if (!strncmp(cmd, "es", 2)) {
		/* fill in efuse field */
		data.efuse_field = MTK_EFUSE_FIELD_SBC_EN;

		/* fill in efuse value */
		data.efuse_value = 0x00000001;

		/* send netlink request */
		ret = mtk_efuse_nl_request(MTK_EFUSE_NL_CMD_WRITE,
					   NULL,
					   construct_attrs,
					   &data);
		if (ret) {
			fprintf(stderr,
				"netlink request fail (%d) for '%s'\n",
				ret, cmd);
			goto error;
		}
	} else if (!strncmp(cmd, "cs", 2)) {
		/* fill in efuse field */
		data.efuse_field = MTK_EFUSE_FIELD_SBC_EN;

		/* send netlink request */
		ret = mtk_efuse_nl_request(MTK_EFUSE_NL_CMD_READ,
					   spilt_attrs,
					   construct_attrs,
					   &data);
		if (ret) {
			fprintf(stderr,
				"netlink request fail (%d) for '%s'\n",
				ret, cmd);
			goto error;
		}

		/* print result */
		printf("Secure Boot : ");
		if (data.efuse_value == 0x00000001)
			printf("enable\n");
		else
			printf("disable\n");
	} else if (!strncmp(cmd, "dj", 2)) {
		/* fill in efuse field */
		data.efuse_field = MTK_EFUSE_FIELD_JTAG_DIS;

		/* fill in efuse value */
		data.efuse_value = 0x00000001;

		/* send netlink request */
		ret = mtk_efuse_nl_request(MTK_EFUSE_NL_CMD_WRITE,
					   NULL,
					   construct_attrs,
					   &data);
		if (ret) {
			fprintf(stderr,
				"netlink request fail (%d) for '%s'\n",
				ret, cmd);
			goto error;
		}
	} else if (!strncmp(cmd, "cj", 2)) {
		/* fill in efuse field */
		data.efuse_field = MTK_EFUSE_FIELD_JTAG_DIS;

		/* send netlink request */
		ret = mtk_efuse_nl_request(MTK_EFUSE_NL_CMD_READ,
					   spilt_attrs,
					   construct_attrs,
					   &data);
		if (ret) {
			fprintf(stderr,
				"netlink request fail (%d) for '%s'\n",
				ret, cmd);
			goto error;
		}

		/* print result */
		printf("JTAG : ");
		if (data.efuse_value == 0x00000001)
			printf("disable\n");
		else
			printf("enable\n");
	} else if (!strncmp(cmd, "db", 2)) {
		/* fill in efuse field */
		data.efuse_field = MTK_EFUSE_FIELD_BROM_CMD_DIS;

		/* fill in efuse value */
		data.efuse_value = 0x00000001;

		/* send netlink request */
		ret = mtk_efuse_nl_request(MTK_EFUSE_NL_CMD_WRITE,
					   NULL,
					   construct_attrs,
					   &data);
		if (ret) {
			fprintf(stderr,
				"netlink request fail (%d) for '%s'\n",
				ret, cmd);
			goto error;
		}
	} else if (!strncmp(cmd, "cb", 2)) {
		/* fill in efuse field */
		data.efuse_field = MTK_EFUSE_FIELD_BROM_CMD_DIS;

		/* send netlink request */
		ret = mtk_efuse_nl_request(MTK_EFUSE_NL_CMD_READ,
					   spilt_attrs,
					   construct_attrs,
					   &data);
		if (ret) {
			fprintf(stderr,
				"netlink request fail (%d) for '%s'\n",
				ret, cmd);
			goto error;
		}

		/* print result */
		printf("BROM CMD : ");
		if (data.efuse_value == 0x00000001)
			printf("disable\n");
		else
			printf("enable\n");
	} else if (!strncmp(cmd, "ea", 2)) {
		/* fill in efuse field */
		data.efuse_field = MTK_EFUSE_FIELD_AR_EN;

		/* fill in efuse value */
		data.efuse_value = 0x00000001;

		/* send netlink request */
		ret = mtk_efuse_nl_request(MTK_EFUSE_NL_CMD_WRITE,
					   NULL,
					   construct_attrs,
					   &data);
		if (ret) {
			fprintf(stderr,
				"netlink request fail (%d) for '%s'\n",
				ret, cmd);
			goto error;
		}
	} else if (!strncmp(cmd, "ca", 2)) {
		/* fill in efuse field */
		data.efuse_field = MTK_EFUSE_FIELD_AR_EN;

		/* send netlink request */
		ret = mtk_efuse_nl_request(MTK_EFUSE_NL_CMD_READ,
					   spilt_attrs,
					   construct_attrs,
					   &data);
		if (ret) {
			fprintf(stderr,
				"netlink request fail (%d) for '%s'\n",
				ret, cmd);
			goto error;
		}

		/* print result */
		printf("Anti-Rollback : ");
		if (data.efuse_value == 0x00000001)
			printf("enable\n");
		else
			printf("disable\n");
	} else if (!strncmp(cmd, "ua", 2)) {
		/* send netlink request */
		ret = mtk_efuse_nl_request(MTK_EFUSE_NL_CMD_UPDATE_AR_VER,
					   NULL,
					   NULL,
					   NULL);
		if (ret) {
			fprintf(stderr,
				"netlink request fail (%d) for '%s'\n",
				ret, cmd);
			goto error;
		}
	} else {
		fprintf(stderr, "unsupported cmd '%s'\n", cmd);
		goto error;
	}

	printf("\nefuse operate (%s) success\n", cmd);

error :
	mtk_efuse_nl_free();
	return ret;
}
