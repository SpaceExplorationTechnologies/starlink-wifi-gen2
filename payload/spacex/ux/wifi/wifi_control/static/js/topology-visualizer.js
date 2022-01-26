// TODO(SATSW-24742): Remove once app supports mesh. This is purely for convenient debugging of the mesh network topology.
var max_cellspan = 2;

function set_max_cellspan(o)
{
    for(var k in o){
        var v = o[k];
        if (v instanceof Array){
            max_cellspan++;
            for(var a_idx=0; a_idx < v.length; a_idx++){
                if(((typeof v[a_idx]) == "object") && v[a_idx] != null){
                    set_max_cellspan(v[a_idx]);
                }
                else{
                    console.log("set_max_cellspan: Incorrect Topology: Array element is not an object!",v[a_idx]);
                }
            }
        }
        else if(((typeof v) == "object") && v != null){
            set_max_cellspan(v);
        }
    }
}

function set_rowspan(o)
{
    var rowspan = 0;
    for(var k in o){
        var v = o[k];
        if (v instanceof Array){
            for(var a_idx=0; a_idx < v.length; a_idx++){
                if(((typeof v[a_idx]) == "object") && v[a_idx] != null){
                    rowspan = rowspan + set_rowspan(v[a_idx]);
                }
                else{
                    console.log("set_rowspan: Incorrect Topology: Array element is not an object!",v[a_idx]);
                }
            }
        }
        else if(((typeof v) == "object") && v != null){
            rowspan = rowspan + set_rowspan(v);
        }
        else{
            rowspan++;
        }
    }
    return rowspan;
}

function obj_loop(table_id, row, o)
{
    var cell;
    for(var k in o){
        var v = o[k];
        if (v instanceof Array){
            arr_loop(table_id, k, v);
        }
        else if(((typeof v) == "object") && v != null){
            obj_loop(table_id, row, v);
        }
        else{
            if(row == null){
                row = table_id.insertRow(-1);
            }
            cell = row.insertCell(-1);
            cell.innerHTML = k;
            if(k == "Device role"){
                v = get_dev_role_string(v);
            }
            cell = row.insertCell(-1);
            cell.colSpan = max_cellspan - row.cells.length + 1;
            cell.innerHTML = v;
            row = null;
        }
    }
}

function arr_loop(table_id, k, a)
{
    var row, cell;
    for(var a_idx=0; a_idx < a.length; a_idx++){
        var v = a[a_idx];
        if(((typeof v) == "object") && v != null){
            row = table_id.insertRow(-1);
            cell = row.insertCell(-1);
            cell.rowSpan = set_rowspan(v);
            cell.style.verticalAlign  = "middle";
            cell.innerHTML = "<strong>" + k + ' - ' + (a_idx + 1) + "</strong>";
            obj_loop(table_id, row, v);
        }
        else{
            console.log("Incorrect Topology: Array element is not an object!",a);
        }
    }
}

function draw_topology_table(jsTopoInfo)
{
    var tree_info = [];
    var topoInfoArr = jsTopoInfo['topology information'];
    if(!(topoInfoArr instanceof Array)){
        console.log("Incorrect TopologyInfo: Value of topology information is not an Array!");
        return;
    }

    for(var idx_1905=0; idx_1905 < topoInfoArr.length; idx_1905++){
        var obj_1905 = topoInfoArr[idx_1905];
        var dev1905tbl = document.createElement("TABLE");
        dev1905tbl.setAttribute("class", "cbi-section-table");
        var dev1905fset = document.createElement("FIELDSET");
        dev1905fset.setAttribute("class", "cbi-section");
        var dev1905legend = document.createElement("LEGEND");
        var dev1905a = document.createElement("A");
        var dev1905text = document.createTextNode("1905 Device - " + (idx_1905 + 1));
        dev1905a.setAttribute("href", "#");
        dev1905a.appendChild(dev1905text);
        dev1905legend.appendChild(dev1905a);
        dev1905fset.appendChild(dev1905legend);
        document.getElementById("display_all_1905_dev_info_div").appendChild(dev1905fset);

        set_max_cellspan(obj_1905);
        obj_loop(dev1905tbl, null, obj_1905);
        tree_info[idx_1905] = prep_tree_info(obj_1905);

        document.getElementById("display_all_1905_dev_info_div").appendChild(dev1905tbl);
    }
    tree_info.sort(function(a, b){return a.hopCount - b.hopCount});
    disp_topology_vis(tree_info);
}

