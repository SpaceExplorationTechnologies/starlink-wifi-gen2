#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <lua.h>							   /* Always include this */
#include <lauxlib.h>						   /* Always include this */
#include <lualib.h>							/* Always include this */
/* MAP related includes */
#include "mapd_interface_ctrl.h"

/* MAP related function declarations */
int triggerMandateSteeringOnAgent(lua_State *L);
int triggerBackhaulSteeringOnAgent(lua_State *L);
int triggerWpsFhAgent(lua_State *L);
int triggerMultiApOnBoarding(lua_State *L);
int triggerUplinkApSelection(lua_State *L);
int getRuntimeTopology(lua_State *L);
int getClientCapabilities(lua_State *L);
int getApCapabilities(lua_State *L);
int getDeviceRole(lua_State *L);
int getBhConnectionStatus(lua_State *L);
int applyApSteerRssiTh(lua_State *L);
int applyChannelUtilizationTh(lua_State *L);
int applyBssConfigRenew(lua_State *L);
int getApBhInfList(lua_State *L);
int getApFhInfList(lua_State *L);
int applyWifiBhPriority(lua_State *L);
int applyForceChSwitch(lua_State *L);
int applyUserPreChannel(lua_State *L);
int triggerChPlanningR2(lua_State *L);
int triggerDeDump(lua_State *L);
int getDataElement(lua_State *L);
int triggerChannelScan(lua_State *L);
int getChannelStats(lua_State *L);
int getChannelPlanningScore(lua_State *L);

/* Logging facility */
void log_c(int line, char *fmt,...);
#define LOG_C(...) log_c(__LINE__, __VA_ARGS__ )
int logDisable = 1;

int luaopen_map_helper(lua_State *L)
{
	lua_register(L,"c_get_device_role",getDeviceRole);
	lua_register(L,"c_trigger_mandate_steering_on_agent",triggerMandateSteeringOnAgent);
	lua_register(L,"c_trigger_back_haul_steering_on_agent",triggerBackhaulSteeringOnAgent);
	lua_register(L,"c_trigger_wps_fh_agent",triggerWpsFhAgent);
	lua_register(L,"c_trigger_multi_ap_on_boarding",triggerMultiApOnBoarding);
	lua_register(L,"c_trigger_uplink_ap_selection",triggerUplinkApSelection);
	lua_register(L,"c_get_runtime_topology",getRuntimeTopology);
	lua_register(L,"c_get_client_capabilities",getClientCapabilities);
	lua_register(L,"c_get_bh_connection_status",getBhConnectionStatus);
	lua_register(L,"c_apply_ap_steer_rssi_th",applyApSteerRssiTh);
	lua_register(L,"c_apply_channel_utilization_th",applyChannelUtilizationTh);
	lua_register(L,"c_apply_bss_config_renew",applyBssConfigRenew);
	lua_register(L,"c_get_ap_bh_inf_list",getApBhInfList);
	lua_register(L,"c_get_ap_fh_inf_list",getApFhInfList);
	lua_register(L,"c_apply_wifi_bh_priority",applyWifiBhPriority);
	lua_register(L,"c_apply_force_ch_switch",applyForceChSwitch);
	lua_register(L,"c_apply_user_preferred_channel",applyUserPreChannel);
	lua_register(L,"c_trigger_channel_planning_r2",triggerChPlanningR2);
	lua_register(L,"c_trigger_de_dump",triggerDeDump);
	lua_register(L,"c_get_data_element",getDataElement);
	lua_register(L,"c_trigger_channel_scan",triggerChannelScan);
	lua_register(L,"c_get_channel_stats",getChannelStats);
	lua_register(L,"c_get_channel_planning_score",getChannelPlanningScore);
	return 0;
}

