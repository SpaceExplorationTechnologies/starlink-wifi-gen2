/* SPDX-License-Identifier:	GPL-2.0+ */
/*
 * Copyright (C) 2021 MediaTek Incorporation. All Rights Reserved.
 *
 * Author: Alvin Kuo <Alvin.Kuo@mediatek.com>
 */

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/arm-smccc.h>
#include <net/genetlink.h>

#include "mtk_efuse.h"

/*
 *  define private netlink command type
 */
struct mtk_efuse_nl_cmd_t {
	enum MTK_EFUSE_NL_CMD cmd;
	int (*process)(struct genl_info *info);
	const enum MTK_EFUSE_NL_ATTR_TYPE *req_attrs;
	u32 req_attrs_num;
};

#define MTK_EFUSE_NL_CMD_REQ_ATTRS(attr) \
	.req_attrs = attr, \
	.req_attrs_num = ARRAY_SIZE(attr),

static const enum MTK_EFUSE_NL_ATTR_TYPE
mtk_efuse_nl_cmd_read_attrs[] = {
	MTK_EFUSE_NL_ATTR_TYPE_EFUSE_FIELD,
};

static const enum MTK_EFUSE_NL_ATTR_TYPE
mtk_efuse_nl_cmd_read_pubk_hash_attrs[] = {
	MTK_EFUSE_NL_ATTR_TYPE_EFUSE_FIELD,
};

static const enum MTK_EFUSE_NL_ATTR_TYPE
mtk_efuse_nl_cmd_write_attrs[] = {
	MTK_EFUSE_NL_ATTR_TYPE_EFUSE_FIELD,
	MTK_EFUSE_NL_ATTR_TYPE_EFUSE_VALUE,
};

static const enum MTK_EFUSE_NL_ATTR_TYPE
mtk_efuse_nl_cmd_write_pubk_hash_attrs[] = {
	MTK_EFUSE_NL_ATTR_TYPE_EFUSE_FIELD,
	MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH0,
	MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH1,
	MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH2,
	MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH3,
	MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH4,
	MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH5,
	MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH6,
	MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH7,
};

/*
 *  define process function and required attributes
 *  for each supported netlink command
 */
static const struct mtk_efuse_nl_cmd_t
mtk_efuse_nl_cmds[] = {
	{
		.cmd = MTK_EFUSE_NL_CMD_READ,
		.process = mtk_efuse_nl_reply_read,
		MTK_EFUSE_NL_CMD_REQ_ATTRS(mtk_efuse_nl_cmd_read_attrs)
	}, {
		.cmd = MTK_EFUSE_NL_CMD_READ_PUBK_HASH,
		.process = mtk_efuse_nl_reply_read_pubk_hash,
		MTK_EFUSE_NL_CMD_REQ_ATTRS(mtk_efuse_nl_cmd_read_pubk_hash_attrs)
	}, {
		.cmd = MTK_EFUSE_NL_CMD_WRITE,
		.process = mtk_efuse_nl_reply_write,
		MTK_EFUSE_NL_CMD_REQ_ATTRS(mtk_efuse_nl_cmd_write_attrs)
	}, {
		.cmd = MTK_EFUSE_NL_CMD_WRITE_PUBK_HASH,
		.process = mtk_efuse_nl_reply_write_pubk_hash,
		MTK_EFUSE_NL_CMD_REQ_ATTRS(mtk_efuse_nl_cmd_write_pubk_hash_attrs)
	}, {
		.cmd = MTK_EFUSE_NL_CMD_UPDATE_AR_VER,
		.process = mtk_efuse_nl_reply_update_ar_ver,
		.req_attrs = NULL,
		.req_attrs_num = 0,
	},
};

/*
 *  define netlink attribute type
 */
static const struct nla_policy
mtk_efuse_nl_cmd_policy[] = {
	[MTK_EFUSE_NL_ATTR_TYPE_EFUSE_FIELD] = { .type = NLA_U32 },
	[MTK_EFUSE_NL_ATTR_TYPE_EFUSE_VALUE] = { .type = NLA_U32 },
	[MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH0] = { .type = NLA_U32 },
	[MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH1] = { .type = NLA_U32 },
	[MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH2] = { .type = NLA_U32 },
	[MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH3] = { .type = NLA_U32 },
	[MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH4] = { .type = NLA_U32 },
	[MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH5] = { .type = NLA_U32 },
	[MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH6] = { .type = NLA_U32 },
	[MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH7] = { .type = NLA_U32 },
};

