// import required modules
const WorldQueryModule = require('LensStudio:WorldQueryModule');
const SIK = require('SpectaclesInteractionKit/SIK').SIK;
const InteractorTriggerType =
  require('SpectaclesInteractionKit/Core/Interactor/Interactor').InteractorTriggerType;
const InteractorInputType =
  require('SpectaclesInteractionKit/Core/Interactor/Interactor').InteractorInputType;
const EPSILON = 0.01;

// define this component (aka TypeScript file) as a new script
@component
export class NewScript extends BaseScriptComponent {
  private primaryInteractor;
  private hitTestSession;
  private transform: Transform;

  @input
  targetObject: SceneObject;

  @input
  filterEnabled: boolean;

   // method that executes first
  onAwake() {
    // create new hit session
    this.hitTestSession = this.createHitTestSession(this.filterEnabled);
    if (!this.sceneObject) {
      print('Please set Target Object input');
      return;
    }
    this.transform = this.targetObject.getTransform();
    // disable target object when surface is not detected
    this.targetObject.enabled = false;
    // create update event
    this.createEvent('UpdateEvent').bind(this.onUpdate.bind(this));
  }

  createHitTestSession(filterEnabled) {
    // create hit test session with options
    var options = HitTestSessionOptions.create();
    options.filter = filterEnabled;

    var session = WorldQueryModule.createHitTestSessionWithOptions(options);
    return session;
  }

  onHitTestResult(results) {
    if (results === null) {
      this.targetObject.enabled = false;
    } else {
      this.targetObject.enabled = true;
      // get hit information
      const hitPosition = results.position;
      const hitNormal = results.normal;

      //identifying the direction the object should look at based on the normal of the hit location.

      var lookDirection;
      if (1 - Math.abs(hitNormal.normalize().dot(vec3.up())) < EPSILON) {
        lookDirection = vec3.forward();
      } else {
        lookDirection = hitNormal.cross(vec3.up());
      }

      const toRotation = quat.lookAt(lookDirection, hitNormal);
      //set position and rotation
      this.targetObject.getTransform().setWorldPosition(hitPosition);
      this.targetObject.getTransform().setWorldRotation(toRotation);

      if ( // if there was NOT a trigger in the previous frame AND there is a trigger in the current frame""
        this.primaryInteractor.previousTrigger !== InteractorTriggerType.None &&
        this.primaryInteractor.currentTrigger === InteractorTriggerType.None
            
        // TODO: change from .None -> .Poke (as a test)
                // If this doesn't work, use either:
                    // distance check (finger joint to detected plane) -> if distance is small enough, instantiate object
                    // collider check (add a collider to object and plane) -> there's a collision, instantiate object'
      ) {
        // ^^ If that is true (i.e. the interaction has occurred aka trigger ended), then:
        // Create a copy of the target sceneObject at the location on the detected plane:
        this.sceneObject.copyWholeHierarchy(this.targetObject);
      }
    }
  }

  onUpdate() {
    this.primaryInteractor =
      SIK.InteractionManager.getTargetingInteractors().shift();

    if (
      this.primaryInteractor &&
      this.primaryInteractor.isActive() &&
      this.primaryInteractor.isTargeting()
    ) {
      const rayStartOffset = new vec3(
        this.primaryInteractor.startPoint.x,
        this.primaryInteractor.startPoint.y,
        this.primaryInteractor.startPoint.z + 30
      );
      const rayStart = rayStartOffset;
      const rayEnd = this.primaryInteractor.endPoint;

      this.hitTestSession.hitTest(
        rayStart,
        rayEnd,
        this.onHitTestResult.bind(this)
      );
    } else {
      this.targetObject.enabled = false;
    }
  }
}