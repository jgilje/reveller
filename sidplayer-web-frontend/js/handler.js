
let path = ""
let lastpath = ""
var conn;
let intervalID
let debug = false;
$(function() {
var msg = $("#msg");
var log = $("#log");
let curSong = ""

getPath = {
"action": "ls", 
"argument": path
}

getState = {
    "action": "state"
}

getHeaders = {
    "action": "currentHeader"
}

// ("ws://{{$}}:8080/ws")
//conn = new WebSocket("ws://{{$}}/ws");
if (window["WebSocket"]) {
    reCon()

} else {
    console.log("<div><b>Your browser does not support WebSockets.</b></div>")
}


});

const textbox = document.getElementById("inputSearchstring");
addEventListener("keydown", function onEvent(event) {
    if (event.key === "Enter") {
        if (debug) {
            console.log("bitch yo")
        }
        
        document.getElementById("SearchButton").click();
    }
});

function reCon(state){
    let ws_url = ""
    if (location.protocol === "http:")
    {
        ws_url = "ws://" + window.location.host + "/ws";
    }
    else if (location.protocol === "https:")
    {
        ws_url = "wss://" + window.location.host + "/ws";
    }
    else if (location.protocol === "file:"){
        ws_url = "ws://" + "10.6.67.175:8080/ws";
    }
    console.log(conn)
    if (state == "recon"){
        if (typeof conn != "undefined" ){
            if (conn.readyState != 1 ){
                if (debug) {
                    console.log("trying to reconnect")  
                }
                conn = new WebSocket(ws_url);
            }
            else if (conn.readyState == 1){
                if (debug) {
                    console.log("We have reconnected...")
                }
                clearInterval(intervalID);
            }
        }
    }
    else {
    conn = new WebSocket(ws_url);
    clearInterval(intervalID);
    }
    conn.onopen = function(evt) {
        clearInterval(intervalID);
        conn.send(JSON.stringify(getPath));
        conn.send(JSON.stringify(getState));
        conn.send(JSON.stringify(getHeaders));
        document.getElementById("reveller").innerText = "Reveller";
        
    }

    conn.onclose = function(evt) {
        document.getElementById("reveller").innerText = "Disconnected";
        intervalID = setInterval(reCon("recon"),1000);
    }
    conn.onmessage = function(evt) {
        if (debug) {
            console.log(evt.data)    
        }
        msgParse(evt.data)
    }
}

function Search() {
    let search = document.getElementById("inputSearchstring").value
    if (debug) {
        console.log(search)
    }
    command = {
        "action": "search",
        "argument": search//filename
    }
    if (debug) {
        console.log(command)
    }
    conn.send(JSON.stringify(command))
}


function home() {
    command = {
        "action": "ls",
        "argument": "/"
    }
    conn.send(JSON.stringify(command))
}

function backDir() {  
    if (!completepath) {
        home()
    }
    lastpath = completepath.split("/")
    let lastdir = ""
    if (debug) {
        console.log("lastpath is: " + lastpath)
    }
    
    for (let i = 0; i < lastpath.length -1; i++){
        if (i > 0 ){
            lastdir += "/" + lastpath[i]
        }
        else {
            lastdir += lastpath[i]
        }
        
    }

    completepath = lastdir

    if (debug) {
        console.log (lastdir)
    }
    command = {
        "action": "ls",
        "argument": lastdir
    }
    if (debug) {
        console.log(command)
    }
    conn.send(JSON.stringify(command))
};

function loadSong(songName, yo = false) {
    if (yo == true) {
        arg = {
            "action": "load",
            "argument": songName
        }
        curSong = "1"
        conn.send(JSON.stringify(arg));
    }
    else {
        arg = {
            "action": "load",
            "argument": completepath + "/" + songName
        }
        curSong = "1"
        conn.send(JSON.stringify(arg));
    }
}

function control(state){
    if (state != "true" && state != "false"){
        if (debug) {    
            console.log(state)
        }
        if (state == "fwd" || state == "prev"){
            fetchSong = {
                "action": "state"
            }
            conn.send(JSON.stringify(fetchSong))
    
            if (state == "fwd"){
                song = Number(curSong)+1
                arg = {
                    "action": "song",
                    "argument": song.toString()
                }
            }
            if (state == "prev"){
                song = Number(curSong)-1
                arg = {
                    "action": "song",
                    "argument": song.toString()
                }
            }
            

            conn.send(JSON.stringify(arg))
        }
        else if (state == "play"){
            arg = {
                "action": "play"
            }
            conn.send(JSON.stringify(arg))
        }
        else if (state == "stop"){
            arg = {
                "action": "stop"
            }
            conn.send(JSON.stringify(arg))
        }
    }
    else {
        if (debug) {
            console.log("lol")
            console.log(document.getElementById("power").textContent)
        }
        if(document.getElementById("power").textContent == "On"){
            document.getElementById("power").setAttribute("onclick", "control('false')");
            document.getElementById("power").className = "btn btn-danger btn-lg";
            document.getElementById("power").innerText = "Off"
        }
        else if(document.getElementById("power").textContent == "Off"){
            document.getElementById("power").setAttribute("onclick", "control('true')");
            document.getElementById("power").className = "btn btn-success btn-lg";
            document.getElementById("power").innerText = "On"
        }
        
        arg = {
            "action": "power",
            "argument": state
        }
        conn.send(JSON.stringify(arg))
    }
    
}


