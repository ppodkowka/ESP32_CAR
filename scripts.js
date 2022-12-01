    var webSocketCameraUrl = "ws:\/\/" + window.location.hostname + "/Camera";     
    var websocketCamera;

    function updateSpeedSlider(element) {
      var carSpeed = document.getElementById("speed").value; // gets current value of slider by its ID and saves it in "carSpeed" variable
      if(carSpeed == 190){
        document.getElementById("speedSliderValue").innerHTML = "LOW"; // sets label with id "speedSliderValue" with correct string acording to carSpeed
      }
      else if(carSpeed == 220){
        document.getElementById("speedSliderValue").innerHTML = "MEDIUM";
      }
      else if(carSpeed == 250){
        document.getElementById("speedSliderValue").innerHTML = "HIGH";
      }
      // http get request:
      var HttpRequest = new XMLHttpRequest();
      HttpRequest.open("GET", "/slider?value="+carSpeed, true);
      HttpRequest.send(); 
    } 

    function initCameraWebSocket(){
        websocketCamera = new WebSocket(webSocketCameraUrl);
        websocketCamera.binaryType = 'blob';
        websocketCamera.onopen    = function(event){};
        websocketCamera.onclose   = function(event){setTimeout(initCameraWebSocket, 2000);};
        websocketCamera.onmessage = function(event)
        {
          var imageId = document.getElementById("cameraImage");
          imageId.src = URL.createObjectURL(event.data);
        };
    }
    
    function initWebSocket(){
        initCameraWebSocket ();
    }
    
    window.onload = initWebSocket;
    document.getElementById("mainTable").addEventListener("touchend", function(event){
        event.preventDefault()
    }); 
