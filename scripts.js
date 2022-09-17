function updateSpeedSlider(element) {
    var carSpeed = document.getElementById("speedSlider").value;
    document.getElementById("speedSliderValue").innerHTML = carSpeed;
    console.log(carSpeed);
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/slider?value="+carSpeed, true);
    xhr.send();
  }