/*
 *  define supported netlink command
 */
static const struct genl_ops
mtk_efuse_nl_ops[] = {
	{
		.cmd = MTK_EFUSE_NL_CMD_READ,
		.doit = mtk_efuse_nl_response,
		.policy = mtk_efuse_nl_cmd_policy,
		.flags = GENL_ADMIN_PERM,
	}, {
		.cmd = MTK_EFUSE_NL_CMD_READ_PUBK_HASH,
		.doit = mtk_efuse_nl_response,
		.policy = mtk_efuse_nl_cmd_policy,
		.flags = GENL_ADMIN_PERM,
	}, {
		.cmd = MTK_EFUSE_NL_CMD_WRITE,
		.doit = mtk_efuse_nl_response,
		.policy = mtk_efuse_nl_cmd_policy,
		.flags = GENL_ADMIN_PERM,
	}, {
		.cmd = MTK_EFUSE_NL_CMD_WRITE_PUBK_HASH,
		.doit = mtk_efuse_nl_response,
		.policy = mtk_efuse_nl_cmd_policy,
		.flags = GENL_ADMIN_PERM,
	}, {
		.cmd = MTK_EFUSE_NL_CMD_UPDATE_AR_VER,
		.doit = mtk_efuse_nl_response,
		.policy = mtk_efuse_nl_cmd_policy,
		.flags = GENL_ADMIN_PERM,
	},
};

static struct genl_family
mtk_efuse_nl_family = {
	.id = GENL_ID_GENERATE,
	.name = MTK_EFUSE_NL_GENL_NAME,
	.version = MTK_EFUSE_NL_GENL_VERSION,
	.maxattr = MTK_EFUSE_NL_ATTR_TYPE_MAX,
};

static int mtk_efuse_smc(u32 smc_fid,
			 u32 x1,
			 u32 x2,
			 u32 x3,
			 u32 x4,
			 struct arm_smccc_res *res)
{
	/* SMC64 calling convention is used if in 64bits Linux */
	if (sizeof(void *) == 8)
		smc_fid |= (0x1 << 30);

	arm_smccc_smc(smc_fid, x1, x2, x3, x4, 0x0, 0x0, 0x0, res);

	return 0;
}

static int mtk_efuse_read(u32 efuse_field,
			  u8 *read_buffer,
			  u32 read_buffer_len)
{
	int ret;
	u32 idx;
	u32 offset;
	u32 efuse_len;
	u32 efuse_data[2] = { 0 };
	static struct arm_smccc_res res;

	/* get efuse length */
	ret = mtk_efuse_smc(MTK_SIP_EFUSE_GET_LEN,
			    efuse_field, 0x0, 0x0, 0x0,
			    &res);
	if (ret < 0)
		return ret;
	else if (res.a0 != MTK_EFUSE_SUCCESS) {
		pr_err("%s : get efuse length fail (%lu)\n",
		       __func__, res.a0);
		if (res.a0 == MTK_EFUSE_ERROR_EFUSE_FIELD_DISABLED)
			pr_err("%s : efuse field (%u) was disabled\n",
			       __func__, efuse_field);
		return -1;
	}
	efuse_len = res.a1;

	/* verify efuse_buffer */
	if (!read_buffer)
		return -EINVAL;
	if (read_buffer_len < efuse_len)
		return -ENOMEM;

	/* issue efuse read */
	ret = mtk_efuse_smc(MTK_SIP_EFUSE_READ,
			    efuse_field, 0x0, 0x0, 0x0,
			    &res);
	if (ret < 0)
		return ret;
	else if (res.a0 != MTK_EFUSE_SUCCESS) {
		pr_err("%s : read efuse fail (%lu)\n",
		       __func__, res.a0);
		return -1;
	}

	/* clean read buffer */
	memset(read_buffer, 0x0, read_buffer_len);

	/*
	 * get efuse data
	 * maximum data length in one time SMC is 8 bytes
	 */
	for (offset = 0; offset < efuse_len; offset += 8) {
		ret = mtk_efuse_smc(MTK_SIP_EFUSE_GET_DATA,
				    offset, 8, 0x0, 0x0,
				    &res);
		if (ret < 0)
			return ret;
		else if (res.a0 != MTK_EFUSE_SUCCESS) {
			pr_err("%s : get efuse data fail (%lu)\n",
			       __func__, res.a0);
			return -1;
		}
		efuse_data[0] = res.a2;
		efuse_data[1] = res.a3;

		for (idx = offset;
		     idx < (offset + 8) && idx < efuse_len;
		     idx++)
		{
			read_buffer[idx] = ((u8 *)efuse_data)[idx - offset];
		}
	}

	return 0;
}

