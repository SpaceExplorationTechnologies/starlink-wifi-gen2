--This file is created for check some deamons like miniupnpd,8021xd...

    local mtkwifi = require("mtkwifi")
    local devs = mtkwifi.get_all_devs()
    local nixio = require("nixio")

function miniupnpd_chk(devname,vif,enable)
    local WAN_IF=mtkwifi.__trim(mtkwifi.read_pipe("uci -q get network.wan.ifname"))

    os.execute("rm -rf /etc/miniupnpd.conf")

    if mtkwifi.exists("/tmp/run/miniupnpd."..vif) then
        os.execute("cat /tmp/run/miniupnpd."..vif.." | xargs kill -9")
    end

    if enable then
        local profile = mtkwifi.search_dev_and_profile()[devname]
        local cfgs = mtkwifi.load_profile(profile)
        local ssid_index = devs[devname]["vifs"][vif].vifidx
        local wsc_conf_mode = ""
        local PORT_NUM = 7777+(string.byte(vif, -1)+string.byte(vif, -2))
        local LAN_IPADDR = mtkwifi.__trim(mtkwifi.read_pipe("uci -q get network.lan.ipaddr"))
        local LAN_MASK = mtkwifi.__trim(mtkwifi.read_pipe("uci -q get network.lan.netmask"))
        local port = 6352 + (string.byte(vif, -1)+string.byte(vif, -2))
        LAN_IPADDR = LAN_IPADDR.."/"..LAN_MASK
        wsc_conf_mode = mtkwifi.token_get(cfgs["WscConfMode"], ssid_index, "")

        local file = io.open("/etc/miniupnpd.conf", "w")
        if nil == file then
            nixio.syslog("debug","open file /etc/miniupnpd.conf fail")
        end

        file:write("ext_ifname=",WAN_IF,'\n','\n',
                   "listening_ip=",LAN_IPADDR,'\n','\n',
                   "port=",port,'\n','\n',
                   "bitrate_up=800000000",'\n',
                   "bitrate_down=800000000",'\n','\n',
                   "secure_mode=no",'\n','\n',
                   "system_uptime=yes",'\n','\n',
                   "notify_interval=30",'\n','\n',
                   "uuid=68555350-3352-3883-2883-335030522880",'\n','\n',
                   "serial=12345678",'\n','\n',
                   "model_number=1",'\n','\n',
                   "enable_upnp=no",'\n','\n')
        file:close()

        if wsc_conf_mode ~= "" and wsc_conf_mode ~= "0" then
            os.execute("miniupnpd -m 1 -I "..vif.." -P /var/run/miniupnpd."..vif.." -G -i "..WAN_IF.." -a "..LAN_IPADDR.." -n "..PORT_NUM)
        end
    end
end

function d8021xd_chk(devname, prefix, vif, enable)
    if mtkwifi.exists("/tmp/run/8021xd_"..vif..".pid") then
        os.execute("cat /tmp/run/8021xd_"..vif..".pid | xargs kill -9")
        os.execute("rm /tmp/run/8021xd_"..vif..".pid")
    end

    if enable then
        local profile = mtkwifi.search_dev_and_profile()[devname]
        local cfgs = mtkwifi.load_profile(profile)
        local auth_mode = cfgs.AuthMode
        local ieee8021x = cfgs.IEEE8021X
        local pat_auth_mode = {"WPA$", "WPA;", "WPA2$", "WPA2;", "WPA1WPA2$", "WPA1WPA2;", "WPA3$", "WPA3;", "192$", "192;"}
        local pat_ieee8021x = {"1$", "1;"}
        local apd_en = false

        for _, pat in ipairs(pat_auth_mode) do
            if string.find(auth_mode, pat) then
                apd_en = true
            end
        end

        for _, pat in ipairs(pat_ieee8021x) do
            if string.find(ieee8021x, pat) then
                apd_en = true
            end
        end

        if apd_en then
            os.execute("8021xd -p "..prefix.. " -i "..vif)
        end
    end
end

-- wifi service that require to start after wifi up
function wifi_service_misc()
    -- 1.Wapp
    if mtkwifi.exists("/usr/bin/wapp_openwrt.sh") then
        os.execute("/usr/bin/wapp_openwrt.sh")
    end

    -- 2.EasyMesh
    if mtkwifi.exists("/usr/bin/EasyMesh_openwrt.sh") then
        os.execute("/usr/bin/EasyMesh_openwrt.sh")
    end
end
