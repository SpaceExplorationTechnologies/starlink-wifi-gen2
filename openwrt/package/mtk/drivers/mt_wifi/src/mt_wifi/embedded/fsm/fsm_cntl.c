/***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2012, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	Common mgmt cntl

*/
#include "rt_config.h"


const CHAR *CNTL_FSM_STATE_STR[MAX_CNTL_STATE] = {
	"CNTL_IDLE",
	"CNTL_WAIT_SYNC",
	"CNTL_WAIT_AUTH",
	"CNTL_WAIT_AUTH2",
	"CNTL_WAIT_DEAUTH",
	"CNTL_WAIT_ASSOC",
	"CNTL_WAIT_DISASSOC",
};

const CHAR *CNTL_FSM_MSG_STR[MAX_CNTL_MSG] = {
	"CNTL_MACHINE_BASE/CNTL_MLME_CONNECT",
	"CNTL_MLME_JOIN_CONF",
	"CNTL_MLME_AUTH_CONF",
	"CNTL_MLME_ASSOC_CONF",
	"CNTL_MLME_REASSOC_CONF",
	"CNTL_MLME_DISCONNECT",
	"CNTL_MLME_DEAUTH_CONF",
	"CNTL_MLME_DISASSOC_CONF",
	"CNTL_MLME_SCAN",
	"CNTL_MLME_SCAN_CONF",
	"CNTL_MLME_SCAN_FOR_CONN",
	"CNTL_MLME_FAIL",
	"CNTL_MLME_RESET_TO_IDLE",
#ifdef APCLI_CONNECTION_TRIAL
	"APCLI_CTRL_TRIAL_CONNECT",
	"APCLI_CTRL_TRIAL_CONNECT_TIMEOUT",
	"APCLI_CTRL_TRIAL_PHASE2_TIMEOUT",
	"APCLI_CTRL_TRIAL_RETRY_TIMEOUT",
#endif
};
/*TRIAL CONNECTION */



#ifdef APCLI_CONNECTION_TRIAL

static VOID ApCliCtrlTrialConnectAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliTrialConnectTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliTrialPhase2TimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliTrialConnectRetryTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);
static VOID ApCliTrialConnectTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

static VOID ApCliTrialConnectPhase2Timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

static VOID ApCliTrialConnectRetryTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

DECLARE_TIMER_FUNCTION(ApCliTrialConnectTimeout);
BUILD_TIMER_FUNCTION(ApCliTrialConnectTimeout);
DECLARE_TIMER_FUNCTION(ApCliTrialConnectPhase2Timeout);
BUILD_TIMER_FUNCTION(ApCliTrialConnectPhase2Timeout);
DECLARE_TIMER_FUNCTION(ApCliTrialConnectRetryTimeout);
BUILD_TIMER_FUNCTION(ApCliTrialConnectRetryTimeout);
UINT GetBandByChannel(UCHAR ch, RTMP_ADAPTER *pAd);
#endif /*ApCli Trial Connection*/



inline BOOLEAN cntl_fsm_state_transition(struct wifi_dev *wdev, UCHAR cli_idx, ULONG next_state, const char *caller)
{
	ULONG old_state = 0;
#ifdef MAC_REPEATER_SUPPORT

	if (cli_idx != NON_REPT_ENTRY) {
		PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)wdev->sys_handle;
		REPEATER_CLIENT_ENTRY *pReptEntry = &pAd->ApCfg.pRepeaterCliPool[cli_idx];

		old_state = pReptEntry->CtrlCurrState;
		pReptEntry->CtrlCurrState = next_state;
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		old_state = wdev->cntl_machine.CurrState;
		wdev->cntl_machine.CurrState = next_state;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			 ("CNTL [%s, TYPE:%d %s]: [%s] \t==============================================> [%s] (by %s)\n",
			  wdev->if_dev->name, wdev->wdev_type, (cli_idx != NON_REPT_ENTRY) ? "(REPT)" : "",
			  CNTL_FSM_STATE_STR[old_state],
			  CNTL_FSM_STATE_STR[next_state],
			  caller));
	return TRUE;
}

static VOID cntl_mlme_connect(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_connect_proc)
		cntl_api->cntl_connect_proc(wdev, Elem->Msg, Elem->MsgLen);
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_connect_proc hook api.\n",
				  __func__));
	}
}

static VOID cntl_mlme_disconnect(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_disconnect_proc)
		cntl_api->cntl_disconnect_proc(Elem);
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_disconnect_proc hook api.\n",
				  __func__));
	}
}

static VOID cntl_mlme_scan(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_scan_proc)
		cntl_api->cntl_scan_proc(Elem);
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_scan_proc hook api.\n",
				  __func__));
	}
}

static VOID cntl_mlme_scan_conf(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_scan_conf)
		cntl_api->cntl_scan_conf(Elem);
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_scan_conf hook api.\n",
				  __func__));
	}
}

static VOID cntl_mlme_join_conf(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_join_conf)
		cntl_api->cntl_join_conf(Elem);
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_join_conf hook api.\n",
				  __func__));
	}
}

static VOID cntl_mlme_auth_conf(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_auth_conf)
		cntl_api->cntl_auth_conf(Elem);
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_auth_conf hook api.\n",
				  __func__));
	}
}

