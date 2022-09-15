function updateSliderPWM(element) {
    var pwmSliderValue = document.getElementById("pwmSlider").value;
    document.getElementById("textSliderValue").innerHTML = pwmSliderValue;
    console.log(pwmSliderValue);
    var httpRequest = new XMLHttpRequest();
    httpRequest.open("GET", "/slider?value="+pwmSliderValue, true);
    httpRequest.send();
}
