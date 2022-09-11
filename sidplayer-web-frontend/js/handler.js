
let path = ""
let lastpath = ""
var conn;
let intervalID

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

// ("ws://reveller.rottlan.fluxxx.lan:8080/ws")
//conn = new WebSocket("ws://{{$}}/ws");
if (window["WebSocket"]) {
    reCon()

} else {
    console.log("<div><b>Your browser does not support WebSockets.</b></div>")
}


});

function reCon(state){
    console.log(conn)
    if (state == "recon"){
        if (typeof conn != "undefined" ){
            if (conn.readyState != 1 ){
                console.log("trying to reconnect")  
                conn = new WebSocket("ws://reveller.rottlan.fluxxx.lan:8080/ws");
            }
            else if (conn.readyState == 1){
                console.log("We have reconnected...")
                clearInterval(intervalID);
            }
        }
    }
    else {
    conn = new WebSocket("ws://reveller.rottlan.fluxxx.lan:8080/ws");
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
        console.log(evt.data)
        msgParse(evt.data)
    }
    
}


function home() {
    command = {
        "action": "ls",
        "argument": "/"
    }
    conn.send(JSON.stringify(command))
}

function backDir() {  
    console.log(completepath)
    lastpath = completepath.split("/")
    let lastdir = ""
    console.log("lastpath is: " + lastpath)

    for (let i = 0; i < lastpath.length -1; i++){
        if (i > 0 ){
            lastdir += "/" + lastpath[i]
        }
        else {
            lastdir += lastpath[i]
        }
        
    }

    completepath = lastdir

    console.log (lastdir)
    command = {
        "action": "ls",
        "argument": lastdir
    }
    console.log(command)
    conn.send(JSON.stringify(command))
};

function loadSong(songName) {
    arg = {
        "action": "load",
        "argument": completepath + "/" + songName
    }
    curSong = "1"
    conn.send(JSON.stringify(arg));
}

function control(state){
    if (state != "true" && state != "false"){
        console.log(state)
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
        console.log("lol")
        console.log(document.getElementById("power").textContent)
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
    console.log(command)
    conn.send(JSON.stringify(command))
};



function msgParse(msg) {
    jsonobj = JSON.parse(msg);
    console.log(jsonobj);
    if(typeof jsonobj.data != "undefined" && jsonobj.type == "ls")
    {
        dir = JSON.parse(jsonobj.data);
        console.log(dir);
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
                    console.log(element)
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
            console.log(dataobj)
            if (jsonobj.type == "currentSidHeader"){
                document.getElementById('artist').textContent = dataobj.Author
                document.getElementById('song').textContent = dataobj.Name
                document.getElementById('subsongavail').textContent = dataobj.Songs
                
            }
            else if (jsonobj.type == 'state'){
                document.getElementById('currsubsong').textContent = dataobj.song
                curSong = dataobj.song
                if (dataobj.power == true){
                    console.log("power is on")
                    document.getElementById("power").setAttribute("onclick", "control('false')");
                    document.getElementById("power").className = "btn btn-danger btn-lg";
                    document.getElementById("power").innerText = "Off"
                }
                else {
                    console.log("power is off")
                    document.getElementById("power").setAttribute("onclick", "control('true')");
                    document.getElementById("power").className = "btn btn-success btn-lg";
                    document.getElementById("power").innerText = "On"
                }
            }
        }
    }
}