static VOID cntl_mlme_auth2_conf(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_auth2_conf)
		cntl_api->cntl_auth2_conf(Elem);
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_auth_conf2 hook api.\n",
				  __func__));
	}
}

static VOID cntl_mlme_deauth_conf(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_deauth_conf)
		cntl_api->cntl_deauth_conf(Elem);
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_deauth_conf hook api.\n",
				  __func__));
	}
}

static VOID cntl_mlme_assoc_conf(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_assoc_conf)
		cntl_api->cntl_assoc_conf(Elem);
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_assoc_conf hook api.\n",
				  __func__));
	}
}

static VOID cntl_mlme_reassoc_conf(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_reassoc_conf)
		cntl_api->cntl_reassoc_conf(Elem);
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_reassoc_conf hook api.\n",
				  __func__));
	}
}

static VOID cntl_mlme_disassoc_conf(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_disassoc_conf)
		cntl_api->cntl_disassoc_conf(Elem);
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_disassoc_conf hook api.\n",
				  __func__));
	}
}

static VOID cntl_mlme_reset_all_fsm(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	struct _cntl_api_ops *cntl_api;

	wdev = Elem->wdev;
	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;

	if (cntl_api->cntl_reset_all_fsm_proc)
		cntl_api->cntl_reset_all_fsm_proc(Elem);
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_disassoc_conf hook api.\n",
				  __func__));
	}
}

static VOID cntl_mlme_error_handle(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = Elem->wdev;
	struct _cntl_api_ops *cntl_api;
	UCHAR cli_idx = Elem->priv_data.rept_cli_idx;
	ULONG curr_state = wdev->cntl_machine.CurrState;

	cntl_api = (struct _cntl_api_ops *)wdev->cntl_api;
#ifdef MAC_REPEATER_SUPPORT

	if (cli_idx != NON_REPT_ENTRY) {
		REPEATER_CLIENT_ENTRY *pReptEntry = Elem->priv_data.rept_cli_entry;

		curr_state = pReptEntry->CtrlCurrState;
	}

#endif /* MAC_REPEATER_SUPPORT */
	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			 ("%s [%s %s]: [%s][%s] ====================> ERR\n",
			  __func__, wdev->if_dev->name, (cli_idx != NON_REPT_ENTRY) ? "(REPT)" : "",
			  CNTL_FSM_STATE_STR[curr_state],
			  CNTL_FSM_MSG_STR[Elem->MsgType]));

	if (cntl_api->cntl_error_handle)
		cntl_api->cntl_error_handle(Elem);
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: No cntl_error_handle hook api.\n",
				  __func__));
	}
}

static BOOLEAN cntl_fsm_msg_checker(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN isMsgDrop = FALSE;
	struct wifi_dev *wdev = Elem->wdev;

	if (wdev) {
		if (!wdev->DevInfo.WdevActive)
			isMsgDrop = TRUE;

#ifdef APCLI_SUPPORT

		if ((IF_COMBO_HAVE_AP_STA(pAd) && wdev->wdev_type == WDEV_TYPE_STA) &&
			(isValidApCliIf(wdev->func_idx) == FALSE))
			isMsgDrop = TRUE;

#endif /* APCLI_SUPPORT */
	}

	if (isMsgDrop == TRUE) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s [%s]: [%s][%s], CNTL reset & Msg Drop\n",
				  __func__, wdev->if_dev->name,
				  CNTL_FSM_STATE_STR[wdev->cntl_machine.CurrState],
				  CNTL_FSM_MSG_STR[Elem->MsgType]));
		cntl_fsm_state_transition(wdev, NON_REPT_ENTRY, CNTL_IDLE, __func__);
	}

	return isMsgDrop;
}

