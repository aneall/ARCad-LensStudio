import { EventDistributor, SketchStates, ETranslate} from "./Utils/EventDistributor";

@component
export class MeshBuilding2 extends BaseScriptComponent {
    
  @input 
  meshVisual : RenderMeshVisual

@input
spherePrefab : ObjectPrefab
@input
eventDistributor : EventDistributor

  private builder: MeshBuilder;
  private origin: vec3;
  private counter: number;
  private isSelecting: boolean;
  private PointSpheres: any[]; // Define a more specific type if possible
  private selections: number[];

  onAwake() {
    this.builder = new MeshBuilder([{ name: 'position', components: 3 }]);
    this.origin = new vec3(0, 0, 0);
    this.builder.topology = MeshTopology.Triangles;
    this.builder.indexType = MeshIndexType.UInt16;
    this.counter = 0;
    this.isSelecting = true;
    this.PointSpheres = [];
    this.selections = [];
  }

  clearSelection() {
    this.selections = [];
    const ogScale = new vec3(2, 2, 2);
    this.PointSpheres.forEach(sphere => {
      sphere.getTransform().setWorldScale(ogScale);
    });
  }

  translateSelections(dir: vec3, scale: number) {
    this.selections.forEach(index => {
      print(index);
      const pos = this.PointSpheres[index].getTransform().getWorldPosition();
      print(pos);
      const arr = [pos.x + dir.x * scale, pos.y + dir.y * scale, pos.z + dir.z * scale];
      const vec = new vec3(pos.x + dir.x * scale, pos.y + dir.y * scale, pos.z + dir.z * scale);
      print(vec);
      this.PointSpheres[index].getTransform().setWorldPosition(vec);
      this.builder.setVertexInterleaved(index, arr);
    });
    this.setMeshRender();
  }

  OffsetByDirection(State: any, CaliperSize: number) {
    if (CaliperSize == null || CaliperSize == 0) {
      CaliperSize = 45 / 10;
    } 
    print(CaliperSize);    
    print("AHHHHHHHHHHHHHH");
    switch (State - 1) {
      case ETranslate.None:
        break;
      case ETranslate.Down:
        this.translateSelections(new vec3(0, 0, 1), CaliperSize); // z
        break;
      case ETranslate.Up:
        this.translateSelections(new vec3(0, 0, -1), CaliperSize); // -z
        break;
      case ETranslate.Left:
        this.translateSelections(new vec3(-1, 0, 0), CaliperSize); // -x
        break;
      case ETranslate.Right:
        this.translateSelections(new vec3(1, 0, 0), CaliperSize); // x
        break;
      default:
        print("wtf");
        break;
    }
  }

  scaleByOriginSelections(scaleFactor: number) {
    this.selections.forEach(index => {
      const pos = this.PointSpheres[index].getTransform().getWorldPosition();
      const adjustedPos = [pos.x - this.origin.x, pos.y - this.origin.y, pos.z - this.origin.z];
      const scaledPos = adjustedPos.map(value => value * scaleFactor) as [number, number, number];
      this.builder.setVertexInterleaved(index, scaledPos);
      this.PointSpheres[index].getTransform().setWorldPosition(new vec3(scaledPos[0], scaledPos[1], scaledPos[2]));
    });
    this.setMeshRender();
  }

  resetMesh() {
    const indCt = this.builder.getIndicesCount();
    const vertCt = this.builder.getVerticesCount()
    if (indCt != 0) { 
      this.builder.eraseIndices(0, indCt);
    }
    if (indCt != 0) {  
      this.builder.eraseVertices(0, vertCt);
    }
      
    this.setMeshRender();

    for (let i = 0; i < this.PointSpheres.length; i++) {
      this.PointSpheres[i].destroy();
    }
    this.PointSpheres = [];

  }

