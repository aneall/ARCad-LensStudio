import {SketchStates} from 'Scripts/Utils/EventDistributor';
import {EventDistributor} from 'Scripts/Utils/EventDistributor';
import {ToggleButton} from "SpectaclesInteractionKit/Components/UI/ToggleButton/ToggleButton"

@component
export class SketchButton extends BaseScriptComponent {

    /**
   */
    @input("int")
    @widget(
      new ComboBoxWidget([
        new ComboBoxItem("None", 1),
        new ComboBoxItem("Up", 2),
        new ComboBoxItem("Down", 3),
        new ComboBoxItem("Left", 4),
        new ComboBoxItem("Right", 5),
      ])
    )
    SketchMode: number = 3

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
            eventDistributor.SetSketchState(this.SketchMode);
          }
          else
          {
            eventDistributor.SetSketchState(SketchStates.None);
          }
      })
      }
    }

}