void cntl_state_machine_init(
	IN struct wifi_dev *wdev,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]) {
#ifdef APCLI_CONNECTION_TRIAL
	RTMP_ADAPTER *pAd;
	PSTA_ADMIN_CONFIG pApCliEntry;
	USHORT ifIndex;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	ifIndex = wdev->func_idx;
	pApCliEntry = &pAd->StaCfg[ifIndex];
#endif /*APCLI_CONNECTION_TRIAL*/

	UCHAR i;


	StateMachineSetMsgChecker(Sm, (STATE_MACHINE_MSG_CHECKER)cntl_fsm_msg_checker);
	StateMachineInit(Sm, Trans, MAX_CNTL_STATE, MAX_CNTL_MSG,
					 (STATE_MACHINE_FUNC) cntl_mlme_error_handle, CNTL_IDLE,
					 CNTL_MACHINE_BASE);
	StateMachineSetAction(Sm, CNTL_IDLE, CNTL_MLME_CONNECT, (STATE_MACHINE_FUNC) cntl_mlme_connect);
	StateMachineSetAction(Sm, CNTL_IDLE, CNTL_MLME_DISCONNECT, (STATE_MACHINE_FUNC) cntl_mlme_disconnect);
	StateMachineSetAction(Sm, CNTL_IDLE, CNTL_MLME_SCAN, (STATE_MACHINE_FUNC) cntl_mlme_scan);
	StateMachineSetAction(Sm, CNTL_IDLE, CNTL_MLME_SCAN_CONF, (STATE_MACHINE_FUNC) cntl_mlme_scan_conf);
	StateMachineSetAction(Sm, CNTL_IDLE, CNTL_MLME_JOIN_CONF, (STATE_MACHINE_FUNC) cntl_mlme_join_conf);	/* for rept */

	/* StateMachineSetAction(Sm, CNTL_WAIT_SYNC, CNTL_MLME_DISCONNECT, (STATE_MACHINE_FUNC) cntl_mlme_disconnect); */
	StateMachineSetAction(Sm, CNTL_WAIT_SYNC, CNTL_MLME_JOIN_CONF, (STATE_MACHINE_FUNC) cntl_mlme_join_conf);
	StateMachineSetAction(Sm, CNTL_WAIT_SYNC, CNTL_MLME_SCAN_CONF, (STATE_MACHINE_FUNC) cntl_mlme_scan_conf);
	StateMachineSetAction(Sm, CNTL_WAIT_SYNC, CNTL_MLME_SCAN, (STATE_MACHINE_FUNC) cntl_mlme_scan);

	StateMachineSetAction(Sm, CNTL_WAIT_AUTH, CNTL_MLME_AUTH_CONF, (STATE_MACHINE_FUNC) cntl_mlme_auth_conf);
	StateMachineSetAction(Sm, CNTL_WAIT_AUTH, CNTL_MLME_DISCONNECT, (STATE_MACHINE_FUNC) cntl_mlme_disconnect);
	StateMachineSetAction(Sm, CNTL_WAIT_AUTH2, CNTL_MLME_AUTH_CONF, (STATE_MACHINE_FUNC) cntl_mlme_auth2_conf);
	StateMachineSetAction(Sm, CNTL_WAIT_AUTH2, CNTL_MLME_DISCONNECT, (STATE_MACHINE_FUNC) cntl_mlme_disconnect);
	StateMachineSetAction(Sm, CNTL_WAIT_DEAUTH, CNTL_MLME_DEAUTH_CONF, (STATE_MACHINE_FUNC) cntl_mlme_deauth_conf);
	StateMachineSetAction(Sm, CNTL_WAIT_ASSOC, CNTL_MLME_ASSOC_CONF, (STATE_MACHINE_FUNC) cntl_mlme_assoc_conf);
	StateMachineSetAction(Sm, CNTL_WAIT_ASSOC, CNTL_MLME_REASSOC_CONF, (STATE_MACHINE_FUNC) cntl_mlme_reassoc_conf);
	StateMachineSetAction(Sm, CNTL_WAIT_ASSOC, CNTL_MLME_DISCONNECT, (STATE_MACHINE_FUNC) cntl_mlme_disconnect);
	StateMachineSetAction(Sm, CNTL_WAIT_DISASSOC, CNTL_MLME_DISASSOC_CONF, (STATE_MACHINE_FUNC) cntl_mlme_disassoc_conf);


#ifdef APCLI_CONNECTION_TRIAL
	StateMachineSetAction(Sm, CNTL_IDLE, APCLI_CTRL_TRIAL_CONNECT, (STATE_MACHINE_FUNC) ApCliCtrlTrialConnectAction);
/* Trial Connect Timer Timeout handling */
	StateMachineSetAction(Sm, CNTL_WAIT_SYNC, APCLI_CTRL_TRIAL_CONNECT_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectTimeoutAction);
	StateMachineSetAction(Sm, CNTL_WAIT_AUTH, APCLI_CTRL_TRIAL_CONNECT_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectTimeoutAction);
	StateMachineSetAction(Sm, CNTL_WAIT_ASSOC, APCLI_CTRL_TRIAL_CONNECT_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectTimeoutAction);
/* Trial Phase2 Timer Timeout handling */
	StateMachineSetAction(Sm, CNTL_WAIT_SYNC, APCLI_CTRL_TRIAL_PHASE2_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialPhase2TimeoutAction);
	StateMachineSetAction(Sm, CNTL_WAIT_AUTH, APCLI_CTRL_TRIAL_PHASE2_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialPhase2TimeoutAction);
	StateMachineSetAction(Sm, CNTL_WAIT_ASSOC, APCLI_CTRL_TRIAL_PHASE2_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialPhase2TimeoutAction);
	StateMachineSetAction(Sm, CNTL_IDLE, APCLI_CTRL_TRIAL_PHASE2_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialPhase2TimeoutAction);
/* Trial Retry Timer Timeout handling */
	StateMachineSetAction(Sm, CNTL_WAIT_SYNC, APCLI_CTRL_TRIAL_RETRY_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectRetryTimeoutAction);
	StateMachineSetAction(Sm, CNTL_WAIT_AUTH, APCLI_CTRL_TRIAL_RETRY_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectRetryTimeoutAction);
	StateMachineSetAction(Sm, CNTL_WAIT_ASSOC, APCLI_CTRL_TRIAL_RETRY_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectRetryTimeoutAction);
	StateMachineSetAction(Sm, CNTL_IDLE, APCLI_CTRL_TRIAL_RETRY_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectRetryTimeoutAction);
#endif /* APCLI_CONNECTION_TRIAL */

	/* Cancel Action */
	for (i = 0; i < MAX_CNTL_STATE; i++)
		StateMachineSetAction(Sm, i, CNTL_MLME_RESET_TO_IDLE, (STATE_MACHINE_FUNC) cntl_mlme_reset_all_fsm);
	wdev->cntl_machine.CurrState = CNTL_IDLE;
#ifdef APCLI_CONNECTION_TRIAL
	if (ifIndex == (pAd->ApCfg.ApCliNum - 1)) {
		/* timer init */
		RTMPInitTimer(pAd,
					  &pApCliEntry->TrialConnectTimer,
					  GET_TIMER_FUNCTION(ApCliTrialConnectTimeout),
					  (PVOID)pApCliEntry,
					  FALSE);
		RTMPInitTimer(pAd,
					  &pApCliEntry->TrialConnectPhase2Timer,
					  GET_TIMER_FUNCTION(ApCliTrialConnectPhase2Timeout),
					  (PVOID)pApCliEntry,
					  FALSE);
		RTMPInitTimer(pAd,
					  &pApCliEntry->TrialConnectRetryTimer,
					  GET_TIMER_FUNCTION(ApCliTrialConnectRetryTimeout),
					  pApCliEntry,
					  FALSE);
		}
#endif /* APCLI_CONNECTION_TRIAL */
}

