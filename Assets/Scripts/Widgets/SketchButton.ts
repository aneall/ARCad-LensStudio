import {SketchStates} from "Scripts/Utils/EventDistributor"

@component
export class NewScript extends BaseScriptComponent {

    @input
    @widget(
      new ComboBoxWidget([
        new ComboBoxItem(SketchStates.None.toString(), 0),
        new ComboBoxItem(SketchStates.Square.toString(), 1),
        new ComboBoxItem(SketchStates.Triangle.toString(), 2),
        new ComboBoxItem(SketchStates.Circle.toString(), 3),
        new ComboBoxItem(SketchStates.Cylinder.toString(), 4),
        new ComboBoxItem(SketchStates.Cube.toString(), 5),
      ])
    )
    SketchMode: number = 3
}
