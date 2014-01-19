var initialized=false;
var clock;
var scenes = [], cameras=[], controls=[], renderers=[];

function scrollListener(e) {
  //alert(e);  
  e.preventDefault();
}

function showScatter3D(data, axisNames, ctxtMin, ctxtMax, colorAttrMin, colorAttrMax, hostDivID) {
  var hostDiv = document.getElementById(hostDivID);
  
  hostDiv.addEventListener("DOMMouseScroll", scrollListener);
  hostDiv.onscroll = scrollListener;

  // Empty out the hostDiv
  hostDiv.innerHTML="";

  if(!initialized) {
    clock = new THREE.Clock();

    initialized = true;
  }

  ///////////
  // SCENE //
  ///////////
  var scene = new THREE.Scene();
  scenes.push(scene);
  
  ////////////
  // CAMERA //
  ////////////
  
  // set the view size in pixels (custom or according to window size)
  //var SCREEN_WIDTH = 400, SCREEN_HEIGHT = 300;
  //var SCREEN_WIDTH = window.innerWidth, SCREEN_HEIGHT = window.innerHeight;  
  var SCREEN_WIDTH = parent.document.getElementById("detail").offsetWidth*.8, SCREEN_HEIGHT = SCREEN_WIDTH*.75;
  // camera attributes
  var VIEW_ANGLE = 45, ASPECT = SCREEN_WIDTH / SCREEN_HEIGHT, NEAR = 0.1, FAR = 20000;
  // set up camera
  var camera = new THREE.PerspectiveCamera( VIEW_ANGLE, ASPECT, NEAR, FAR);
  cameras.push(camera);
  // add the camera to the scene
  scene.add(camera);
  // the camera defaults to position (0,0,0)
  //   so pull it back (z = 400) and up (y = 100) and set the angle towards the scene origin
  camera.position.set(0,150,400);
  camera.lookAt(scene.position);  
  
  //////////////
  // RENDERER //
  //////////////
  
  // create and start the renderer; choose antialias setting.
  if ( Detector.webgl )
    renderer = new THREE.WebGLRenderer( {antialias:true} );
  else
    renderer = new THREE.CanvasRenderer(); 
  renderers.push(renderer);
  
  renderer.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
  
  // attach div element to variable to contain the renderer
  //container = document.getElementById( 'ThreeJS' );
  // alternatively: to create the div at runtime, use:
  //   container = document.createElement( 'div' );
  //    document.body.appendChild( container );
  
  // attach renderer to the container div
  hostDiv.appendChild( renderer.domElement );
  hostDiv.style.width = SCREEN_WIDTH;
  hostDiv.style.height = SCREEN_HEIGHT;
  
  ////////////
  // EVENTS //
  ////////////
  
  // automatically resize renderer
  THREEx.WindowResize(renderer, camera);
  
  //////////////
  // CONTROLS //
  //////////////
  
  // move mouse and: left   click to rotate, 
  //                 middle click to zoom, 
  //                 right  click to pan
  var control = new THREE.OrbitControls( camera, renderer.domElement );
  controls.push(control);
  
  ///////////
  // LIGHT //
  ///////////
  
  // create a light
  /*var light = new THREE.PointLight(0xffffff);
  light.position.set(0,250,0);
  scene.add(light);*/
  var ambientLight = new THREE.AmbientLight(0x111111);
  scene.add(ambientLight);
  
  //////////////
  // GEOMETRY //
  //////////////
    
  // most objects displayed are a "mesh":
  //  a collection of points ("geometry") and
  //  a set of surface parameters ("material")  
 
  // Create the gradient to be used to color the tiles
  var colors = gradientFactory.generate({
      from: "#0000FF",
      to: "#FF0000",
      stops: 1000
  });
  
  // Identify the minimum separation between adjacent context values in each dimension
  var sep = ctxtSep(data, ctxtMin, ctxtMax);
  var maxCoord = []; // Array that keeps track of the maximum value taken by any coordinate
  for(var i=0; i<3; i++) maxCoord[i] = -1e100;
  var maxAllCoords = -1e100; // The maximum value among all values along all coordinates
  
  for(d in data) { if(data.hasOwnProperty(d)) {
    var sphereGeometry = new THREE.SphereGeometry( 3, 8, 8);
    // use a "lambert" material rather than "basic" for realistic lighting.
    //   (don't forget to add (at least one) light!)
    //var sphereMaterial = new THREE.MeshLambertMaterial();
    var sphereMaterial = new THREE.MeshBasicMaterial();

    var colorIdx=0;
    if(colorAttrMax - colorAttrMin > 0) 
      colorIdx = Math.floor(((data[d][3] - colorAttrMin) / (colorAttrMax - colorAttrMin))*1000);
    var color = new THREE.Color(colors[colorIdx]);
    sphereMaterial.color = color;
    var sphere = new THREE.Mesh(sphereGeometry, sphereMaterial);

    // Set the sphere's position
    var pos = [];
    for(var i=0; i<3; i++) pos[i] = 16 * (data[d][i] - ctxtMin[i])/sep[i];
    sphere.position.set(pos[0], pos[1], pos[2]);

    // Update maxCoord
    for(var i=0; i<3; i++) {
    	maxCoord[i] = (pos[i] > maxCoord[i]? pos[i] : maxCoord[i]);
    	maxAllCoords = (maxCoord[i] > maxAllCoords? maxCoord[i]: maxAllCoords);
    }

    // Add the sphere to the scene
    scene.add(sphere);

    // If there are additional trace dimensions, add a vector represent them
    if(data[d].length>4) {
      // Direction vector
      // direction (normalized), origin, length, color(hex)

      var origin = new THREE.Vector3(pos[0], pos[1], pos[2]);
      var direction  = new THREE.Vector3(data[d][4], 
                                         data[d].length>5? data[d][5]: 0, 
                                         data[d].length>6? data[d][6]: 0);
      //var direction = new THREE.Vector3().subVectors(terminus, origin).normalize();
      var arrow = new THREE.ArrowHelper(direction, origin, 50, 0x884400);
      scene.add(arrow);
    }
  } }
  
  // create a set of coordinate axes to help orient user
  //    specify length in pixels in each direction
  var axes = new THREE.AxisHelper(maxAllCoords);
  scene.add( axes );
  
  axisLabel(scene, axes, 160+maxCoord[0], -20,         0,              axisNames[0]);
  axisLabel(scene, axes, 130,             maxCoord[1], 0,              axisNames[1]);
  axisLabel(scene, axes, 130,             -20,         10+maxCoord[2], axisNames[2]);
  
  // fog must be added to scene before first render
  scene.fog = new THREE.FogExp2( 0x9999ff, 0.00025 );

  animate();
}