/* Export API - Start */
BOOLEAN cntl_connect_request(
	struct wifi_dev *wdev,
	enum _CNTL_CONNECT_TYPE conn_type,
	UCHAR data_len,
	UCHAR *data)
{
	RTMP_ADAPTER *pAd;
	CNTL_MLME_CONNECT_STRUCT *cntl_conn;

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	os_alloc_mem(pAd, (UCHAR **)&cntl_conn, sizeof(CNTL_MLME_CONNECT_STRUCT) + data_len);

	if (cntl_conn) {
		cntl_conn->conn_type = conn_type;
		cntl_conn->data_len = data_len;

		if (data && data_len)
			os_move_mem(cntl_conn->data, data, data_len);
		MlmeEnqueueWithWdev(pAd,
							MLME_CNTL_STATE_MACHINE,
							CNTL_MLME_CONNECT,
							sizeof(CNTL_MLME_CONNECT_STRUCT) + data_len,
							cntl_conn,
							0,
							wdev,
							NULL);
		RTMP_MLME_HANDLER(pAd);
		os_free_mem(cntl_conn);
		return TRUE;
	} else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Alloc memory failed.\n",
				  __func__));
	}

	return FALSE;
}

BOOLEAN cntl_disconnect_request(
	struct wifi_dev *wdev,
	enum _CNTL_DISCONNECT_TYPE disconn_type,
	UCHAR *addr,
	USHORT reason,
	struct inter_machine_info *priv_data)
{
	RTMP_ADAPTER *pAd;
	CNTL_MLME_DISCONNECT_STRUCT cntl_disconn;

	ASSERT(wdev->sys_handle);
	cntl_disconn.cntl_disconn_type = disconn_type;
	os_move_mem(cntl_disconn.mlme_disconn.addr, addr, MAC_ADDR_LEN);
	cntl_disconn.mlme_disconn.reason = reason;
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;

	MlmeEnqueueWithWdev(pAd,
						MLME_CNTL_STATE_MACHINE,
						CNTL_MLME_DISCONNECT,
						sizeof(CNTL_MLME_DISCONNECT_STRUCT),
						&cntl_disconn,
						0,
						wdev,
						priv_data);

	RTMP_MLME_HANDLER(pAd);
	return TRUE;
}

BOOLEAN cntl_scan_request(
	struct wifi_dev *wdev,
	MLME_SCAN_REQ_STRUCT *mlme_scan_request)
{
	RTMP_ADAPTER *pAd;

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
		MlmeEnqueueWithWdev(pAd,
						MLME_CNTL_STATE_MACHINE,
						CNTL_MLME_SCAN,
						sizeof(MLME_SCAN_REQ_STRUCT),
						mlme_scan_request,
						0,
						wdev,
						NULL);
		RTMP_MLME_HANDLER(pAd);
	return TRUE;
}

BOOLEAN cntl_scan_conf(
	struct wifi_dev *wdev,
	USHORT status)
{
	RTMP_ADAPTER *pAd;

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	MlmeEnqueueWithWdev(pAd,
						MLME_CNTL_STATE_MACHINE,
						CNTL_MLME_SCAN_CONF,
						sizeof(USHORT),
						&status,
						0,
						wdev,
						NULL);
	RTMP_MLME_HANDLER(pAd);
	return TRUE;
}

BOOLEAN cntl_join_start_conf(
	struct wifi_dev *wdev,
	USHORT status,
	struct inter_machine_info *priv_data)
{
	RTMP_ADAPTER *pAd;

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	MlmeEnqueueWithWdev(pAd,
						MLME_CNTL_STATE_MACHINE,
						CNTL_MLME_JOIN_CONF,
						sizeof(USHORT),
						&status,
						0,
						wdev,
						priv_data);
	RTMP_MLME_HANDLER(pAd);
	return TRUE;
}

BOOLEAN cntl_auth_assoc_conf(
	struct wifi_dev *wdev,
	enum _CNTL_MLME_EVENT event_type,
	USHORT reason,
	struct inter_machine_info *priv_data)
{
	RTMP_ADAPTER *pAd;
	ULONG cntl_curr_state = wdev->cntl_machine.CurrState;
#ifdef MAC_REPEATER_SUPPORT
	{
		UCHAR cli_idx = priv_data->rept_cli_idx;

		if (cli_idx != NON_REPT_ENTRY)
			cntl_curr_state = priv_data->rept_cli_entry->CtrlCurrState;
	}
#endif /* MAC_REPEATER_SUPPORT */
	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;