static int mtk_efuse_write(u32 efuse_field,
			   u8 *write_buffer,
			   u32 write_buffer_len)
{
	int ret;
	u32 idx;
	u32 offset;
	u32 efuse_len;
	u32 efuse_data[2] = { 0 };
	static struct arm_smccc_res res;
	u32 read_buffer[MTK_EFUSE_PUBK_HASH_INDEX_MAX] = { 0 };

	/* get efuse length */
	ret = mtk_efuse_smc(MTK_SIP_EFUSE_GET_LEN,
			    efuse_field, 0x0, 0x0, 0x0,
			    &res);
	if (ret < 0)
		return ret;
	else if (res.a0 != MTK_EFUSE_SUCCESS) {
		pr_err("%s : get efuse length fail (%lu)\n",
		       __func__, res.a0);
		if (res.a0 == MTK_EFUSE_ERROR_EFUSE_FIELD_DISABLED)
			pr_err("%s : efuse field (%u) was disabled\n",
			       __func__, efuse_field);
		return -1;
	}
	efuse_len = res.a1;

	/* verify buffer */
	if (!write_buffer)
		return -EINVAL;
	if (write_buffer_len < efuse_len)
		return -ENOMEM;

	/*
	 * send efuse data
	 * maximum data length in one time SMC is 8 bytes
	 */
	for (offset = 0; offset < efuse_len; offset += 8) {
		memset(efuse_data, 0x0, sizeof(efuse_data));

		for (idx = offset;
		     idx < (offset + 8) && idx < efuse_len;
		     idx++) {
			((u8 *)efuse_data)[idx - offset] = write_buffer[idx];
		}

		ret = mtk_efuse_smc(MTK_SIP_EFUSE_SEND_DATA,
				    offset, idx - offset,
				    efuse_data[0], efuse_data[1],
				    &res);
		if (ret < 0)
			return ret;
		else if (res.a0 != MTK_EFUSE_SUCCESS) {
			pr_err("%s : send efuse data fail (%lu)\n",
			       __func__, res.a0);
			return -1;
		}
	}

	/*
	 * get efuse data
	 * maximum data length in one time SMC is 8 bytes
	 */
	for (offset = 0; offset < efuse_len; offset += 8) {
		memset(efuse_data, 0x0, sizeof(efuse_data));

		ret = mtk_efuse_smc(MTK_SIP_EFUSE_GET_DATA,
				    offset, 8, 0x0, 0x0,
				    &res);
		if (ret < 0)
			return ret;
		else if (res.a0 != MTK_EFUSE_SUCCESS) {
			pr_err("%s : get efuse data fail (%lu)\n",
			       __func__, res.a0);
			return -1;
		}
		efuse_data[0] = res.a2;
		efuse_data[1] = res.a3;

		for (idx = offset;
		     idx < (offset + 8) && idx < efuse_len;
		     idx++) {
			((u8 *)read_buffer)[idx] =
				((u8 *)efuse_data)[idx - offset];
		}
	}

	/* verify efuse data */
	if (memcmp(write_buffer, read_buffer, efuse_len)) {
		pr_err("%s : verify efuse data fail\n", __func__);
		return -1;
	}

	/* issue efuse write */
	ret = mtk_efuse_smc(MTK_SIP_EFUSE_WRITE,
			    efuse_field, 0x0, 0x0, 0x0,
			    &res);
	if (ret < 0)
		return ret;
	else if (res.a0 != MTK_EFUSE_SUCCESS) {
		pr_err("%s : write efuse fail (%lu)\n", __func__, res.a0);
		return -1;
	}

	return 0;
}

