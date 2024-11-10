//@input string WebsiteURL
//@input Asset.RemoteServiceModule remoteServiceModule

/** @type {RemoteServiceModule} */

var remoteServiceModule = script.remoteServiceModule;

function ParseJson(JsonObject)
{
  if (JsonObject)
  {
    JsonObject["id"];
    JsonObject["value"];
    JsonObject["accel"];
    JsonObject["gyro"];
    JsonObject["timestamp"];
    JsonObject["opts"];
  }
}


function makeRequest() {
  // Create a new HTTP request
  let httpRequest = RemoteServiceHttpRequest.create();
  // Set the URL for the request
  httpRequest.url = script.WebsiteURL; 
  // Set the HTTP method to GET
  httpRequest.method = RemoteServiceHttpRequest.HttpRequestMethod.Get; 

  // Perform the HTTP request
  remoteServiceModule.performHttpRequest(httpRequest, (response) => {
    if (response.statusCode === 200) {
      // Check if the response status is 200 (OK)
      print('Body: ' + response.body); 
      JsonObject = JSON.parse(response.body);
      ParseJson(JsonObject);
    }
  });
}

var delegate = global.setInterval(makeRequest, 1, true);
script.removeEvent(delegate);
    