	if ((event_type == CNTL_MLME_DISASSOC_CONF)
		&& (cntl_curr_state != CNTL_WAIT_DISASSOC)) {
		/*
		 *	Ignore this message directly for this case.
		 */
		return FALSE;
	}

	MlmeEnqueueWithWdev(pAd,
						MLME_CNTL_STATE_MACHINE,
						event_type,
						sizeof(USHORT),
						&reason,
						0,
						wdev,
						priv_data);
	RTMP_MLME_HANDLER(pAd);
	return TRUE;
}

BOOLEAN cntl_do_disassoc_now(
	struct wifi_dev *wdev)
{
	if (wdev->cntl_machine.CurrState == CNTL_WAIT_DISASSOC)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN cntl_idle(
	struct wifi_dev *wdev)
{
	return (wdev->cntl_machine.CurrState == CNTL_IDLE ? TRUE : FALSE);
}

VOID cntl_fsm_reset(struct wifi_dev *wdev)
{
	cntl_fsm_state_transition(wdev, NON_REPT_ENTRY, CNTL_IDLE, __func__);
}

BOOLEAN cntl_reset_all_fsm_in_ifdown(
	struct wifi_dev *wdev)
{
	RTMP_ADAPTER *pAd;
	USHORT status = MLME_SUCCESS;

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	MlmeEnqueueWithWdev(pAd,
						MLME_CNTL_STATE_MACHINE,
						CNTL_MLME_RESET_TO_IDLE,
						sizeof(USHORT),
						&status,
						0,
						wdev,
						NULL);
	RTMP_MLME_HANDLER(pAd);
	return TRUE;
}


/* Export API - End */

#ifdef APCLI_CONNECTION_TRIAL
BOOLEAN trial_cntl_connect_request(
	struct wifi_dev *wdev,
	enum _CNTL_CONNECT_TYPE conn_type,
	UCHAR data_len,
	UCHAR *data)
{
	RTMP_ADAPTER *pAd;
	CNTL_MLME_CONNECT_STRUCT *cntl_conn;
	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	os_alloc_mem(pAd, (UCHAR **)&cntl_conn, sizeof(CNTL_MLME_CONNECT_STRUCT) + data_len);

	if (cntl_conn) {
		cntl_conn->conn_type = conn_type;
		cntl_conn->data_len = data_len;

		if (data && data_len)
			os_move_mem(cntl_conn->data, data, data_len);
		MlmeEnqueueWithWdev(pAd,
							MLME_CNTL_STATE_MACHINE,
							APCLI_CTRL_TRIAL_CONNECT,
							sizeof(CNTL_MLME_CONNECT_STRUCT) + data_len,
							cntl_conn,
							0,
							wdev,
							NULL);
		RTMP_MLME_HANDLER(pAd);
		os_free_mem(cntl_conn);
		return TRUE;
	} else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Alloc memory failed.\n",
				  __func__));
	}

	return FALSE;
}

static VOID ApCliTrialConnectionBeaconControl(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, BOOLEAN start)
{
	struct wifi_dev *wdevMbss;
	INT IdBss;
	INT band_index;
	INT MaxNumBss;
	MaxNumBss = pAd->ApCfg.BssidNum;
	band_index = GetBandByChannel(wdev->channel, pAd);
	for (IdBss = 0; IdBss < MaxNumBss; IdBss++) {
		wdevMbss = &pAd->ApCfg.MBSSID[IdBss].wdev;
	if (HcGetBandByWdev(wdevMbss) == band_index)
		if (WDEV_BSS_STATE(wdevMbss) == BSS_READY) {
			UpdateBeaconHandler(
				pAd,
				wdevMbss,
				(start) ? BCN_UPDATE_ENABLE_TX : BCN_UPDATE_DISABLE_TX);
		}
	}
}

static VOID JoinParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_JOIN_REQ_STRUCT * JoinReq,
	IN ULONG BssIdx)
{
	JoinReq->BssIdx = BssIdx;
}

static VOID ApCliCtrlTrialConnectAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	MLME_JOIN_REQ_STRUCT JoinReq;
	PSTA_ADMIN_CONFIG pApCliEntry;
	struct wifi_dev *wdev;
	wdev = Elem->wdev;
	USHORT ifIndex = wdev->func_idx;
	PULONG pCurrState = &pAd->StaCfg[ifIndex].wdev.cntl_machine.CurrState;
	BOOLEAN	Cancelled;
	struct freq_oper oper;
	hc_radio_query_by_wdev(wdev, &oper);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Start Probe Req Trial.\n", __func__));
	if (ifIndex >= MAX_APCLI_NUM || ifIndex == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) Index: %d error!!\n", __func__, ifIndex));
		return;
	}

	if (scan_in_run_state(pAd, wdev) == TRUE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) Ap Scanning.....\n", __func__));
		return;
	}

	pApCliEntry = &pAd->StaCfg[ifIndex];

	if (!(pApCliEntry->TrialCh)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) Didn't assign the RootAP channel\n", __func__));
		return;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) channel TrialConnectTimer\n", __func__));
	RTMPCancelTimer(&(pApCliEntry->TrialConnectTimer), &Cancelled);
	pApCliEntry->PrevCh = oper.prim_ch;
	if (pApCliEntry->TrialCh != oper.prim_ch) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Jump to CH:%d\n", __func__, pApCliEntry->TrialCh));
		/* TBD disable beacon? */
		ApCliTrialConnectionBeaconControl(pAd, wdev, FALSE);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s[%d]Change ch %d to %d\n\r",
				__func__, __LINE__,
				pApCliEntry->wdev.channel, pApCliEntry->TrialCh));
		/* switch channel to trial channel */
		wlan_operate_scan(wdev, pApCliEntry->TrialCh);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) set  TrialConnectTimer(%d ms)\n", __func__, TRIAL_TIMEOUT));
	RTMPSetTimer(&(pApCliEntry->TrialConnectTimer), TRIAL_TIMEOUT);

	NdisZeroMemory(&JoinReq, sizeof(MLME_JOIN_REQ_STRUCT));

	if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
		COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);

	if (pApCliEntry->CfgSsidLen != 0) {
		JoinReq.SsidLen = pApCliEntry->CfgSsidLen;
		NdisMoveMemory(&(JoinReq.Ssid), pApCliEntry->CfgSsid, JoinReq.SsidLen);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Probe Ssid=%s, Bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
			 __func__, JoinReq.Ssid, JoinReq.Bssid[0], JoinReq.Bssid[1], JoinReq.Bssid[2],
			 JoinReq.Bssid[3], JoinReq.Bssid[4], JoinReq.Bssid[5]));
	cntl_fsm_state_transition(wdev, NON_REPT_ENTRY, CNTL_WAIT_SYNC, __func__);
	JoinParmFill(pAd, &JoinReq, BSS_NOT_FOUND);
	MlmeEnqueueWithWdev(pAd, SYNC_FSM,	SYNC_FSM_JOIN_REQ, sizeof(MLME_JOIN_REQ_STRUCT), &JoinReq, ifIndex, wdev, NULL);
}

static VOID ApCliTrialConnectTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	struct wifi_dev *wdev = (struct wifi_dev *)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;
		MlmeEnqueueWithWdev(pAd,
							MLME_CNTL_STATE_MACHINE,
							APCLI_CTRL_TRIAL_CONNECT_TIMEOUT,
							sizeof(PVOID),
							&FunctionContext,
							0,
							wdev,
							NULL);
	RTMP_MLME_HANDLER(pAd);
}

static VOID ApCliTrialConnectPhase2Timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	struct wifi_dev *wdev = (struct wifi_dev *)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;

	MlmeEnqueueWithWdev(pAd,
							MLME_CNTL_STATE_MACHINE,
							APCLI_CTRL_TRIAL_PHASE2_TIMEOUT,
							sizeof(PVOID),
							&FunctionContext,
							0,
							wdev,
							NULL);
	RTMP_MLME_HANDLER(pAd);
}

static VOID ApCliTrialConnectRetryTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	struct wifi_dev *wdev = (struct wifi_dev *)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	MlmeEnqueueWithWdev(pAd,
							MLME_CNTL_STATE_MACHINE,
							APCLI_CTRL_TRIAL_RETRY_TIMEOUT,
							sizeof(PVOID),
							&FunctionContext,
							0,
							wdev,
							NULL);
	RTMP_MLME_HANDLER(pAd);
}


static VOID ApCliTrialConnectTimeoutAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *pElem)
{
	PSTA_ADMIN_CONFIG	pApCliEntry;
	USHORT	ifIndex;
	PULONG	pCurrState;
	UCHAR ch;
	SCAN_CTRL *ScanCtrl = NULL;
	struct freq_oper oper;
	struct wifi_dev *wdev;
	wdev = pElem->wdev;
	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
	ifIndex = wdev->func_idx;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	pCurrState = &pAd->StaCfg[ifIndex].wdev.cntl_machine.CurrState;
	/*query physical radio setting by wdev*/
	hc_radio_query_by_wdev(&pApCliEntry->wdev, &oper);
	if (pApCliEntry->TrialCh != pApCliEntry->PrevCh) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s[%d]Change ch %d to %d(%d)\n\r",
				__func__, __LINE__,
				pApCliEntry->TrialCh, pApCliEntry->PrevCh, pApCliEntry->PrevCh));
		pApCliEntry->wdev.channel = pApCliEntry->PrevCh;
		wlan_operate_set_prim_ch(&pApCliEntry->wdev, pApCliEntry->PrevCh);
		/* TBD regenerate beacon? */
		ApCliTrialConnectionBeaconControl(pAd, wdev, TRUE);
	}

	if (*pCurrState == CNTL_WAIT_ASSOC) {
		/* trialConnectTimeout, and currect status is ASSOC, */
		/* it means we got Auth Resp from new root AP already, */
		/* we shall serve the origin channel traffic first, */
		/* and jump back to trial channel to issue Assoc Req later, */
		/* and finish four way-handshake if need. */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, ApCliTrialConnectTimeout APCLI_CTRL_ASSOC set TrialConnectPhase2Timer\n", __func__));
		RTMPSetTimer(&(pApCliEntry->TrialConnectPhase2Timer), TRIAL_TIMEOUT);
	} else {
		/* RTMPCancelTimer(&(pApCliEntry->ApCliMlmeAux.ProbeTimer), &Cancelled); */
		pApCliEntry->NewRootApRetryCnt++;

		if (pApCliEntry->NewRootApRetryCnt >= MAX_TRIAL_COUNT) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, RetryCnt:%d, pCurrState = %lu,\n", __func__, pApCliEntry->NewRootApRetryCnt, *pCurrState));
			pApCliEntry->TrialCh = 0;
			cntl_disconnect_request(wdev, CNTL_DISASSOC, pApCliEntry->MlmeAux.Bssid, REASON_DISASSOC_STA_LEAVING, NULL);
			NdisZeroMemory(pAd->StaCfg[ifIndex].CfgSsid, MAX_LEN_OF_SSID);/* cleanup CfgSsid. */
			pApCliEntry->CfgSsidLen = 0;
			pApCliEntry->NewRootApRetryCnt = 0;/* cleanup retry count */
			pApCliEntry->ApcliInfStat.Enable = FALSE;
		} else {
			/* trial connection probe fail                                               */
			/* change apcli sync state machine to idle state to reset sync state machine */
			PULONG pSync_CurrState = &ScanCtrl->SyncFsm.CurrState;
			*pSync_CurrState = SYNC_FSM_IDLE;
			*pCurrState = CNTL_IDLE;/* Disconnected State will bring the next probe req, auth req. */
		}
	}
}