function animate() 
{
  requestAnimationFrame( animate );
  render();
  update();
}

function update()
{
  // delta = change in time since last call (in seconds)
  var delta = clock.getDelta(); 

  for(i in controls) { if(controls.hasOwnProperty(i)) {
    controls[i].update();
  } }
  //stats.update();
}

function render() 
{ 
  for(i in renderers) { if(renderers.hasOwnProperty(i)) {
    renderers[i].render( scenes[i], cameras[i] );
  } }
}

function axisLabel(scene, axes, x, y, z, text)
{
  var canvas1 = document.createElement('canvas');
  var context1 = canvas1.getContext('2d');
  context1.font = "Bold 30px Arial";
  context1.fillStyle = "rgba(0,0,0,1)";
  context1.fillText(text, 0, 50);
    
  // canvas contents will be used for a texture
  var texture1 = new THREE.Texture(canvas1) 
  texture1.needsUpdate = true;
      
  var material1 = new THREE.MeshBasicMaterial( {map: texture1, side:THREE.DoubleSide } );
  material1.transparent = true;

  var mesh1 = new THREE.Mesh(
        new THREE.PlaneGeometry(canvas1.width, canvas1.height),
        material1
      );
  mesh1.position.set(x, y, z);
  scene.add(mesh1);
}

// Identify the appropriate separation between adjacent context values in each dimension,
// which is a balance between showing the difference between the closest observations and
// keeping the overall distribution of observations visible
// ctxtMin/ctxtMas - the minimum and maximum values in each coordinate
function ctxtSep(data, ctxtMin, ctxtMax) {
  var coordList = []; // One list for each dimension of the context values in that dimension
  for(var i=0; i<3; i++) coordList[i] = {};
  
  // Populate coordList
  for(d in data) { if(data.hasOwnProperty(d)) {
    for(var i=0; i<3; i++)
      coordList[i][data[d][i]]=1;
  } }
  
  // Replace the hashes in coordList to be arrays of their keys, sorted so that adjacent 
  // values in the list correspond to numerically adjacent values
  for(var i=0; i<3; i++) coordList[i] = Object.keys(coordList[i]).sort(function(a,b){return a - b});
  
  // Iterate through coordList to identify the smallest distance between adjacent values
  var sep = [];
  for(var i=0; i<3; i++) sep[i] = 1e100;
  
  for(var i=0; i<3; i++) {
    for(var j in coordList[i]) { if(coordList[i].hasOwnProperty(j)) {
      if(j>0) {
        var diff = coordList[i][j] - coordList[i][j-1];
        sep[i] = (diff < sep[i] ? diff: sep[i]);
      }
    } }

    // If the separation is still 1e100, coordList[i] must only have one element. Thus, default sep[i] to 1.
    if(sep[i]==1e100) sep[i] = 1;
  }

  // Now adjust sep to ensure that the minimum separation we'll show is not too much smaller
  // than the overall size of the dataset in each dimension.
  for(var i=0; i<3; i++) sep[i] = Math.max((ctxtMax[i]-ctxtMin[i]) / 2e1, sep[i]);
  
  return sep;
}
