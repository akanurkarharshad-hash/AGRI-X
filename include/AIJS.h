#pragma once

const char aiJS[] PROGMEM = R"rawliteral(

async function updateAI()
{
    try
    {
        const response = await fetch("http://172.21.36.105:8000/latest");

        if (!response.ok)
            return;

        const data = await response.json();

        setValue("aiDisease", data.disease);
        setValue("aiConfidence", data.confidence + "%");
        setValue("aiSeverity", data.severity);
        setValue("aiRecommendation", data.recommendation);

        const img = document.getElementById("aiImage");

        if (img && data.image && data.image.length > 0)
        {
            img.src =
"http://http://172.21.36.105:8000/latest.jpg?t=" +
Date.now();
        }
    }
    catch(error)
    {
        console.error(error);
    }
}

updateAI();
setInterval(updateAI, 1000);

)rawliteral";