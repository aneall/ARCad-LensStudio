import { MeshBuilding2 } from 'Scripts/MeshBuilding';
import {WebServer2} from 'Scripts/WebServer/WebServer2';


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

    @input
    @hint("Mesh builder")
    @allowUndefined
    meshBuilder : MeshBuilding2
    
    // put your class variables here
    onAwake()
    {
        // initialize class varaible here
    }

    public SetCaliperSize(CaliperData)
    {
        print(CaliperData)    
        this.CaliperSize = CaliperData
    }

    public SetSketchState(State)
    {
        this.SketchState = State;
        print("Sketch State" + this.SketchState);
        this.meshBuilder.createPrimitive(State, 20);
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

    public SetTranslationOffset(State)
    {
        this.meshBuilder.OffsetByDirection(State, this.CaliperSize);
    }
}

export enum SketchStates {
    None = 0,
    Square = 1,
    Triangle = 2,
    Circle = 3,
    Cylinder = 4,
    Cube = 5,
  }


export enum ManipulateStates {
    None,
    Select, 
    Translate,
    Scale,
    Extrude,
    Clear,
}

export enum ETranslate {
    None, 
    Up,
    Down,
    Left,
    Right
}