static VOID ApCliTrialPhase2TimeoutAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *pElem)
{
	PSTA_ADMIN_CONFIG pApCliEntry;
	MLME_ASSOC_REQ_STRUCT  AssocReq;
	USHORT ifIndex;
	struct wifi_dev *wdev;
	struct freq_oper oper;
	wdev = pElem->wdev;
	UINT link_down_type = 0;
	PMAC_TABLE_ENTRY pMacEntry;
	STA_TR_ENTRY *tr_entry;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	ifIndex = wdev->func_idx;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	/*query physical radio setting by wdev*/
	hc_radio_query_by_wdev(wdev, &oper);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCli_SYNC - %s,\nJump back to trial channel:%d\nto issue Assoc Req to new root AP\n",
			 __func__, pApCliEntry->TrialCh));
	pMacEntry = MacTableLookup(pAd, pApCliEntry->MlmeAux.Bssid);
	tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];

	if (pApCliEntry->TrialCh != pApCliEntry->PrevCh) {
		/* TBD disable beacon? */
		ApCliTrialConnectionBeaconControl(pAd, wdev, FALSE);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s[%d]Change ch %d to %d\n\r",
				  __func__, __LINE__,
				 pApCliEntry->PrevCh, pApCliEntry->TrialCh));
		/* switch channel to trial channel */
		wlan_operate_scan(wdev, pApCliEntry->TrialCh);
	}

	link_down_type |= LINK_HAVE_INTER_SM_DATA;

	/* if (wdev->AuthMode >= Ndis802_11AuthModeWPA) */
	if (IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap)) {
		RTMPSetTimer(&(pApCliEntry->TrialConnectRetryTimer), (TRIAL_PHASE2_TIMEOUT + pApCliEntry->NewRootApRetryCnt*200));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) set  TrialConnectRetryTimer(%d ms)\n", __func__, (TRIAL_PHASE2_TIMEOUT  + pApCliEntry->NewRootApRetryCnt*200)));
	} else {
		RTMPSetTimer(&(pApCliEntry->TrialConnectRetryTimer), TRIAL_TIMEOUT);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) set  TrialConnectRetryTimer(%d ms)\n", __func__, TRIAL_TIMEOUT));
	}
	tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	AssocParmFill(pAd, &AssocReq, pStaCfg->MlmeAux.Bssid,
						  pStaCfg->MlmeAux.CapabilityInfo,
						  ASSOC_TIMEOUT,
						  pStaCfg->DefaultListenCount);
	MlmeEnqueueWithWdev(pAd, ASSOC_FSM,
							ASSOC_FSM_MLME_ASSOC_REQ,
							sizeof(MLME_ASSOC_REQ_STRUCT),
							&AssocReq, 0, wdev, &pElem->priv_data);
	RTMP_MLME_HANDLER(pAd);
}