void log_c(int line, char *fmt,...)
{
    va_list list;
    FILE *fp;
    char *p, *r;
    int e;

    if (logDisable == 1){
        return;
    }
    fp = fopen("/tmp/log_ui_map","a+");
    if(fp == NULL){
        return;
    }

    fprintf(fp,"[%d] ",line);
    va_start(list, fmt);

    for(p = fmt; *p; ++p){
        if(*p != '%'){
            fputc(*p,fp);
        }
        else{
            switch (*++p){
                /* string */
                case 's':
                    r = va_arg(list, char*);
                    fprintf(fp, "%s", r);
                    continue;
                /* integer */
                case 'd':
                    e = va_arg(list, int);
                    fprintf(fp, "%d", e);
                    continue;
                default:
                    fputc(*p, fp);
            }
        }
    }
    va_end(list);
    fputc('\n', fp);
    fclose(fp);
}

int triggerMandateSteeringOnAgent(lua_State *L)
{
	const char *mac_addr = luaL_checkstring(L, 1);
	const char *bssid = luaL_checkstring(L, 2);
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;

	lua_newtable(L);

	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}

	ret = mapd_interface_mandate_steer(ctrl, mac_addr, bssid);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_mandate_steer API Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_mandate_steer API Failed \n\n", __FUNCTION__);
	}
	else{
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS \n\n", __FUNCTION__);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int triggerBackhaulSteeringOnAgent(lua_State *L)
{
	const char *bh_mac_addr = luaL_checkstring(L, 1);
	const char *bh_target_mac_addr = luaL_checkstring(L, 2);
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;

	lua_newtable(L);

	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}

	ret = mapd_interface_bh_steer(ctrl, bh_mac_addr, bh_target_mac_addr);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_bh_steer API Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_bh_steer API Failed \n\n", __FUNCTION__);
	}
	else{
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS \n\n", __FUNCTION__);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int triggerWpsFhAgent(lua_State *L)
{
	const char *fh_mac_addr = luaL_checkstring(L, 1);
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;

	lua_newtable(L);

	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}

	ret = mapd_interface_trigger_map_wps(ctrl, fh_mac_addr);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_trigger_map_wps API returned -1!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_trigger_map_wps API Failed \n\n", __FUNCTION__);
	}
	else{
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS \n\n", __FUNCTION__);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int triggerMultiApOnBoarding(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;
	const char *ifmed = luaL_checkstring(L, 1);
	lua_newtable(L);

	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}

	ret = mapd_interface_trigger_onboarding(ctrl, ifmed); /* ifmed: 1 - Wireless and 0 - Ethernet */
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_trigger_onboarding API returned -1!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_trigger_onboarding API returned -1\n\n", __FUNCTION__);
	}
	else{
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS \n\n", __FUNCTION__);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int triggerUplinkApSelection(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;
	lua_newtable(L);
	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}
	ret = mapd_interface_select_best_ap(ctrl);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == -2){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Timed-out!");  /* push value */
		lua_settable(L, -3);
	}
	else{
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int getRuntimeTopology(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;
	char bufStr[128] = {0};
	char topo_buf[10240] = {0};
	size_t buf_len = 10240;
	lua_newtable(L);
	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
       log_c(__LINE__, "[map_helper] %s enter\n", __FUNCTION__);
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}
	ret = mapd_interface_get_topology(ctrl, topo_buf, &buf_len, NULL);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == 0){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		lua_pushstring(L, "luaTopologyInfo");  /* push key */
		lua_pushstring(L, topo_buf);  /* push value */
		lua_settable(L, -3);
		snprintf(bufStr, sizeof(bufStr), "%zu",buf_len);
		lua_pushstring(L, "luaTopologyInfoLen");  /* push key */
		lua_pushstring(L, bufStr);  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS: %s \n\n", __FUNCTION__, topo_buf);
	}
	else{
		snprintf(bufStr, sizeof(bufStr), "Error: %d",ret);
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, bufStr);  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s fail  error: %d \n\n", __FUNCTION__, ret);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int triggerDeDump(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;
	const char *almac = luaL_checkstring(L, 1);
	lua_newtable(L);
	log_c(__LINE__, "[map_helper] %s enter\n", __FUNCTION__);

	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}

	ret = mapd_interface_trigger_de_dump(ctrl, almac);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == -2){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Timed-out!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Timed-out\n\n", __FUNCTION__);
	}
	else{
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS  \n\n", __FUNCTION__);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int getDataElement(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;
	char bufStr[128] = {0};
	char data_element_buf[10240] = {0};
	size_t buf_len = 10240;
	lua_newtable(L);
	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	log_c(__LINE__, "[map_helper] %s enter\n", __FUNCTION__);
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}
	ret = mapd_interface_get_de_dump(ctrl, data_element_buf, &buf_len, NULL);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == 0){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		lua_pushstring(L, "luaDataElementInfo");  /* push key */
		lua_pushstring(L, data_element_buf);  /* push value */
		lua_settable(L, -3);
		snprintf(bufStr, sizeof(bufStr), "%zu",buf_len);
		lua_pushstring(L, "luaDataElementInfoLen");  /* push key */
		lua_pushstring(L, bufStr);  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS: %s \n\n", __FUNCTION__, data_element_buf);
	}
	else{
		snprintf(bufStr, sizeof(bufStr), "Error: %d",ret);
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, bufStr);  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s fail  error: %d \n\n", __FUNCTION__, ret);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int getClientCapabilities(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	struct client_db *cli_dbs = NULL;
	int num_clis, cli_idx, max_bw_idx, bit_idx;
	char bufStr[128] = {0};
	char tmpStr[128] = {0};
	char cli_db_buf[15360] = {0};
	size_t cli_db_buf_len = sizeof(cli_db_buf);

	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_newtable(L);
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		return 1;
	}
	num_clis = mapd_interface_get_client_db(ctrl, (struct client_db*)cli_db_buf, &cli_db_buf_len);
	if(num_clis < 0){
		lua_newtable(L);
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
		return 1;
	}
	else if(num_clis == 0){
		lua_newtable(L);
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "No Clients Available");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s No Clients Available\n\n", __FUNCTION__);
		return 1;
	}
	else{
		lua_newtable(L);
		lua_pushstring(L, "num_clis");  /* push key */
		snprintf(bufStr, sizeof(bufStr), "%d", num_clis);
		lua_pushstring(L, bufStr);  /* push value */
		lua_settable(L, -3);

		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);

		lua_pushstring(L, "cli_db");  /* push key */

		/* Create parent table of size as num_clis sub-tables */
		lua_createtable(L, 0, num_clis);

		cli_dbs = (struct client_db*)cli_db_buf;

		for(cli_idx=0; cli_idx < num_clis; cli_idx++){
			snprintf(bufStr, sizeof(bufStr), "%d", cli_idx);
			lua_pushstring(L, bufStr);

			/* Create child non-sequence table of size as per struct client_db */
			lua_createtable(L, 0, 9);

			// MAC Address
			snprintf(bufStr, sizeof(bufStr), "%02X:%02X:%02X:%02X:%02X:%02X",
			cli_dbs[cli_idx].mac[0], cli_dbs[cli_idx].mac[1],
			cli_dbs[cli_idx].mac[2], cli_dbs[cli_idx].mac[3],
			cli_dbs[cli_idx].mac[4], cli_dbs[cli_idx].mac[5]);
			lua_pushstring(L, bufStr);
			lua_setfield(L, -2, "MAC");

			// BSSID
			snprintf(bufStr, sizeof(bufStr), "%02X:%02X:%02X:%02X:%02X:%02X",
			cli_dbs[cli_idx].bssid[0], cli_dbs[cli_idx].bssid[1],
			cli_dbs[cli_idx].bssid[2], cli_dbs[cli_idx].bssid[3],
			cli_dbs[cli_idx].bssid[4], cli_dbs[cli_idx].bssid[5]);
			lua_pushstring(L, bufStr);
			lua_setfield(L, -2, "BSSID");

			// Capability
			bufStr[0] = '\0';
			for(bit_idx=0; bit_idx < 4; bit_idx++){
				if(cli_dbs[cli_idx].capab & (1 << bit_idx)){
					switch(bit_idx){
						case 0:
							strncat(bufStr,
									"DOT11K_SUPPORTED, ",
									sizeof(bufStr) - strlen(bufStr) - 1);
						break;
						case 1:
							strncat(bufStr,
									"DOT11V_SUPPORTED, ",
									sizeof(bufStr) - strlen(bufStr) - 1);
						break;
						case 2:
							strncat(bufStr,
									"DOT11R_SUPPORTED, ",
									sizeof(bufStr) - strlen(bufStr) - 1);
						break;
						case 3:
							strncat(bufStr,
									"MBO_SUPPORTED, ",
									sizeof(bufStr) - strlen(bufStr) - 1);
						break;
						default:
						break;
					}
				}
			}
			if(bufStr[0] == '\0'){
				snprintf(bufStr, sizeof(bufStr), "N/A");
			}
			else{
				bufStr[strlen(bufStr) - 2] = '\0'; //Remove last comma and space character
			}
			lua_pushstring(L, bufStr);
			lua_setfield(L, -2, "Capability");

			// Phy Mode
			switch(cli_dbs[cli_idx].phy_mode){
				case 0:
					snprintf(bufStr, sizeof(bufStr), "CCK");
				break;
				case 1:
					snprintf(bufStr, sizeof(bufStr), "OFDM");
				break;
				case 2:
					snprintf(bufStr, sizeof(bufStr), "HTMIX");
				break;
				case 3:
					snprintf(bufStr, sizeof(bufStr), "GREENFIELD");
				break;
				case 4:
					snprintf(bufStr, sizeof(bufStr), "VHT");
				break;
				default:
					snprintf(bufStr, sizeof(bufStr), "N/A");
			}
			lua_pushstring(L, bufStr);
			lua_setfield(L, -2, "Phy Mode");

			// Max. Bandwidth 0 and Max. Bandwidth 1
			for(max_bw_idx=0; max_bw_idx < 2; max_bw_idx++){
				switch(cli_dbs[cli_idx].max_bw[max_bw_idx]){
					case 0:
						snprintf(bufStr, sizeof(bufStr), "20");
					break;
					case 1:
						snprintf(bufStr, sizeof(bufStr), "40");
					break;
					case 2:
						snprintf(bufStr, sizeof(bufStr), "80");
					break;
					case 3:
						snprintf(bufStr, sizeof(bufStr), "160");
					break;
					case 4:
						snprintf(bufStr, sizeof(bufStr), "10");
					break;
					case 5:
						snprintf(bufStr, sizeof(bufStr), "5");
					break;
					case 6:
						snprintf(bufStr, sizeof(bufStr), "8080");
					break;
					default:
						snprintf(bufStr, sizeof(bufStr), "N/A");
				}
				lua_pushstring(L, bufStr);
				snprintf(bufStr, sizeof(bufStr), "Max. Bandwidth %d",max_bw_idx);
				lua_setfield(L, -2, (const char*)bufStr);
			}

			// Spatial Stream
			snprintf(bufStr, sizeof(bufStr), "%d", cli_dbs[cli_idx].spatial_stream);
			lua_pushstring(L, bufStr);
			lua_setfield(L, -2, "Spatial Stream");

			// Known Band
			bufStr[0] = '\0';
			for(bit_idx=0; bit_idx < 2; bit_idx++){
				if(cli_dbs[cli_idx].know_band & (1 << bit_idx)){
					switch(bit_idx){
						case 0:
							strncat(bufStr,
									"2GHz, ",
									sizeof(bufStr) - strlen(bufStr) - 1);
						break;
						case 1:
							strncat(bufStr,
									"5GHz, ",
									sizeof(bufStr) - strlen(bufStr) - 1);
						break;
						default:
						break;
					}
				}
			}
			if(bufStr[0] == '\0'){
				snprintf(bufStr, sizeof(bufStr), "N/A");
			}
			else{
				bufStr[strlen(bufStr) - 2] = '\0'; //Remove last comma and space character
			}
			lua_pushstring(L, bufStr);
			lua_setfield(L, -2, "Known Band");

			// Known Channels
			bufStr[0] = '\0';
			for (bit_idx = 0; bit_idx < 38; bit_idx++) {
				if (cli_dbs[cli_idx].know_channels[bit_idx / 8] & (1 << (bit_idx % 8))) {
					if (bit_idx >= 0 && bit_idx <= 13){
						snprintf(tmpStr, sizeof(tmpStr), "%d, ", bit_idx + 1);
						strncat(bufStr,
								tmpStr,
								sizeof(bufStr) - strlen(bufStr) - 1);
					}
					else if (bit_idx >= 14 && bit_idx <= 21){
						snprintf(tmpStr, sizeof(tmpStr), "%d, ", 36 + ((bit_idx - 14) * 4));
						strncat(bufStr,
								tmpStr,
								sizeof(bufStr) - strlen(bufStr) - 1);
					}
					else if (bit_idx >= 22 && bit_idx <= 32){
						snprintf(tmpStr, sizeof(tmpStr), "%d, ", 100 + ((bit_idx - 22) * 4));
						strncat(bufStr,
								tmpStr,
								sizeof(bufStr) - strlen(bufStr) - 1);
					}
					else if (bit_idx >= 33 && bit_idx <= 37){
						snprintf(tmpStr, sizeof(tmpStr), "%d, ", 149 + ((bit_idx -33) * 4));
						strncat(bufStr,
								tmpStr,
								sizeof(bufStr) - strlen(bufStr) - 1);
					}
				}
			}
			if(bufStr[0] == '\0'){
				snprintf(bufStr, sizeof(bufStr), "N/A");
			}
			else{
				bufStr[strlen(bufStr) - 2] = '\0'; //Remove last comma and space character
			}
			lua_pushstring(L, bufStr);
			lua_setfield(L, -2, "Known Channels");

			lua_settable(L, -3);
			log_c(__LINE__, "[map_helper] %s SUCCESS  bufStr: %s\n\n", __FUNCTION__, bufStr);
		}
		lua_settable(L, -3);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int getDeviceRole(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int devRole;
	char devRoleStr[2] = {0};
	int ret;
	lua_newtable(L);
	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	log_c(__LINE__, "[map_helper] %s enter\n", __FUNCTION__);
	
	if(!ctrl){
		lua_pushstring(L, "mapDevRole");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}
	ret = mapd_interface_get_role(ctrl,&devRole);
	if(ret == -1){
		lua_pushstring(L, "mapDevRole");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command failed\n\n", __FUNCTION__);
	}
	else if(ret == -2){
		lua_pushstring(L, "mapDevRole");  /* push key */
		lua_pushstring(L, "Command Timed-out!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Timed-out\n\n", __FUNCTION__);
	}
	else{
		snprintf(devRoleStr, sizeof(devRoleStr), "%d", devRole);
		lua_pushstring(L, "mapDevRole");  /* push key */
		lua_pushstring(L, devRoleStr);  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s success devRole: %d\n\n", __FUNCTION__, devRole);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int getBhConnectionStatus(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	char conStatus[2] = {0};
	int ret;
	lua_newtable(L);
	log_c(__LINE__, "[map_helper] %s enter\n", __FUNCTION__);
	
	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}
	ret = mapd_interface_bh_ConnectionStatus(ctrl, conStatus);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == -2){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Timed-out!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Timed-out\n\n", __FUNCTION__);
	}
	else{
		conStatus[1] = '\0';
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);

		lua_pushstring(L, "conStatus");  /* push key */
		if((int)conStatus[0] == 1){
			lua_pushstring(L, "Connected");  /* push value */
		}
		else{
			lua_pushstring(L, "Disconnected");  /* push value */
		}
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS conStatus: %d \n\n", __FUNCTION__, conStatus[0]);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int applyApSteerRssiTh(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;
	const char *rssi = luaL_checkstring(L, 1);
	lua_newtable(L);
       log_c(__LINE__, "[map_helper] %s enter\n", __FUNCTION__);
	   
	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}

	ret = mapd_interface_set_rssi_thresh(ctrl, rssi);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == -2){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Timed-out!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Timed-out\n\n", __FUNCTION__);
	}
	else{
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS  \n\n", __FUNCTION__);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int applyChannelUtilizationTh(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;
	const char *channelUtilTh2G = luaL_checkstring(L, 1);
	const char *channelUtilTh5GL = luaL_checkstring(L, 2);
	const char *channelUtilTh5GH = luaL_checkstring(L, 3);
	lua_newtable(L);

	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}

	ret = mapd_interface_set_ChUtil_thresh(ctrl, channelUtilTh2G, channelUtilTh5GL, channelUtilTh5GH);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == -2){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Timed-out!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Timed-out\n\n", __FUNCTION__);
	}
	else{
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS  \n\n", __FUNCTION__);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int applyBssConfigRenew(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;
	lua_newtable(L);

	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}

	ret = mapd_interface_set_renew(ctrl);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == -2){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Timed-out!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Timed-out\n\n", __FUNCTION__);
	}
	else{
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS  \n\n", __FUNCTION__);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int getApBhInfList(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;
	/* MAX_VIF * MAX_BAND *SIZEOF(MAC_ADDRESS_STRING) + MAX_VIF_NUMBER_OF_SEMI_COLON + NULL CHARACTER
	* 16 * 3 * 17 + 16 + 1 = 833 Bytes. Rounding to 900 Bytes
	*/
	char mac_buf[900]={0};
	lua_newtable(L);

	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}

	ret = mapd_interface_get_bh_ap(ctrl, mac_buf);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == -2){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Timed-out!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Timed-out\n\n", __FUNCTION__);
	}
	else{
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		lua_pushstring(L, "macList");  /* push key */
		lua_pushstring(L, mac_buf);  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS  mac_buf: %s\n\n", __FUNCTION__, mac_buf);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int getApFhInfList(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;
	/* MAX_VIF * MAX_BAND *SIZEOF(MAC_ADDRESS_STRING) + MAX_VIF_NUMBER_OF_SEMI_COLON + NULL CHARACTER
	* 16 * 3 * 17 + 16 + 1 = 833 Bytes. Rounding to 900 Bytes
	*/
	char mac_buf[900]={0};
	lua_newtable(L);

	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}

	ret = mapd_interface_get_fh_ap(ctrl, mac_buf);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == -2){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Timed-out!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Timed-out\n\n", __FUNCTION__);
	}
	else{
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		lua_pushstring(L, "macList");  /* push key */
		lua_pushstring(L, mac_buf);  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS  mac_buf: %s\n\n", __FUNCTION__, mac_buf);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}


int applyWifiBhPriority(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret = -1;
	const char *BhPriority2G = luaL_checkstring(L, 1);
	const char *BhPriority5GL = luaL_checkstring(L, 2);
	const char *BhPriority5GH = luaL_checkstring(L, 3);

	lua_newtable(L);
    log_c(__LINE__, "[map_helper] %s enter\n", __FUNCTION__);
	   
	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}

	ret = mapd_interface_set_bh_priority(ctrl, BhPriority2G, BhPriority5GL, BhPriority5GH);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == -2){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Timed-out!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Timed-out\n\n", __FUNCTION__);
	}
	else{
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS  \n\n", __FUNCTION__);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int applyForceChSwitch(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;
	const char *almac = luaL_checkstring(L, 1);
	const char *channel1 = luaL_checkstring(L, 2);
	const char *channel2 = luaL_checkstring(L, 3);
	const char *channel3 = luaL_checkstring(L, 4);
	lua_newtable(L);
  log_c(__LINE__, "[map_helper] %s enter\n", __FUNCTION__);
	   
	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}

	ret = mapd_interface_forceChSwitch(ctrl, almac, channel1, channel2, channel3);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == -2){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Timed-out!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Timed-out\n\n", __FUNCTION__);
	}
	else{
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS  \n\n", __FUNCTION__);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int applyUserPreChannel(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;
	const char *channel = luaL_checkstring(L, 1);
	lua_newtable(L);
  log_c(__LINE__, "[map_helper] %s enter\n", __FUNCTION__);
	   
	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}

	ret = mapd_interface_user_preferred_channel(ctrl, channel);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == -2){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Timed-out!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Timed-out\n\n", __FUNCTION__);
	}
	else{
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS  \n\n", __FUNCTION__);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int triggerChPlanningR2(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;
	const char *band = luaL_checkstring(L, 1);
	lua_newtable(L);

	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}

	ret = mapd_interface_trigger_ch_plan_R2(ctrl, band);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == -2){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Timed-out!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Timed-out\n\n", __FUNCTION__);
	}
	else if(ret == -3){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Try again!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Try Again\n\n", __FUNCTION__);
	}
	else{
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS  \n\n", __FUNCTION__);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int triggerChannelScan(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;
	const char *almac = luaL_checkstring(L, 1);
	lua_newtable(L);
	log_c(__LINE__, "[map_helper] %s enter\n", __FUNCTION__);

	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}

	ret = mapd_interface_trigger_ch_scan(ctrl, almac);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == -2){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Timed-out!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Timed-out\n\n", __FUNCTION__);
	}
	else if(ret == -3){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Try again!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Try Again\n\n", __FUNCTION__);
	}
	else{
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS  \n\n", __FUNCTION__);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int getChannelStats(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;
	char bufStr[128] = {0};
	char channel_stats_buf[24576] = {0};
	size_t buf_len = 24576;
	lua_newtable(L);
	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	log_c(__LINE__, "[map_helper] %s enter\n", __FUNCTION__);
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}
	ret = mapd_interface_get_ch_scan_dump(ctrl, channel_stats_buf, &buf_len, NULL);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == 0){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		lua_pushstring(L, "luaChannelStatsInfo");  /* push key */
		lua_pushstring(L, channel_stats_buf);  /* push value */
		lua_settable(L, -3);
		snprintf(bufStr, sizeof(bufStr), "%zu",buf_len);
		lua_pushstring(L, "luaChannelStatsInfoLen");  /* push key */
		lua_pushstring(L, bufStr);  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS: %s \n\n", __FUNCTION__, channel_stats_buf);
	}
	else{
		snprintf(bufStr, sizeof(bufStr), "Error: %d",ret);
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, bufStr);  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s fail  error: %d \n\n", __FUNCTION__, ret);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}