function prep_tree_info(dev1905Obj)
{
    var node = {};
    node["devRole"] = get_dev_role_string(dev1905Obj['Device role']);
    node["alMac"] = dev1905Obj['AL MAC'];
    node["hopCount"] = parseInt(dev1905Obj['Distance from controller']);
    node['UplinkAlMac'] = dev1905Obj['Upstream 1905 device'];
    node["wifiStaInfo"] = {};
    node["ethStaInfo"] = {};
    node["bhAgentMedium"] = ["Not Present"];
    node["radioChannelInfo"] = [];
    for(var radIdx=0; radIdx < dev1905Obj['Radio Info'].length; radIdx++){
        var channel = dev1905Obj['Radio Info'][radIdx]['channel'];
        node["radioChannelInfo"].push(channel);
        for(var bssIdx=0; bssIdx < dev1905Obj['Radio Info'][radIdx]['BSSINFO'].length; bssIdx++){
            if(dev1905Obj['Radio Info'][radIdx]['BSSINFO'][bssIdx].hasOwnProperty('connected sta info')){
                for(var staIdx=0; staIdx < dev1905Obj['Radio Info'][radIdx]['BSSINFO'][bssIdx]["connected sta info"].length; staIdx++){
                    var ssid = dev1905Obj['Radio Info'][radIdx]['BSSINFO'][bssIdx]['SSID'];
                    var staMac = dev1905Obj['Radio Info'][radIdx]['BSSINFO'][bssIdx]["connected sta info"][staIdx]['STA MAC address'];
                    var bhSta = dev1905Obj['Radio Info'][radIdx]['BSSINFO'][bssIdx]["connected sta info"][staIdx]['BH STA'];
                    if (node["wifiStaInfo"].hasOwnProperty(ssid))
                        node["wifiStaInfo"][ssid].push({"Sta MAC": staMac, "BH Sta": bhSta});
                    else
                        node["wifiStaInfo"][ssid] = Array({"Sta MAC": staMac, "BH Sta": bhSta});
                }
            }
        }
    }

    for (var cliIdx=0; cliIdx < dev1905Obj['Other Clients Info'].length; cliIdx++){
        if (dev1905Obj['Other Clients Info'][cliIdx].hasOwnProperty('Client Address')){
            var cliAdd = dev1905Obj['Other Clients Info'][cliIdx]['Client Address'];
            var medium = dev1905Obj['Other Clients Info'][cliIdx]['Medium'];
            if (node["ethStaInfo"].hasOwnProperty(medium))
                node["ethStaInfo"][medium].push(cliAdd);
            else
                node["ethStaInfo"][medium] = Array(cliAdd);
        }
    }

    for (var bhIdx=0; bhIdx < dev1905Obj['BH Info'].length; bhIdx++){
            var bhMedium = dev1905Obj['BH Info'][bhIdx]['Backhaul Medium Type']
            if (node["bhAgentMedium"] == null || node["bhAgentMedium"] == ""){
                node["bhAgentMedium"] = ["Not Present"];
            }
            else{
                node["bhAgentMedium"].push(bhMedium);
            }
    }

    node["bhAgentMedium"].shift();

    return node;
}

function get_uplink_tree_info(tree_info, mac)
{
    for(var nIdx=0; nIdx < tree_info.length; nIdx++){
        if(tree_info[nIdx]['alMac'] == mac){
            if (tree_info[nIdx]['devRole'] == "Agent")
                return '"' + tree_info[nIdx]['devRole'] + '\n' + "AL_MAC" + ' - ' + tree_info[nIdx]['alMac'] + '\n' + "Medium - " + tree_info[nIdx]['bhAgentMedium'] + '\n' + "Channel Number - " + tree_info[nIdx]['radioChannelInfo'].join('/') + '"';
            else
                return '"' + tree_info[nIdx]['devRole'] + '\n' + "AL_MAC" + ' - ' + tree_info[nIdx]['alMac'] + '\n' + "Channel Number - " + tree_info[nIdx]['radioChannelInfo'].join('/') + '"';
        }
    }
    return "";
}

var tempStr = "";
var topoFlag = false