function enterDir(subpath) {
    
    completepath = path + "/" + subpath
    command = {
        "action": "ls",
        "argument": completepath
    }
    if (debug) {
        console.log(command)
    }
    conn.send(JSON.stringify(command))
};



function msgParse(msg) {
    jsonobj = JSON.parse(msg);
    if (debug) {
        console.log(jsonobj);
    }
    if(typeof jsonobj.data != "undefined" && jsonobj.type == "ls")
    {
        dir = JSON.parse(jsonobj.data);
        if (debug) {
            console.log(dir);
        }
        if(typeof dir.directories != "undefined"){
            if(dir.directories.length > 0){
                path = dir.path
                document.getElementById("directory").innerHTML = ""
                document.getElementById("directory").innerHTML += `
                    <tr onclick=backDir('../') class="dir">
                        <td><i class="bi bi-folder"></i> ../</td>
                    </tr>
                `
                dir.directories.forEach(element => {
                document.getElementById("directory").innerHTML += `
                    <tr onclick=enterDir('${ element }') class="dir">
                        <td><i class="bi bi-folder"></i> ${ element }</td>
                    </tr>`
                });
            }
            else {
                if (dir.sidfiles.length > 0) {
                    document.getElementById("directory").innerHTML = ""
                    document.getElementById("directory").innerHTML += `
                    <tr onclick=backDir('../') class="dir">
                        <td><i class="bi bi-folder"></i> ../</td>
                    </tr>
                    `
                    dir.sidfiles.forEach(element => {
                    if (debug) {
                        console.log(element)
                    }
                    document.getElementById("directory").innerHTML += `
                        <tr onclick=loadSong('${ element }') class="dir">
                            <td><i class="bi bi-folder"></i> ${ element }</td>
                        </tr>`
                    });
                }
            }
            
        }
    }
    else{
        if (jsonobj.type == "load"){
            control("play");
        }
        else if (jsonobj.type == "state" || jsonobj.type == "currentSidHeader"){
            
            dataobj = JSON.parse(jsonobj.data)
            if (debug) {
                console.log(dataobj)
            }
            if (jsonobj.type == "currentSidHeader"){
                document.getElementById('artist').textContent = dataobj.Author
                document.getElementById('song').textContent = dataobj.Name
                document.getElementById('subsongavail').textContent = dataobj.Songs
                
            }
            else if (jsonobj.type == 'state'){
                document.getElementById('currsubsong').textContent = dataobj.song
                curSong = dataobj.song
                if (dataobj.power == true){
                    if (debug) {
                        console.log("power is on")
                    }
                    document.getElementById("power").setAttribute("onclick", "control('false')");
                    document.getElementById("power").className = "btn btn-danger btn-lg";
                    document.getElementById("power").innerText = "Off"
                }
                else {
                    if (debug) {
                        console.log("power is off") 
                    }
                    document.getElementById("power").setAttribute("onclick", "control('true')");
                    document.getElementById("power").className = "btn btn-success btn-lg";
                    document.getElementById("power").innerText = "On"
                }
            }
        }
        else if (jsonobj.type == "search"){
            if (debug) {
            console.log(jsonobj.data)
            }
            searchres = JSON.parse(jsonobj.data)
            if (debug) {
                console.log(searchres.results)  
            }
            document.getElementById("directory").innerHTML = ""
            if (searchres.results == null){
                document.getElementById("directory").innerHTML = "<tr></tr><td>No results found</td>"
            }
            else{
                for (let i = 0; i < searchres.results.length; i++) {
                    // select last string of filename
                    let filePath = searchres.results[i];
                    let fileName = filePath.substring(filePath.lastIndexOf("/") + 1);
                    if (debug) {
                        console.log(fileName);
                    }
                    document.getElementById("directory").innerHTML += `
                        <tr onclick="loadSong('${searchres.results[i]}', true)" class="dir">
                            <td><i class="bi bi-folder"></i> ${fileName}</td>
                        </tr>`;
                }
            }
        }
    }
}
