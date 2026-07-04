const statusText=document.getElementById("statusText");

function send(cmd){

fetch("/"+cmd);

statusText.innerHTML=cmd.toUpperCase();

}

document.getElementById("forward").onclick=()=>send("forward");

document.getElementById("backward").onclick=()=>send("backward");

document.getElementById("left").onclick=()=>send("left");

document.getElementById("right").onclick=()=>send("right");

document.getElementById("stop").onclick=()=>send("stop");