function disp_topology_vis(tree_info)
{
    var div = document.getElementById("display_tree_vis");
    div.style.display = "";
    var topoErrMsg = document.getElementById("topoErrMsgDiv");
    topoErrMsg.innerHTML = "";
    var str = "";
    var numStaPerDev = 0;
    console.log("TREE INFO VIS = ",tree_info);
    for(var nIdx=0; nIdx < tree_info.length; nIdx++){
        if (tree_info[nIdx]['devRole'] == "Agent"){
            var upLinkNodeStr = get_uplink_tree_info(tree_info, tree_info[nIdx]['UplinkAlMac']);
            if(upLinkNodeStr != ""){
                str += '"' + tree_info[nIdx]['devRole'] + '\n' + "AL_MAC" + ' - ' + tree_info[nIdx]['alMac'] + '\n' + "Medium - " + tree_info[nIdx]['bhAgentMedium'] + '\n' + "Channel Number - " + tree_info[nIdx]['radioChannelInfo'].join('/') + '"' + '->' + upLinkNodeStr + " ";
            }
            else{
                console.log("INFO: AGENT WITH NO UP-LINK MAC = ",tree_info[nIdx]['alMac']);
                topoErrMsg.innerHTML += "<br />ERROR: Agent with AL-MAC " + tree_info[nIdx]['alMac'] +" has no upstream 1905 device!";
                topoErrMsg.style.display = "";
            }
        }
        for(var ssid in tree_info[nIdx]['wifiStaInfo']){
            var staIdx;
            for(var staIdx=0; staIdx < tree_info[nIdx]['wifiStaInfo'][ssid].length; staIdx++){
                var mac1stOctet = parseInt(tree_info[nIdx]['wifiStaInfo'][ssid][staIdx]["Sta MAC"].substring(0,2),16);
                var bhStaChk = tree_info[nIdx]['wifiStaInfo'][ssid][staIdx]["BH Sta"];
                if((mac1stOctet & 0x02) && (bhStaChk == "Yes")){
                    continue;
                }
                if (tree_info[nIdx]['devRole'] == "Agent")
                    str += '"STA\nMAC - ' + tree_info[nIdx]['wifiStaInfo'][ssid][staIdx]["Sta MAC"] + '\nSSID - ' + ssid + '"' + '->' + '"' + tree_info[nIdx]['devRole'] + '\n' + "AL_MAC" + ' - ' + tree_info[nIdx]['alMac'] + '\n' + "Medium - " + tree_info[nIdx]['bhAgentMedium'] + '\n' + "Channel Number - " + tree_info[nIdx]['radioChannelInfo'].join('/') + '"' + " ";
                else
                    str += '"STA\nMAC - ' + tree_info[nIdx]['wifiStaInfo'][ssid][staIdx]["Sta MAC"] + '\nSSID - ' + ssid + '"' + '->' + '"' + tree_info[nIdx]['devRole'] + '\n' + "AL_MAC" + ' - ' + tree_info[nIdx]['alMac'] + '\n' + "Channel Number - " + tree_info[nIdx]['radioChannelInfo'].join('/') + '"' + " ";
                numStaPerDev += 1;
            }
        }

        for(var medium in tree_info[nIdx]['ethStaInfo']){
            var ethStaIdx;
            for(var ethStaIdx=0; ethStaIdx < tree_info[nIdx]['ethStaInfo'][medium].length; ethStaIdx++){
                var mac1stOctet = parseInt(tree_info[nIdx]['ethStaInfo'][medium][ethStaIdx].substring(0,2),16);
                if(mac1stOctet & 0x02){
                    continue;
                }
                if (tree_info[nIdx]['devRole'] == "Agent")
                    str += '"STA\nMAC - ' + tree_info[nIdx]['ethStaInfo'][medium][ethStaIdx] + '\nMedium - ' + medium + '"' + '->' + '"' + tree_info[nIdx]['devRole'] + '\n' + "AL_MAC" + ' - ' + tree_info[nIdx]['alMac'] + '\n' + "Medium - " + tree_info[nIdx]['bhAgentMedium'] + '\n' + "Channel Number - " + tree_info[nIdx]['radioChannelInfo'].join('/') + '"' + " ";
                else
                    str += '"STA\nMAC - ' + tree_info[nIdx]['ethStaInfo'][medium][ethStaIdx] + '\nMedium - ' + medium + '"' + '->' + '"' + tree_info[nIdx]['devRole'] + '\n' + "AL_MAC" + ' - ' + tree_info[nIdx]['alMac'] + '\n' + "Channel Number - " + tree_info[nIdx]['radioChannelInfo'].join('/') + '"' + " ";
                numStaPerDev += 1;
            }
        }

        if (numStaPerDev == 0 && tree_info.length == 1 &&
            (tree_info[nIdx]['devRole'] == "Controller" ||
            tree_info[nIdx]['devRole'] == "Agent")){
            str += '"' + tree_info[nIdx]['devRole'] + '\n' + 'AL_MAC' + ' - ' + tree_info[nIdx]['alMac'] + '"' + " ";
        }

        /* Uncomment below if connected STA bug is observed in mapd. */
        /*
        if (tree_info.length == 1 &&
            (tree_info[nIdx]['devRole'] == "Controller" ||
            tree_info[nIdx]['devRole'] == "Agent")){
            str += '"' + tree_info[nIdx]['devRole'] + '\n' + 'AL_MAC' + ' - ' + tree_info[nIdx]['alMac'] + '"' + " ";
        }
        */
    }
    console.log("TREE INFO str = ",str);

    if (tempStr !== str){
        tempStr = str;
        topoFlag = true;
    }
    else {
        topoFlag = false;
        return;
    }

    var NewDOTstring = "dinetwork{"+str+"}";
    var parsedData = vis.network.convertDot(NewDOTstring);

    for(var count=0;count<parsedData.nodes.length;count++){
        console.log("NODE LABEL = ",parsedData.nodes[count].label);
        if(parsedData.nodes[count].label.startsWith('Agent')){
            parsedData.nodes[count].color='Orange';
        }
        else if(parsedData.nodes[count].label.startsWith('STA')){
            parsedData.nodes[count].color='Green';
        }
        else{
            parsedData.nodes[count].shape='box';
        }
    }
    var data = {
    nodes: parsedData.nodes,
    edges: parsedData.edges
    }
    var options = parsedData.options;

    // create a network
    var network = new vis.Network(div, data, options);
    const visualizerScale = 0.8;
    network.setSize(window.innerWidth, window.innerHeight * visualizerScale);

    // <% if mapcfgs.map_ver ~= "R1" then %>
    //     var alMac;
    //     network.on('click',function(params){
    //         if (!params.nodes[0].includes('STA')){
    //             alMac = params.nodes[0].match(/[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}/)[0];
    //             XHR.get('<%=luci.dispatcher.build_url("admin", "mtk", "multi_ap", "trigger_de_dump")%>' + '/' + alMac, null,
    //                 function(x)
    //                 {
    //                     console.log(x);
    //                     try{
    //                         var rsp = JSON.parse(x.response);
    //                         if(rsp.status == "SUCCESS"){
    //                             location.href = '<%=luci.dispatcher.build_url("admin", "mtk", "multi_ap", "display_data_element")%>'+'/'+alMac;
    //                         }
    //                         else{
    //                             alert("Failed to trigger Data Element Dump!\nStatus: ",rsp.status);
    //                         }
    //                     }
    //                     catch(e){
    //                         alert("Incorrect response!\nFailed to trigger Data Element Dump!");
    //                     }
    //                 }
    //             );
    //         }
    //     })
    // <% end %>
}

