import {WebServer2} from 'Scripts/WebServer/WebServer2';
import { MeasureMesh } from 'Scripts/Widgets/MeasureMesh';


@component
export class EventDistributor extends BaseScriptComponent {
    @input
    @hint("The icon to be shown when the button is toggled on")
    @allowUndefined
    _tapText: SceneObject

    SketchState: SketchStates
    ManipulationState: ManipulateStates

    CaliperSize: number

    @input("Component.ScriptComponent")
    @hint("The icon to be shown when the button is toggled on")
    @allowUndefined
    WebServerRef: WebServer2
    

    @input()
    @hint("The icon to be shown when the button is toggled on")
    @allowUndefined
    measureMesh: MeasureMesh


    // put your class variables here
    onAwake()
    {
        // initialize class varaible here
    }

    public SetCaliperSize(CaliperData)
    {
        print(CaliperData)    
        this.CaliperSize = CaliperData
        this.measureMesh.SetMeasureWidgetSize(CaliperData);
    }

    public SetSketchState(State)
    {
        this.SketchState = State;
        print("Sketch State" + this.SketchState);
        // send to anyone down stream that cares about this...
        //@felicity
    } 

    
    public SetManipulationState(State)
    {
        this.ManipulationState = State
        print("Manipulation state" + this.ManipulationState)
        //this.OnManipulationStateChanged.invoke(this.ManipulationState);
        // Send to anyone down stream that cares about this... 
        //@felicity
    }

    public QuerryServer(CanQuery: boolean)
    {
        print(CanQuery)
        if (CanQuery)
        {
            this.WebServerRef.TurnOn();
        }
        else
        {
            this.WebServerRef.TurnOff();
        }
    }
}

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