static VOID ApCliTrialConnectRetryTimeoutAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *pElem)
{
	PSTA_ADMIN_CONFIG pApCliEntry;
	PULONG pCurrState;
	USHORT ifIndex;
	UCHAR ch;
	PMAC_TABLE_ENTRY pMacEntry;
	STA_TR_ENTRY *tr_entry;
	struct freq_oper oper;
	struct wifi_dev *wdev;
	wdev = pElem->wdev;
	UINT link_down_type = 0;
	/* PMAC_TABLE_ENTRY pOldRootAp = &pApCliEntry->oldRootAP; */
	ifIndex = wdev->func_idx;
	pCurrState = &pAd->StaCfg[ifIndex].wdev.cntl_machine.CurrState;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	pMacEntry = MacTableLookup(pAd, pApCliEntry->MlmeAux.Bssid);
	/*query physical radio setting by wdev*/
	hc_radio_query_by_wdev(&pApCliEntry->wdev, &oper);
	if (pMacEntry == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ApCli_SYNC - %s, no CfgApCliBssid in mactable!\n", __func__));
		/* *pCurrState = APCLI_CTRL_DISCONNECTED; */
		pApCliEntry->NewRootApRetryCnt++;

		if (pApCliEntry->NewRootApRetryCnt >= MAX_TRIAL_COUNT) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, RetryCnt:%d, pCurrState = %lu,\n", __func__, pApCliEntry->NewRootApRetryCnt, *pCurrState));
			pApCliEntry->TrialCh = 0;
			link_down_type |= LINK_HAVE_INTER_SM_DATA;
			cntl_disconnect_request(wdev, CNTL_DISASSOC, pApCliEntry->MlmeAux.Bssid, REASON_DISASSOC_STA_LEAVING, NULL);
			NdisZeroMemory(pAd->StaCfg[ifIndex].CfgSsid, MAX_LEN_OF_SSID);/* cleanup CfgSsid. */
			pApCliEntry->CfgSsidLen = 0;
			pApCliEntry->NewRootApRetryCnt = 0;/* cleanup retry count */
			pApCliEntry->ApcliInfStat.Enable = FALSE;
		}

		if (pApCliEntry->TrialCh != pApCliEntry->PrevCh) {
			ch = pApCliEntry->PrevCh;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s[%d]Change ch %d to %d(%d)\n\r",
					__func__, __LINE__,
					pApCliEntry->TrialCh, ch, ch));
			/* switch channel to orignal channel */
			pApCliEntry->wdev.channel = ch;
			wlan_operate_set_prim_ch(&pApCliEntry->wdev, ch);
			/* TBD enable beacon? */
			ApCliTrialConnectionBeaconControl(pAd, wdev, TRUE);
		}

		*pCurrState = CNTL_IDLE;
		return;
	}

	tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];

	if ((tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) && (*pCurrState == CNTL_IDLE)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCli_SYNC - %s, new rootAP connected!!\n", __func__));
		/* connected to new ap ok, change common channel to new channel */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCli_SYNC - %s, jump back to origin channel to wait for User's operation!\n", __func__));
		if (pApCliEntry->TrialCh != pApCliEntry->PrevCh) {
			ch = pApCliEntry->PrevCh;
			oper.prim_ch = ch;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s[%d]Change ch %d to %d(%d)\n\r",
					__func__, __LINE__,
					pApCliEntry->TrialCh, ch, ch));
			/* switch channel to orignal channel */
			pApCliEntry->wdev.channel = ch;
			wlan_operate_set_prim_ch(&pApCliEntry->wdev, ch);
			/* TBD enable beacon? */
			ApCliTrialConnectionBeaconControl(pAd, wdev, TRUE);
		}

		NdisZeroMemory(pAd->StaCfg[ifIndex].CfgSsid, MAX_LEN_OF_SSID);/* cleanup CfgSsid. */
		pApCliEntry->CfgSsidLen = 0;
		pApCliEntry->NewRootApRetryCnt = 0;/* cleanup retry count */
		pApCliEntry->TrialCh = 0;
	} else {
		/*
		   Apcli does not connect to new root ap successfully yet,
		   jump back to origin channel to serve old rootap traffic.
		   re-issue assoc_req to go later.
		*/
		/* pApCliEntry->MacTabWCID = pOldRootAp->Aid; */
		pApCliEntry->NewRootApRetryCnt++;
		if (pApCliEntry->NewRootApRetryCnt < MAX_TRIAL_COUNT)
				RTMPSetTimer(&(pApCliEntry->TrialConnectPhase2Timer), TRIAL_TIMEOUT);
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, RetryCnt:%d, pCurrState = %lu,\n", __func__, pApCliEntry->NewRootApRetryCnt, *pCurrState));
			pApCliEntry->TrialCh = 0;
			cntl_disconnect_request(wdev, CNTL_DISASSOC, pApCliEntry->MlmeAux.Bssid, REASON_DISASSOC_STA_LEAVING, NULL);
			NdisZeroMemory(pAd->StaCfg[ifIndex].CfgSsid, MAX_LEN_OF_SSID);/* cleanup CfgSsid. */
			pApCliEntry->CfgSsidLen = 0;
			pApCliEntry->NewRootApRetryCnt = 0;/* cleanup retry count */
			pApCliEntry->ApcliInfStat.Enable = FALSE;
			link_down_type |= LINK_HAVE_INTER_SM_DATA;
		}

		if (pApCliEntry->TrialCh != pApCliEntry->PrevCh) {
			ch = pApCliEntry->PrevCh;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s[%d]Change ch %d to %d(%d)\n\r",
					__func__, __LINE__,
					pApCliEntry->TrialCh, ch, ch));
			/* switch channel to orignal channel */
			pApCliEntry->wdev.channel = ch;
			wlan_operate_set_prim_ch(&pApCliEntry->wdev, ch);
			/* TBD enable beacon? */
			ApCliTrialConnectionBeaconControl(pAd, wdev, TRUE);
		}
	}
}

UINT GetBandByChannel(UCHAR ch, RTMP_ADAPTER *pAd)
{
	UINT BandIdx = 0;
	UINT idx;
	CHANNEL_CTRL *pChCtrl;
	if ((pAd) && (pAd->CommonCfg.dbdc_mode))
		for (BandIdx = 0; BandIdx <= 1; BandIdx++) {
			pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
			for (idx = 0; idx < pChCtrl->ChListNum; idx++) {
				if (pChCtrl->ChList[idx].Channel == ch)
					return BandIdx;
			}
		}
	else
		return BandIdx;
}

#endif /* APCLI_CONNECTION_TRIAL */
