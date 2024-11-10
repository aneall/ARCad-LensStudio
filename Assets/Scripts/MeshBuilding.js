//@input Component.MeshVisual meshVisual
//@input Asset.ObjectPrefab spherePrefab

var builder = new MeshBuilder([{ name: 'position', components: 3 }]);
var origin = new vec3(0, 0, 0);

builder.topology = MeshTopology.Triangles;
builder.indexType = MeshIndexType.UInt16;

var counter = 0;
var isSelecting = true;

/* 
Selection Handler
*/
// Additive selection: can select multiple points and perform translation and scaling
// Press clear button to clear selections
var PointSpheres = []; // list of spherePrefabs representing every vertex in the mesh, turns red when selected
var selections = []; // stores indices of vertices selected

// script.collider.onCollisionEnter.add(function(eventArgs) {
//   var collision = eventArgs.collision; 
//   print("CollisionEnter(" + collision.id + "): contacts=" + collision.contactCount + " ---> " + collision.collider.getSceneObject().name);
// });

function clearSelection() {
  selections = [];
}

/*
Operations
*/

function translateSelections() { // confirms translation
  for (let i = 0; i < selections.length; i++) { // translate corresponding vertices
    let pos = PointSpheres[selections[i]].getTransform().getWorldPosition();
    builder.setVertexInterleaved(i, [pos.x, pos.y, pos.z]);
  } 
}

function scaleByOriginSelections(scaleFactor) {
  for (let i = 0; i < selections.length; i++) { // scale corresponding vertices by origin
    let pos = PointSpheres[selections[i]].getTransform().getWorldPosition();
    let adjustedPos = [pos.x - origin.x, pos.y - origin.y, pos.z - origin.y];
    const scaledPos = adjustedPos.map(element => element * scaleFactor);
    builder.setVertexInterleaved(i, scaledPos); // update builder
    PointSpheres[i].getTransform().setWorldPosition(new vec3(scaledPos[0], scaledPos[1], scaledPos[2])); // update pointSpheres
  } 
  setMeshRender();
}

/* 
Mesh Builder 
*/
function resetMesh() {
    var indices = builder.getIndicesCount();
    var vertices = builder.getVerticesCount();
    builder.eraseIndices(0, indices);
    builder.eraseVertices(0, vertices);
    setMeshRender();
}

function createPrimitive(shape, size) {
  if (shape == "square") {
    create2DSquare(size);
  } else if (shape == "triangle") {
    create2DTriangle(size);
  } else if (shape == "cube") {
    createCube(size);
  } else {
    print("wtf");
  }
  setMeshRender();
}

function create2DSquare(size) {
  const halfSize = size / 2;
  const vertices = [
      -halfSize, -halfSize, 0,
      halfSize, -halfSize, 0,
      halfSize, halfSize, 0,
      -halfSize, halfSize, 0
  ];
  const indices = [
      0, 1, 2, // First triangle
      0, 2, 3  // Second triangle
  ];
  builder.appendVerticesInterleaved(vertices);
  createSpheresAtVertices(vertices);
  builder.appendIndices(indices);
}

function create2DTriangle(size) {
  const halfSize = size / 2;
  const vertices = [
      -halfSize, -halfSize, 0,
      halfSize, -halfSize, 0,
      -halfSize, halfSize, 0
  ];
  const indices = [
      0, 1, 2
  ];  
  builder.appendVerticesInterleaved(vertices);
  createSpheresAtVertices(vertices);
  builder.appendIndices(indices);
}

function createCube(size) {
  const halfSize = size / 2;
  const array = [
    // Front face
    -1, -1,  1, // 0
     1, -1,  1, // 1
     1,  1,  1, // 2
    -1,  1,  1, // 3
    // Back face
    -1, -1, -1, // 4
     1, -1, -1, // 5
     1,  1, -1, // 6
    -1,  1, -1  // 7
  ]
  const vertices = array.map(element => element * halfSize);
  builder.appendVerticesInterleaved(vertices);
  createSpheresAtVertices(vertices);

  // Define indices for the cube faces
  builder.appendIndices([
      // Front face
      0, 1, 2, 0, 2, 3,
      // Back face
      4, 5, 6, 4, 6, 7,
      // Top face
      3, 2, 6, 3, 6, 7,
      // Bottom face
      0, 1, 5, 0, 5, 4,
      // Right face
      1, 2, 6, 1, 6, 5,
      // Left face
      0, 3, 7, 0, 7, 4
  ]);
  // testing transformations
  var face = [0, 1, 2, 3];
  for (let i = 0; i < face.length; i++) {
    selections.push(face[i]);
  }  
}

// creates sphere markers at every vertex
function createSpheresAtVertices(vertices) {
  for (let i = 0; i < vertices.length; i+=3) {
    var pos = new vec3(vertices[i], vertices[i+1], vertices[i+2]);
    var sphereInstance = script.spherePrefab.instantiate(script.getSceneObject());
    sphereInstance.getTransform().setWorldPosition(pos);
    PointSpheres.push(sphereInstance);
  }  
}

function setMeshRender() {
    script.meshVisual.mesh = builder.getMesh();
    builder.updateMesh();
}

createPrimitive("cube", 20);

/* 
Editor touch events for testing
*/
script.createEvent('TouchStartEvent').bind(function (eventData) {
  print('Touch Start');
  // print(PointSpheres.length);
  // if (counter == 0) {
  //   createPrimitive("cube", 20);
  // } else {
    
  //   //translateSelections();
  //   //scaleByOriginSelections(2);
  // }  
  // var scale = PointSpheres[counter].getTransform().getWorldScale();
  // var newScale = new vec3(scale.x * 2, scale.y * 2, scale.z * 2);
  // PointSpheres[counter].getTransform().setWorldScale(newScale);
  // for (let i = 0; i < PointSpheres.length; i++) {
  //   print(PointSpheres[i].getTransform().getWorldScale());
  // }
});

script.createEvent('TouchMoveEvent').bind(function (eventData) {
  print('Touch Move');
});

script.createEvent('TouchEndEvent').bind(function (eventData) {
  print('Touch End');
  //resetMesh();
});

script.createEvent('TapEvent').bind(function (eventData) {
  print('Tap');
  counter++;
});

// Attach collision event listener to each instance
for (let i = 0; i < PointSpheres.length; i++) {
  var instance = PointSpheres[i];
  print(instance.getTransform().getWorldPosition());
  var body = instance.getComponent('Physics.BodyComponent');
  if (body != null)
  {
    body.onCollisionEnter.add(function (e) {
      var collision = e.collision;      
      var contact = collision.contacts[0].position;
      var minVal = 5000;
      var index = 0;
      for (let i = 0; i < PointSpheres.length; i++) {
        var dist = distance(PointSpheres[i].getTransform().getWorldPosition(), contact);
        if (minVal > dist) {
          minVal = dist;
          index = i;
        }
      }
      selections.push(index);
      var scale = PointSpheres[index].getTransform().getWorldScale();
      var newScale = new vec3(scale.x * 2, scale.y * 2, scale.z * 2);
      PointSpheres[index].getTransform().setWorldScale(newScale);
    });
  }

  function distance(v1, v2) {
    const dx = v1.x - v2.x;
    const dy = v1.y - v2.y;
    const dz = v1.z - v2.z;
    return Math.sqrt(dx * dx + dy * dy + dz * dz);
  }
  
}