int getChannelPlanningScore(lua_State *L)
{
	struct mapd_interface_ctrl *ctrl = NULL;
	int ret;
	char bufStr[128] = {0};
	char channel_planning_buf[10240] = {0};
	size_t buf_len = 10240;
	lua_newtable(L);
	ctrl = mapd_interface_ctrl_open("/tmp/mapd_ctrl");
	log_c(__LINE__, "[map_helper] %s enter\n", __FUNCTION__);
	if(!ctrl){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "mapd_interface_ctrl_open failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s mapd_interface_ctrl_open failed\n\n", __FUNCTION__);
		return 1;
	}
	ret = mapd_interface_get_ch_score_dump(ctrl, channel_planning_buf, &buf_len, NULL);
	if(ret == -1){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "Command Failed!");  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s Command Failed\n\n", __FUNCTION__);
	}
	else if(ret == 0){
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, "SUCCESS");  /* push value */
		lua_settable(L, -3);
		lua_pushstring(L, "luaChannelPlanningInfo");  /* push key */
		lua_pushstring(L, channel_planning_buf);  /* push value */
		lua_settable(L, -3);
		snprintf(bufStr, sizeof(bufStr), "%zu",buf_len);
		lua_pushstring(L, "luaChannelPlanningInfoLen");  /* push key */
		lua_pushstring(L, bufStr);  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s SUCCESS: %s \n\n", __FUNCTION__, channel_planning_buf);
	}
	else{
		snprintf(bufStr, sizeof(bufStr), "Error: %d",ret);
		lua_pushstring(L, "status");  /* push key */
		lua_pushstring(L, bufStr);  /* push value */
		lua_settable(L, -3);
		log_c(__LINE__, "[map_helper] %s fail  error: %d \n\n", __FUNCTION__, ret);
	}
	mapd_interface_ctrl_close(ctrl);
	return 1;
}