static int mtk_efuse_nl_prepare_reply(struct genl_info *info,
				      u8 cmd,
				      struct sk_buff **skbp)
{
	void *reply;
        struct sk_buff *skb;

        if (!info)
		return -EINVAL;

        skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
        if (!skb)
		return -ENOMEM;

	/* construct send-back message header */
	reply = genlmsg_put(skb, info->snd_portid, info->snd_seq,
			    &mtk_efuse_nl_family, 0, cmd);
	if (!reply) {
		nlmsg_free(skb);
		return -EINVAL;
	}

	*skbp = skb;
        return 0;
}

static int mtk_efuse_nl_send_reply(struct sk_buff *skb,
				   struct genl_info *info)
{
        struct genlmsghdr *genlhdr = nlmsg_data(nlmsg_hdr(skb));
        void *reply = genlmsg_data(genlhdr);

        /* finalize a generic netlink message */
        genlmsg_end(skb, reply);

        /* reply to a request */
        return genlmsg_reply(skb, info);
}

static int mtk_efuse_nl_reply_read(struct genl_info *info)
{
	int ret;
	u32 efuse_field;
	u32 read_buffer = 0;
	struct sk_buff *reply_skb = NULL;

	efuse_field =
		nla_get_u32(info->attrs[MTK_EFUSE_NL_ATTR_TYPE_EFUSE_FIELD]);
	if (efuse_field >= MTK_EFUSE_FIELD_MAX ||
	    efuse_field < MTK_EFUSE_FIELD_SBC_PUBK0_HASH_LOCK)
		return -EINVAL;

	ret = mtk_efuse_nl_prepare_reply(info, MTK_EFUSE_NL_CMD_READ,
					 &reply_skb);
	if (ret < 0)
		goto error;

	ret = mtk_efuse_read(efuse_field, (u8 *)&read_buffer,
			     sizeof(read_buffer));
	if (ret < 0)
		goto error;

	ret = nla_put_u32(reply_skb, MTK_EFUSE_NL_ATTR_TYPE_EFUSE_VALUE,
			  read_buffer);
	if (ret < 0)
		goto error;

        return mtk_efuse_nl_send_reply(reply_skb, info);

error:
        if (reply_skb)
		nlmsg_free(reply_skb);

        return ret;
}

static int mtk_efuse_nl_reply_read_pubk_hash(struct genl_info *info)
{
	int ret;
	u32 idx;
	u32 efuse_field;
	struct sk_buff *reply_skb = NULL;
	u32 read_buffer[MTK_EFUSE_PUBK_HASH_INDEX_MAX] = { 0 };

	efuse_field =
		nla_get_u32(info->attrs[MTK_EFUSE_NL_ATTR_TYPE_EFUSE_FIELD]);
	if (efuse_field > MTK_EFUSE_FIELD_SBC_PUBK3_HASH)
		return -EINVAL;

	ret = mtk_efuse_nl_prepare_reply(info, MTK_EFUSE_NL_CMD_READ_PUBK_HASH,
					 &reply_skb);
	if (ret < 0)
		goto error;

	ret = mtk_efuse_read(efuse_field, (u8 *)&read_buffer[0],
			     sizeof(read_buffer));
	if (ret < 0)
		goto error;

	for (idx = 0; idx < MTK_EFUSE_PUBK_HASH_INDEX_MAX; idx++) {
		ret = nla_put_u32(reply_skb,
				  MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH0 + idx,
				  read_buffer[idx]);
		if (ret < 0)
			goto error;
	}

        return mtk_efuse_nl_send_reply(reply_skb, info);

error:
        if (reply_skb)
		nlmsg_free(reply_skb);

        return ret;
}

