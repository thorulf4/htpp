async function update(){
    const response = await fetch('/api/time');
    const myJson = await response.json(); //extract JSON from the http response
    
    let span = document.getElementById("time");
    span.textContent = myJson["hour"] + ':' + myJson["minute"] + ':' + myJson["second"];
}