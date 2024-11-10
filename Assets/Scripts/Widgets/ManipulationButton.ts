import {ManipulateStates} from 'Scripts/Utils/EventDistributor';
import {EventDistributor} from 'Scripts/Utils/EventDistributor';
import {ToggleButton} from "SpectaclesInteractionKit/Components/UI/ToggleButton/ToggleButton"

@component
export class ManipulationButton extends BaseScriptComponent {

    /**
   */
    @input("int")
    @widget(
      new ComboBoxWidget([
        new ComboBoxItem("None",      1),
        new ComboBoxItem("Select",    2),
        new ComboBoxItem("Translate", 3),
        new ComboBoxItem("Scale",     4),
        new ComboBoxItem("Extrude",   5),
      ])
    )
    ManipulateState: number = 3

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
            eventDistributor.SetManipulationState(this.ManipulateState);
          }
          else
          {
            eventDistributor.SetManipulationState(ManipulateStates.None);
          }
      })
      }
    }

}