function toggle_disp_dev_table(btn)
{
    var div = document.getElementById("display_all_1905_dev_info_div");
    var btn_str = btn.innerHTML;
    if(btn_str.startsWith('Show')){
        btn.innerHTML = 'Hide all device info';
        div.style.display = "";
        if(div.children.length == 0){
            div.innerHTML = "";
            var b = document.createElement("BIG");
            var s = document.createElement("STRONG");
            var t = document.createTextNode("Run-time topology information has not been received yet! Please wait.");
            s.appendChild(t);
            b.appendChild(s);
            div.setAttribute("class", "alert-message");
            div.appendChild(b);
        }
    }
    else{
        btn.innerHTML = 'Show all device info';
        div.style.display = "none";
    }
}

function get_dev_role_string(devRole)
{
    switch(parseInt(devRole)){
        case 1:
            return 'Controller';
        case 2:
            return 'Agent';
        default:
            return 'Not Configured';
    }
}

function get_run_time_topology_cb(rsp)
{
    try{
        var r = JSON.parse(rsp);
        console.log(r, rsp)
        if(r.status == "SUCCESS"){
            // var jsTopoInfo = JSON.parse(r.luaTopologyInfo);
            // console.log("PP", r["topology information"])
            // var jsTopoInfo = JSON.parse(r["topology information"]);
            // console.log("P", r["topology information"])
            document.getElementById("display_all_1905_dev_info_div").innerHTML = "";
            draw_topology_table(r);
        }
        else{
            console.log("Failed to get Topology Info!\nStatus: ",r.status);
        }
        // window.setTimeout(get_run_time_topology, 15000);
    }
    catch(e){
        console.log("Incorrect response! Failed to get Topology Info!",e.name,e.message);
        // window.setTimeout(get_run_time_topology, 15000);
    }
}

function get_run_time_topology(index)
{
    document.getElementById("topoErrMsgDiv").style.display = "none";
    document.getElementById("display_all_1905_dev_info_div").removeAttribute("class");

    get_run_time_topology_cb(read_topology_from_file(index))
}

function read_topology_from_file(index) {
    const topo = topoJSON[Math.min(Math.max(0, index), topoJSON.length - 1)]
    topo.status = "SUCCESS";
    return JSON.stringify(topo);
}

const visualizationRadioButtons = document.querySelectorAll("input[name='visualizer-choice']");

for (const radio of visualizationRadioButtons) {
    radio.onclick = function() {
        if (radio.checked) {
            const visualizerContainers = document.querySelectorAll(".visualizer-container");
            for (const visualizerContainer of visualizerContainers) {
                visualizerContainer.style.display = "none"
            }
            const selectedVisualizerContainer = document.querySelector(".visualizer-container[value=" + radio.value + "]");
            selectedVisualizerContainer.style.display = "block";
        }
    }
}

window.onload = function(){
    get_run_time_topology(topoJSON.length - 1);
}