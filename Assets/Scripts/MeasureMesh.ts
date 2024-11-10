@component
export class MeasureMesh extends BaseScriptComponent {
    
    @input
    @hint("Parent of both the rig and the mesh")
    /** @inheritdoc */
    origin?: SceneObject = null

    @input("int")
    @hint(
      "Precission supported by this widget"
    )
    /** @inheritdoc */
    precission: number = 3

    @input()
    @hint(
      "Direction Vector to extend towards"
    )
    /** @inheritdoc */
    directionVector: vec3 =  new vec3(0, 1, 0)
    
    @input
    @hint("Parent of both the rig and the mesh")
    /** @inheritdoc */
    text: Text

    builder = new MeshBuilder([
        { name: 'position', components: 3 },
        { name: 'color', components: 4 },
    ]);

    public SetMeasureWidgetSize(newSize: number) : void
    {
        let renderMesh = this.sceneObject.createComponent("RenderMeshVisual")
        var builder = new MeshBuilder([
            { name: 'position', components: 3 }, //attribute 1
        ]);
          
          builder.topology = MeshTopology.LineStrip;
          var originPosition = this.origin.getTransform().getLocalPosition();
          
          originPosition.x + ((this.directionVector.x * originPosition.x) * newSize);
          var extentPosition = this.directionVector.uniformScale(newSize).add(originPosition);
          //var LinePoints = [originPosition, extentPosition];

          // for(var i = 0; i < LinePoints.length; i++)
          // {
          //   builder.appendVerticesInterleaved([
          //       LinePoints[i].x - this.thickness, LinePoints[i].y - this.thickness, LinePoints[i].z,
          //       LinePoints[i].x - this.thickness, LinePoints[i].y + this.thickness, LinePoints[i].z,
          //       LinePoints[i].x + this.thickness, LinePoints[i].y + this.thickness, LinePoints[i].z,
          //       LinePoints[i].x + this.thickness, LinePoints[i].y - this.thickness, LinePoints[i].z,
          //   ]);    
          // }


          builder.appendVerticesInterleaved([
            originPosition.x,originPosition.y,originPosition.z,
            extentPosition.x,extentPosition.y,extentPosition.z,
          ]);
          renderMesh.mesh = builder.getMesh();
          builder.updateMesh();
            
          this.text.text = extentPosition.distance(originPosition).toPrecision(this.precission).toString();
          var midpoint = originPosition.add(extentPosition).uniformScale(0.5);
          this.text.getTransform().setLocalPosition(midpoint);
    }
}