static int mtk_efuse_nl_reply_write(struct genl_info *info)
{
	int ret;
	u32 efuse_field;
	u32 write_buffer;

	efuse_field =
		nla_get_u32(info->attrs[MTK_EFUSE_NL_ATTR_TYPE_EFUSE_FIELD]);
	if (efuse_field >= MTK_EFUSE_FIELD_MAX ||
	    efuse_field < MTK_EFUSE_FIELD_SBC_PUBK0_HASH_LOCK)
		return -EINVAL;

	write_buffer =
		nla_get_u32(info->attrs[MTK_EFUSE_NL_ATTR_TYPE_EFUSE_VALUE]);

	ret = mtk_efuse_write(efuse_field, (u8 *)&write_buffer,
			      sizeof(write_buffer));

	return ret;
}

static int mtk_efuse_nl_reply_write_pubk_hash(struct genl_info *info)
{
	int ret;
	u32 idx;
	u32 efuse_field;
	u32 write_buffer[MTK_EFUSE_PUBK_HASH_INDEX_MAX] = { 0 };

	efuse_field =
		nla_get_u32(info->attrs[MTK_EFUSE_NL_ATTR_TYPE_EFUSE_FIELD]);
	if (efuse_field > MTK_EFUSE_FIELD_SBC_PUBK3_HASH)
		return -EINVAL;

	for (idx = 0; idx < MTK_EFUSE_PUBK_HASH_INDEX_MAX; idx++) {
		write_buffer[idx] =
			nla_get_u32(
			info->attrs[MTK_EFUSE_NL_ATTR_TYPE_PUBK_HASH0 + idx]);
	}

	ret = mtk_efuse_write(efuse_field, (u8 *)&write_buffer[0],
			      sizeof(write_buffer));

	return ret;
}

static int mtk_efuse_nl_reply_update_ar_ver(struct genl_info *info)
{
	int ret;
	struct arm_smccc_res res;

	/* issue update anti-rollback version*/
	ret = mtk_efuse_smc(MTK_SIP_EFUSE_UPDATE_AR_VER,
			    0x0, 0x0, 0x0, 0x0,
			    &res);
	if (ret < 0)
		return ret;
	else if (res.a0 != MTK_EFUSE_SUCCESS) {
		pr_err("%s : update anti-rollback version fail (%lu)\n",
		       __func__, res.a0);
		return -1;
	}

	return 0;
}

static int mtk_efuse_nl_response(struct sk_buff *skb, struct genl_info *info)
{
	int idx;
        u32 nl_cmd_attrs_num = 0;
        const struct mtk_efuse_nl_cmd_t *cmditem = NULL;
        struct genlmsghdr *hdr = nlmsg_data(info->nlhdr);

	for (idx = 0; idx < ARRAY_SIZE(mtk_efuse_nl_cmds); idx++) {
		if (hdr->cmd == mtk_efuse_nl_cmds[idx].cmd) {
			cmditem = &mtk_efuse_nl_cmds[idx];
			break;
		}
	}

	if (!cmditem) {
		pr_err("%s : unknown cmd %u\n", __func__, hdr->cmd);
		return -EINVAL;
	}

	for (idx = 0; idx < cmditem->req_attrs_num; idx++) {
		if (info->attrs[cmditem->req_attrs[idx]])
			nl_cmd_attrs_num++;
	}

	if (nl_cmd_attrs_num != cmditem->req_attrs_num) {
		pr_err("%s : missing required attr(s) for cmd %u\n",
		       __func__, hdr->cmd);
		return -EINVAL;
	}

	return cmditem->process(info);
}

static int __init mtk_efuse_nl_init(void)
{
	int ret;

	ret = genl_register_family_with_ops(&mtk_efuse_nl_family,
					    mtk_efuse_nl_ops);
	if (ret) {
		pr_err("%s : genl_register_family_with_ops fail (%d)\n",
		       __func__, ret);
		return ret;
	}

	pr_info("%s : netlink family \"mtk-efuse\" register success\n",
		__func__);
        return 0;
}

static void __exit mtk_efuse_nl_exit(void)
{
	genl_unregister_family(&mtk_efuse_nl_family);

	pr_info("%s : netlink family \"mtk-efuse\" unregister success\n",
		__func__);
}

module_init(mtk_efuse_nl_init);
module_exit(mtk_efuse_nl_exit);

MODULE_DESCRIPTION("Mediatek eFuse driver");
MODULE_LICENSE("GPL");
