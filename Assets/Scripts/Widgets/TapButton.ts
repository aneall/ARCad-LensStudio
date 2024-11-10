import {Interactable} from "SpectaclesInteractionKit/Components/Interaction/Interactable/Interactable"
import ReplayEvent from "SpectaclesInteractionKit/Utils/ReplayEvent"
import {createCallback} from "SpectaclesInteractionKit/Utils/InspectorCallbacks"

/*
 * TapButton provides basic toggle functionality for the prefab toggle button.
 * It is meant to be added to a Scene Object with an Interactable component, with visual behavior configured in the Lens Studio scene.
 */
@component
export class TapButton extends BaseScriptComponent {

  @input
  @hint("The icon to be shown when the button is toggled on")
  @allowUndefined
  _tapText: SceneObject
  @input
  @hint(
    "The initial state of the button, set to true if toggled on upon lens launch."
  )
  private _isToggledOn: boolean = false
  @input
  @hint("Enable this to add functions from another script to this component's callback events")
  editEventCallbacks: boolean = false
  @ui.group_start("On State Changed Callbacks")
  @showIf("editEventCallbacks")
  @input
  @hint("The script containing functions to be called on toggle state change")
  @allowUndefined
  private customFunctionForOnStateChanged: ScriptComponent
  @input
  @hint("The names for the functions on the provided script, to be called on toggle state change")
  @allowUndefined
  private onStateChangedFunctionNames: string[] = []
  @ui.group_end
  
  private interactable: Interactable

  private onStateChangedEvent = new ReplayEvent<boolean>()
  public readonly onStateChanged = this.onStateChangedEvent.publicApi()

  onAwake() {
    this.interactable = this
      .getSceneObject()
      .getComponent(Interactable.getTypeName()) 

    this.createEvent("OnStartEvent").bind(() => {
      if (!this.interactable) {
        throw new Error(
          "Toggle Button requires an Interactable Component on the same Scene object in order to work - please ensure one is added."
        )
      }
      this.interactable.onTriggerEnd.add(() => {
        if (this.enabled) {
          this.toggleState()           
        }
      })
    })
    
    if (this.editEventCallbacks && this.customFunctionForOnStateChanged) {
      this.onStateChanged.add(
        createCallback<boolean>(
          this.customFunctionForOnStateChanged,
          this.onStateChangedFunctionNames
        )
      )
    }

    this.refreshVisual()
    this.onStateChangedEvent.invoke(this._isToggledOn)
  }

  /**
   * Toggles the state of the button
   */
  toggle() {
    this.toggleState()
  }

  /**
   * @returns the icon to be shown when the button is toggled on
   */
  get onIcon(): SceneObject {
    return this._tapText
  }

  /**
   * @param iconObject - the icon to be shown when the button is toggled on
   */
  set onIcon(iconObject: SceneObject) {
    this._tapText = iconObject
    this.refreshVisual()
  }

  /**
   * @returns the current toggle state of the button
   */
  get isToggledOn(): boolean {
    return this._isToggledOn
  }

  /**
   * @param toggleOn - the new state of the button, invoking the toggle event if different than current state.
   */
  set isToggledOn(toggleOn: boolean) {
    this.toggleState()
  }

  private refreshVisual() {
    if (this._tapText !== undefined) {
      this._tapText.enabled = this._isToggledOn
    }

  }

  private toggleState() {
    this._isToggledOn = !this._isToggledOn
    this.refreshVisual()
    this.onStateChangedEvent.invoke(this._isToggledOn)
  }
}
