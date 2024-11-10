import {EventDistributor} from 'Scripts/Utils/EventDistributor';
import {ToggleButton} from "SpectaclesInteractionKit/Components/UI/ToggleButton/ToggleButton"

@component
export class NewScript extends BaseScriptComponent {
    private toggleButton: ToggleButton

    
    onAwake()
    {
      this.createEvent("OnStartEvent").bind(() => {
        this.init()
      })
    }

    init()
    {
      var eventDistributor = global.scene.getRootObject(1).getComponent(EventDistributor.getTypeName());
      this.toggleButton = this.getSceneObject().getComponent(ToggleButton.getTypeName());
      if (this.toggleButton != null)
      {
        this.toggleButton.onStateChanged.add((isToggledOn) => {
          if (isToggledOn)
          {
            eventDistributor.QuerryServer(true);
          }
          else
          {
            eventDistributor.QuerryServer(false);
          }
      })
      }
    }
}
