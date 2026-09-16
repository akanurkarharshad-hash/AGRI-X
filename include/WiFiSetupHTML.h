#pragma once

const char wifiSetupHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>

<meta charset="UTF-8">

<meta name="viewport" content="width=device-width, initial-scale=1.0">

<title>AGRI-X WiFi Setup</title>

<style>

body
{
    margin:0;
    padding:0;
    background:#0f172a;
    color:white;
    font-family:Arial,Helvetica,sans-serif;
}

.container
{
    width:90%;
    max-width:420px;
    margin:60px auto;
    background:#1e293b;
    border-radius:15px;
    padding:25px;
    box-shadow:0 0 20px rgba(0,0,0,.35);
}

h1
{
    text-align:center;
    margin-bottom:10px;
}

p
{
    text-align:center;
    color:#cbd5e1;
}

input
{
    width:100%;
    padding:14px;
    margin-top:15px;
    border:none;
    border-radius:8px;
    font-size:16px;
    box-sizing:border-box;
}

button
{
    width:100%;
    margin-top:20px;
    padding:15px;
    border:none;
    border-radius:8px;
    background:#16a34a;
    color:white;
    font-size:17px;
    cursor:pointer;
}

button:hover
{
    background:#15803d;
}

#status
{
    margin-top:20px;
    text-align:center;
    color:#facc15;
}

</style>

</head>

<body>

<div class="container">

<h1>🚜 AGRI-X</h1>

<p>WiFi Configuration</p>

<input
id="ssid"
type="text"
placeholder="WiFi Name">

<input
id="password"
type="password"
placeholder="WiFi Password">

<button onclick="saveWiFi()">
Connect
</button>

<div id="status"></div>

</div>

<script>

async function saveWiFi()
{
    const ssid=document.getElementById("ssid").value;
    const password=document.getElementById("password").value;

    if(ssid==="")
    {
        alert("Enter WiFi Name");
        return;
    }

    document.getElementById("status").innerHTML="Saving...";

    const response=await fetch(
        "/savewifi",
        {
            method:"POST",
            headers:
            {
                "Content-Type":"application/json"
            },
            body:JSON.stringify(
            {
                ssid:ssid,
                password:password
            })
        });

    const text=await response.text();

    document.getElementById("status").innerHTML=text;
}

</script>

</body>

</html>
)rawliteral";