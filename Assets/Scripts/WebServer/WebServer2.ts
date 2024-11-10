import { EventDistributor } from "Scripts/Utils/EventDistributor";

@component
export class WebServer2 extends BaseScriptComponent {
  @input
  remoteServiceModule: RemoteServiceModule;

    private delay: DelayedCallbackEvent
    private httpRequest: RemoteServiceHttpRequest
    private IsOn: boolean; 

    @input 
    eventDistro: EventDistributor

    onAwake()
    {
        this.httpRequest = RemoteServiceHttpRequest.create();
        this.delay = this.QuerryAtInterval(1, true);    
    }

    ParseJson(JsonObject)
    {
      if (JsonObject)
      {
        this.eventDistro.SetCaliperSize(JsonObject["value"]);
      }
    }

    
    QuerryAtInterval(interval: number, executeInitially: boolean) {
        const delay = this.createEvent("DelayedCallbackEvent");
        delay.bind(() => {
            this.Querry();
            delay.reset(interval);
        });

        if (executeInitially) {
            delay.reset(0.0001);
        } else {
            delay.reset(interval);
        }

        return delay;
    }

  // Method called when the script is awake
  public Querry()
  {
    // Create a new HTTP request
    this.httpRequest.url = 'https://jaguar-wealthy-globally.ngrok-free.app/get_data'; // Set the URL for the request
    this.httpRequest.method = RemoteServiceHttpRequest.HttpRequestMethod.Get; // Set the HTTP method to GET

    // Perform the HTTP request
    if (this.IsOn)
    {
        this.remoteServiceModule.performHttpRequest(this.httpRequest, (response) => {
            if (response.statusCode === 200) {
              // Check if the response status is 200 (OK)
              var JsonObject = JSON.parse(response.body);
              this.ParseJson(JsonObject);
            }
            else
            {
                print(response.statusCode)
            }
          });
    }
  }


  public TurnOn() {
    this.IsOn = true;
  }

  public TurnOff()
  {
    this.IsOn = false;
  }

}