  createPrimitive(SketchState: SketchStates, size: number) {
    this.resetMesh();
    switch (SketchState - 1) {
      case SketchStates.Square:
        this.create2DSquare(size);
        break;
      case SketchStates.Triangle:
        this.create2DTriangle(size);
        break;
      case SketchStates.Cube:
        this.createCube(size);
        break;
      default:
        print("wtf");
    }
    this.setMeshRender();
    this.setupColliders();
  }

  private create2DSquare(size: number) {
    const halfSize = size / 2;
    const vertices = [
      -halfSize, -halfSize, 0,
      halfSize, -halfSize, 0,
      halfSize, halfSize, 0,
      -halfSize, halfSize, 0
    ];
    const indices = [0, 1, 2, 0, 2, 3];
    this.builder.appendVerticesInterleaved(vertices);
    this.createSpheresAtVertices(vertices);
    this.builder.appendIndices(indices);
  }

  private create2DTriangle(size: number) {
    const halfSize = size / 2;
    const vertices = [
      -halfSize, -halfSize, 0,
      halfSize, -halfSize, 0,
      -halfSize, halfSize, 0
    ];
    const indices = [0, 1, 2];
    this.builder.appendVerticesInterleaved(vertices);
    this.createSpheresAtVertices(vertices);
    this.builder.appendIndices(indices);
  }

  private createCube(size: number) {
    const halfSize = size / 2;
    const vertices = [
      -halfSize, -halfSize, halfSize,  halfSize, -halfSize, halfSize,
      halfSize, halfSize, halfSize, -halfSize, halfSize, halfSize,
      -halfSize, -halfSize, -halfSize, halfSize, -halfSize, -halfSize,
      halfSize, halfSize, -halfSize, -halfSize, halfSize, -halfSize
    ].map(element => element * halfSize);
    
    this.builder.appendVerticesInterleaved(vertices);
    this.createSpheresAtVertices(vertices);

    const indices = [
      0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7,
      3, 2, 6, 3, 6, 7, 0, 1, 5, 0, 5, 4,
      1, 2, 6, 1, 6, 5, 0, 3, 7, 0, 7, 4
    ];
    this.builder.appendIndices(indices);

    // const faceIndices = [0, 1, 2, 3];
    // faceIndices.forEach(i => this.selections.push(i));
  }

  private createSpheresAtVertices(vertices: number[]) {
    for (let i = 0; i < vertices.length; i += 3) {
      const pos = new vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
      const sphereInstance = this.spherePrefab.instantiate(this.getSceneObject());
      sphereInstance.getTransform().setWorldPosition(pos);
      this.PointSpheres.push(sphereInstance);
    }
  }

  private setMeshRender() {
    this.meshVisual.mesh = this.builder.getMesh();
    this.builder.updateMesh();
  }

  private setupColliders()
  {
    for (var i = 0; i < this.PointSpheres.length; i++) {
      const instance = this.PointSpheres[i];
      print(instance.getTransform().getWorldPosition());
      let index = 0;
      const body = instance.getComponent("Physics.BodyComponent");
      if (body !== null) {
        body.onCollisionEnter.add((e: CollisionEnterEventArgs) => 
          {
          const collision = e.collision;
          const contact = collision.contacts[0].position;
          let minVal = 5000;
    
          for (let j = 0; j < this.PointSpheres.length; j++) {
            const dist = this.distance(this.PointSpheres[j].getTransform().getWorldPosition(), contact);
            if (minVal > dist) {
              minVal = dist;
              index = j;
            }
          }
        
    
          this.selections.push(index);
          const scale = this.PointSpheres[index].getTransform().getWorldScale();
          const newScale = new vec3(scale.x * 2, scale.y * 2, scale.z * 2);
          this.PointSpheres[index].getTransform().setWorldScale(newScale);
        });
      }
    }
  }

  private distance(v1: vec3, v2: vec3): number {
    const dx = v1.x - v2.x;
    const dy = v1.y - v2.y;
    const dz = v1.z - v2.z;
    return Math.sqrt(dx * dx + dy * dy + dz * dz);
  }
  
}

// const manipulator = new MeshBuilding2();
// manipulator.createPrimitive("cube", 20);
