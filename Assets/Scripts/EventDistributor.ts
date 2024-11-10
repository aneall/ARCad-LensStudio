export enum SketchStates {
    None,
    Square,
    Triangle,
    Circle,
    Cylinder,
    Cube,
  }


export enum ManipulateStates {
    None,
    Select, 
    Translate,
    Scale,
    Extrude,
}


export class EventDistributor extends BaseScriptComponent {
    @input
    @hint("The icon to be shown when the button is toggled on")
    @allowUndefined
    _tapText: SceneObject

    SketchState: SketchStates
    ManipulationState: ManipulateStates

    public SetSketchState(State)
    {
        this.SketchState = State
        // send to anyone down stream that cares about this...
        //@felicity
    } 

    public SetManipulationState(State)
    {
        this.ManipulationState = State
        // Send to anyone down stream that cares about this... 
        //@felicity
    }

}
