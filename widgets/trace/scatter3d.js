var initialized=false;
var clock;
// Maps each hostID to the properties of the 3D scene within it
var scenes = {}, cameras={}, controls={}, renderers={}, screenSizes={}, axes = {}, axisText = {};

// Maps each hostID to the hash of the arguments passed to refreshScatter3D() to define its contents
var contents = {};

// Maps each hostID to a hash that maps data attribute indexes to a records that indicate 
// whether this attribute's data is numerical or categorical:
// If all values are either numeric or undefined {type: "numeric", min:, max:}
// If at least one defined value is non-numeric {type: "categorical", categories: hash with categories as keys }
var attrType = {};

// Maps each host div and attribute index to the minimum and maximum values of 
// its current range, as selected by the user.
var rangeMin = {}, rangeMax = {};

// Maps each host div and index within its data array to the sphere object that 
// represents this observation and a flag that indicates whether the sphere is currently 
// being shown.
var particles = {};
var particleSystems = {};
var particleAttrs = {};
var particleUnif = { /*color:   { type: "c", value: new THREE.Color( 0xff0000 ) },*/
                     texture: { type: "t", value: THREE.ImageUtils.loadTexture( "img/dot.png" ) } };
var particleMaterials = {};
var arrows = {};
var sphereShown = {};
var arrowShown = {};

// The span in pixels of the data in each dimension. The axis for that dimension will be scaled
// to make dat fit into this many pixels.
var spaceAxisSize = 500;
var colorAxisSize = 1000;

 // Create the gradient to be used to color the tiles
 var colors = gradientFactory.generate({
     from: "#0000FF",
     to: "#FF0000",
     stops: colorAxisSize+1
 });

// The unique IDs of each visualization and their printable names
var vizNames = {};
var vizX       = 0; vizNames[vizX      ] = "X";
var vizY       = 1; vizNames[vizY      ] = "Y";
var vizZ       = 2; vizNames[vizZ      ] = "Z";
var vizColor   = 3; vizNames[vizColor  ] = "Color";
/*var vizXVector = 4; vizNames[vizXVector] = "XVector";
var vizYVector = 5; vizNames[vizYVector] = "YVector";
var vizZVector = 6; vizNames[vizZVector] = "ZVector";*/

// For each host div, maps the index of each visualization (x, y, z, color, vectorX, vectorY, vectorZ) 
// to the data dimensions assigned to it and vice versa.
var viz2Attr = {};
var attr2Viz = {};

function scrollListener(e) {
  //alert(e);  
  e.preventDefault();
}

/*var projector, mouse = { x: 0, y: 0 }, INTERSECTED;
function onDocumentMouseMove( event ) 
{
	// the following line would stop any other event handler from firing
	// (such as the mouse's TrackballControls)
	//event.preventDefault();
	
	// update the mouse variable
	mouse.x = ( event.clientX / window.innerWidth ) * 2 - 1;
	mouse.y = - ( event.clientY / window.innerHeight ) * 2 + 1;
}*/

// create the particle variables
//var pMaterial = new THREE.ParticleSystemMaterial( { size: 40, map: THREE.ImageUtils.loadTexture( "img/dot.png" ), vertexColors: true, transparent: true } );
	/*new THREE.ParticleBasicMaterial({
      color: 0x000000,
      size: 20
    });*/
//pMaterial.color.setHSL( 1.0, 0.2, 0.7 );


// Called from trace.js to show or un-hide the 3d scatter plot visualization
function showScatter3D(data, attrNames, minVals, maxVals, 
                       numCtxtVars, numTraceAttrs, hostDivID) {
  // Records whether we've already initialized the state of this hostDiv
  var hostDivInitialized = (typeof contents[hostDivID] !== "undefined");

  // If the 3D visualization has already been initialized, then the user is requesting to show it after they previously hid it
  if(hostDivInitialized)
    show3DViz(hostDivID);
  // Otherwise, if the 3d visualization has not been initialized yet, do so now
  else {
    refreshScatter3D(data, attrNames, minVals, maxVals, 
                     numCtxtVars, numTraceAttrs, hostDivID);
    show3DViz(hostDivID);
  }
}
  
function refreshScatter3D(data, attrNames, minVals, maxVals, 
                          numCtxtVars, numTraceAttrs, hostDivID) {
  // Records whether we've already initialized the state of this hostDiv
  var hostDivInitialized = (typeof contents[hostDivID] !== "undefined");
  
  var hostDiv = document.getElementById(hostDivID);
  
  ///////////
  // SCENE //
  ///////////
  var scene;
  if(hostDivInitialized) {
    scene = scenes[hostDivID];
  } else {
    scene = new THREE.Scene();
    scenes[hostDivID]=scene;
  }

  // If this is the first time we're trying to display this host div
  if(!hostDivInitialized) {
    // Cache the arguments to refreshScatter3D
    contents[hostDivID] = {data: data,
                           attrNames: attrNames,
                           minVals: minVals,         maxVals: maxVals,
                           numCtxtVars: numCtxtVars, numTraceAttrs: numTraceAttrs};
   
   
    // Initialize the particles array for this host div, one particle for each data observation
    particles[hostDivID] = new THREE.Geometry();
    // Initialize the particle geometry, with one vertex for each data point
    for(d in data) { if(data.hasOwnProperty(d)) {
	    //for(var d=0; d<100; d++) { {
      //particles[hostDivID].vertices.push(new THREE.Vertex(new THREE.Vector3(0, 0, 0)));
		  particles[hostDivID].vertices.push(new THREE.Vector3(0, 0, 0));
	  } }
	  // Initialize the material to use for the particles
	  particleAttrs[hostDivID] = { alpha:   { type: 'f', value: [] },
                                 color:   { type: "c", value: [] }  };
	  particleMaterials[hostDivID] = new THREE.ShaderMaterial( {
                                                uniforms:       particleUnif,
                                                attributes:     particleAttrs[hostDivID],
                                                vertexShader:   //document.getElementById( 'vertexshader' ).textContent,
												                            [
                                                      "attribute vec3 color;",
                                                      "attribute float alpha;",
                                                      "varying vec3 vColor;",
                                                      "varying float vAlpha;",
                                                      
                                                      "void main() {",
                                                        "vColor = color;",
                                                        "vAlpha = alpha;",
                                                        "vec4 mvPosition = modelViewMatrix * vec4( position, 1.0 );",
                                                        "gl_PointSize = 10.0;",
                                                        "gl_Position = projectionMatrix * mvPosition;",
                                                      "}"
                                                    ].join("\n"),
                                                fragmentShader: //document.getElementById( 'fragmentshader' ).textContent,
												                            [
                                                      "uniform vec3 color;",
                                                      "uniform sampler2D texture;",
                                                      "varying vec3 vColor;",
                                                      "varying float vAlpha;",
                                                      
                                                      "void main() {",
                                                        "gl_FragColor = vec4(vColor, 1);",
                                                        "gl_FragColor.a *= vAlpha;",
                                                        
														//"vec4 col = vec4(vColor, 100);",
                                                        //"vec4 textureColor =  texture2D( texture, gl_PointCoord );",
                                                        //"gl_FragColor = textureColor; // vec4( (col*textureColor).rgb, vAlpha );",
														//"gl_FragColor.a = vAlpha;",
														//"gl_FragColor = vec4(0, 1, 0, .1);",
														
														//"vec2 loc = vec2(25, 25);",
														//"gl_FragColor = texture2D( texture, loc );",
														//"gl_FragColor.a *= vAlpha;",
                                                      "}"
                                                    ].join("\n"),
                                                transparent:    true
                                            });
	  // Create a ParticleSystem from the array of particles
	  particleSystem = new THREE.ParticleSystem(particles[hostDivID], particleMaterials[hostDivID]);
	  particleSystem.dynamic = true;
	  particleSystem.sortParticles = true;
	  scene.add(particleSystem);
	    
	  //particleColors[hostDivID] = [];
    //sphereMaterials[hostDivID] = [];
    arrows[hostDivID] = [];
    sphereShown[hostDivID] = [];
    arrowShown[hostDivID] = [];
	  axisText[hostDivID] = [];
    
    // Initialize the mapping between the visualizations available for different data dimensions
    // and the coordinate of the data to be used with that visualization
    viz2Attr[hostDivID] = {};
    attr2Viz[hostDivID] = {};
    
    // x, y, z of sphere position
    var curAttrIdx=0;
    if(numCtxtVars >= 1) { viz2Attr[hostDivID][vizX] = curAttrIdx; attr2Viz[hostDivID][curAttrIdx] = vizX; curAttrIdx++; }
    else                 { viz2Attr[hostDivID][vizX] = -1; }
    if(numCtxtVars >= 2) { viz2Attr[hostDivID][vizY] = curAttrIdx; attr2Viz[hostDivID][curAttrIdx] = vizY; curAttrIdx++; }
    else                 { viz2Attr[hostDivID][vizY] = -1; }
    if(numCtxtVars >= 3) { viz2Attr[hostDivID][vizZ] = curAttrIdx; attr2Viz[hostDivID][curAttrIdx] = vizZ; curAttrIdx++; }
    else                 { viz2Attr[hostDivID][vizZ] = -1; }
    // sphere color
    if(numCtxtVars >= 4 || numTraceAttrs >= 1) { viz2Attr[hostDivID][vizColor] = curAttrIdx; attr2Viz[hostDivID][curAttrIdx] = vizColor; curAttrIdx++; }
    else                 { viz2Attr[hostDivID][vizColor] = -1; }
    // x, y, z of arrow direction
    /*if(numCtxtVars >= 5 || numTraceAttrs >= 2) { viz2Attr[hostDivID][vizXVector] = curAttrIdx; attr2Viz[hostDivID][curAttrIdx] = vizXVector; curAttrIdx++; }
    else                 { viz2Attr[hostDivID][vizYVector] = -1; }
    if(numCtxtVars >= 6 || numTraceAttrs >= 3) { viz2Attr[hostDivID][vizYVector] = curAttrIdx; attr2Viz[hostDivID][curAttrIdx] = vizYVector; curAttrIdx++; }
    else                 { viz2Attr[hostDivID][vizYVector] = -1; }
    if(numCtxtVars >= 7 || numTraceAttrs >= 4) { viz2Attr[hostDivID][vizZVector] = curAttrIdx; attr2Viz[hostDivID][curAttrIdx] = vizZVector; curAttrIdx++; }
    else                 { viz2Attr[hostDivID][vizZVector] = -1; }*/
  }
  
  // Determine whether each data attribute is numeric or categorical
  if(!hostDivInitialized) {
    attrType[hostDivID] = new Array(numCtxtVars+numTraceAttrs);
    rangeMin[hostDivID] = new Array(numCtxtVars+numTraceAttrs);
    rangeMax[hostDivID] = new Array(numCtxtVars+numTraceAttrs);
  }
  for(var attrIdx=0; attrIdx<(numCtxtVars+numTraceAttrs); attrIdx++)
    attrType[hostDivID][attrIdx] = getAttrType(data, attrIdx, hostDivID, attr2Viz[hostDivID][attrIdx]==vizColor? colorAxisSize: spaceAxisSize);
  
  // If this is the first time we're trying to display this host div
  if(!hostDivInitialized) {
    // Set up this host
    setUpHostDiv(attrNames, minVals, maxVals, 
                 numCtxtVars, numTraceAttrs, hostDivID);
  }
  
  // plotDiv contains the 3d data plot
  var plotDiv = document.getElementById(hostDivID+"_plot");
  
  // Set up a listener for scrolling events to make sure that all scrolling over a
  // 3D plot controls the plot's view.
  if(!hostDivInitialized) {
    plotDiv.addEventListener("DOMMouseScroll", scrollListener);
    plotDiv.onscroll = scrollListener;
  }

  // Empty out the hostDiv
  if(!hostDivInitialized)
    plotDiv.innerHTML="";

  if(!initialized) {
    clock = new THREE.Clock();

    initialized = true;
  }
  
  ////////////
  // CAMERA //
  ////////////

  // If we have not yet created a camera for the scene, do so now
  var camera;
  //var SCREEN_WIDTH, SCREEN_HEIGHT;
  var screenSize;
  if(hostDivInitialized) {
    camera = cameras[hostDivID];
    screenSize = screenSizes[hostDivID];
  } else {
    // Set the view size in pixels (custom or according to window size)
    //var SCREEN_WIDTH = 400, SCREEN_HEIGHT = 300;
    //var SCREEN_WIDTH = window.innerWidth, SCREEN_HEIGHT = window.innerHeight;  
    //SCREEN_WIDTH = parent.document.getElementById("detail").offsetWidth*.8, SCREEN_HEIGHT = SCREEN_WIDTH*.75;
    var screenSize = getScreenSize(plotDiv);
    
    // camera attributes
    var VIEW_ANGLE = 45, ASPECT = screenSize["width"] / screenSize["height"], NEAR = 0.1, FAR = 20000;
    // set up camera
    camera = new THREE.PerspectiveCamera( VIEW_ANGLE, ASPECT, NEAR, FAR);
    // the camera defaults to position (0,0,0)
    //   so pull it back (z = 400) and up (y = 100) and set the angle towards the scene origin
    camera.position.set(750,750,750);
    camera.lookAt(scene.position);  
    
    cameras[hostDivID] = camera;
    screenSizes[hostDivID] = screenSize;
    
	// add the camera to the scene	
	scene.add(camera);
  }  
  
  //////////////
  // RENDERER //
  //////////////
  
  // If we have not yet created a renderer for the scene, do so now
  var renderer;
  if(hostDivInitialized) {
    renderer = renderers[hostDivID];
  } else {
    // create and start the renderer; choose antialias setting.
    if ( Detector.webgl )
      renderer = new THREE.WebGLRenderer( {antialias:true} );
    else
      renderer = new THREE.CanvasRenderer(); 
    
    renderer.setSize(screenSize["width"], screenSize["height"]);
    
    // attach renderer to the container div
    plotDiv.appendChild( renderer.domElement );
    plotDiv.style.width = screenSize["width"];
    plotDiv.style.height = screenSize["height"];
    
    renderers[hostDivID] = renderer;
  }

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
  
  var control;
  if(hostDivInitialized) {
    control = controls[hostDivID]
  } else {
    control = new THREE.OrbitControls( camera, renderer.domElement );
    controls[hostDivID] = control;
  }
  
  ///////////
  // LIGHT //
  ///////////
  
  // create a light
  if(!hostDivInitialized) {
    /*var light = new THREE.PointLight(0xffffff);
    light.position.set(0,250,0);
    scene.add(light);*/

    var ambientLight = new THREE.AmbientLight(0x111111);
    scene.add(ambientLight);
  }
  
  //////////////
  // GEOMETRY //
  //////////////
  
  // Identify the minimum separation between adjacent context values in each dimension
  var sep = ctxtSep(hostDivID, data, minVals, maxVals);

  for(d in data) { if(data.hasOwnProperty(d)) {
  //for(var d=0; d<100; d++) { {
    d = parseInt(d);
	
    if(!hostDivInitialized)
	  sphereShown[hostDivID][d] = false;
	
    // Determine whether all the coordinates are within the bounds set by the controls
    var withinBounds = true;
    for(var attrIdx=0; attrIdx < numCtxtVars+numTraceAttrs; attrIdx++) {
      if(//attr2Viz[hostDivID][attrIdx]>=0 &&
	     attrType[hostDivID][attrIdx]["type"] != "categorical" &&
	     (rangeMin[hostDivID][attrIdx] > data[d][attrIdx] || data[d][attrIdx] > rangeMax[hostDivID][attrIdx])) {
        withinBounds = false;
        break;
      }
    }

    // If the coordinates are within bounds
    if(withinBounds) {
      // Calculate the sphere's position in space (x,y,z visualization)
      var pos = [];
      for(var i=0; i<3; i++) {
        var xyzAttrIdx = viz2Attr[hostDivID][i]; // the index of the data attribute being visualized using this xyz visualization
    
        // If the current visual dimension has a data attribute mapped to it
        if(xyzAttrIdx>=0 && data[d][xyzAttrIdx]!==undefined) {
		   pos[i] = Math.floor(attrType[hostDivID][xyzAttrIdx]["scale"] ( data[d][xyzAttrIdx], 10 ));
        // Otherwise, set the value in this dimension to zero
        } else
          pos[i] = 0;
      }
      // Set the sphere's position
	  particles[hostDivID].vertices[d].setX(pos[0]);
	  particles[hostDivID].vertices[d].setY(pos[1]);
	  particles[hostDivID].vertices[d].setZ(pos[2]);
	  
//console.log(d+": "+pos[0]+", "+pos[1]+", "+pos[2]);

      // Update the particles's color and opacity
      /*particleAttr.color.value[ d ] = new THREE.Color("#000000");
	  particleAttr.alpha.value[ d ] = 1;*/
	  var colorAttrIdx = viz2Attr[hostDivID][vizColor]; // the index of the data attribute being visualized using color
	  
	  // If the data attribute is undefined, default to black
	  if(data[d][colorAttrIdx] === undefined)
        particleAttrs[hostDivID].color.value[ d ] = new THREE.Color("#000000");
	  else {
		var colorIdx=0; // The index of the sphere's color within the gradient
		  
		// If the gradient has more than one color, compute the index of this color based on this data attribute's scale
		if(maxVals[colorAttrIdx] - minVals[colorAttrIdx] > 0)
          colorIdx = Math.floor(attrType[hostDivID][colorAttrIdx]["scale"] ( data[d][colorAttrIdx], 10 ));
		
        particleAttrs[hostDivID].color.value[ d ] = new THREE.Color(colors[colorIdx]);
	  }

      // Show the particle in the scene if it is not already being shown
      if(!sphereShown[hostDivID][d]) {
        particleAttrs[hostDivID].alpha.value[ d ] = 1;
		
        // The sphere is now being shown
        sphereShown[hostDivID][d] = true;
      }

      // If any of the data dimensions are mapped to the vector
/*      if(viz2Attr[hostDivID][vizXVector]>=0 || viz2Attr[hostDivID][vizYVector]>=0 || viz2Attr[hostDivID][vizZVector]>=0) {
        // Direction vector
        // direction (normalized), origin, length, color(hex)
        var origin = new THREE.Vector3(pos[0], pos[1], pos[2]);
        var direction  = new THREE.Vector3(viz2Attr[hostDivID][vizXVector]>=0? data[d][viz2Attr[hostDivID][vizXVector]]: 0, 
                                           viz2Attr[hostDivID][vizYVector]>=0? data[d][viz2Attr[hostDivID][vizYVector]]: 0, 
                                           viz2Attr[hostDivID][vizZVector]>=0? data[d][viz2Attr[hostDivID][vizZVector]]: 0);
        //var direction = new THREE.Vector3().subVectors(terminus, origin).normalize();
        arrow.position.set(origin);
        arrow.setDirection(direction.normalize());
        arrow.setLength(direction.length());

        // Add the arrow to the scene if it is not already being shown
        if(!arrowShown[hostDivID][d]) {
          scene.add(arrow);
          // The arrow is now being shown
          arrowShown[hostDivID][d] = true;
        }
      }*/
      
    // If this sphere should not be shown because it is outside the selected bounds
    } else {
      // If the sphere is currently being shown, remove it and its associated arrow 
      // (if any) from the scene
      if(sphereShown[hostDivID][d]) {
        particleAttrs[hostDivID].alpha.value[ d ] = 0;
		
        // The sphere is now not being shown
        sphereShown[hostDivID][d] = false;
      }
      
/*      if(arrowShown[hostDivID][d]) {
        scene.remove(arrow);
        
        // The arrow is now not being shown
        arrowShown[hostDivID][d] = false;
      }*/
    }
  } }
  
  /*if(!hostDivInitialized) {
     particles[hostDivID].colors = particleColors[hostDivID];
  }*/
  
  // If we've already created axes for this scene, remove the old one before creating the new one
  if(hostDivInitialized)
    scene.remove(axes[hostDivID]);
  
  // The number of coordinates by which the visible axes on which value markers are shown are offset
  // from the axes that denote the origin.
  var visibleAxesOffsetFromOrigin = 60;
  if(!hostDivInitialized) {
    // create a set of coordinate axes to help orient user
    //    specify length in pixels in each direction
    var axis = new THREE.AxisHelper(spaceAxisSize);
    axes[hostDivID] = [axis];
    scene.add( axis );
    
    for(var i=0; i<3; i++) {
      var barMaterial = new THREE.MeshBasicMaterial();
      barMaterial.color = new THREE.Color("#444444");
	  var axisBar = new THREE.Mesh(new THREE.CubeGeometry(i==0? spaceAxisSize: 10, i==1? spaceAxisSize: 10, i==2? spaceAxisSize: 10), 
                                   barMaterial);
	  axisBar.position.set(i==0? spaceAxisSize/2: -visibleAxesOffsetFromOrigin, 
                           i==1? spaceAxisSize/2: -visibleAxesOffsetFromOrigin, 
                           i==2? spaceAxisSize/2: -visibleAxesOffsetFromOrigin);
	  scene.add(axisBar);
      axes[hostDivID].push(axisBar);
    }
  }
  
  /****************
   ***** AXES *****
   ****************/
  
  if(hostDivInitialized) {
    for(a in axisText[hostDivID]) { if(axisText[hostDivID].hasOwnProperty(a)) {
		scene.remove(axisText[hostDivID][a]);
	} }
	// Empty the array
	axisText[hostDivID] = [];
  }
  for(var i=0; i<3; i++) {
    var xyzAttrIdx = viz2Attr[hostDivID][i]; // the index of the data attribute being visualized using this xyz visualization
    
    axisText[hostDivID].push(
	      axisLabel(scene, axis, 
		           (i==0?spaceAxisSize+100:0), (i==1?spaceAxisSize+100:0), (i==2?spaceAxisSize+100:0), 
	                xyzAttrIdx>=0 ? attrNames[xyzAttrIdx]: 
					                (i==0?"X":(i==1?"Y":(i==2?"Z":"???"))),
				    22, {r:255, g:0, b:0, a:1.0}, 0));
	
    if(xyzAttrIdx>=0) {
      var steps = attrType[hostDivID][xyzAttrIdx]["stepsFunc"](10);
	  for(s in steps["vals"]) { if(steps["vals"].hasOwnProperty(s)) {
	    var coord = Math.floor(attrType[hostDivID][xyzAttrIdx]["scale"] ( steps["print"][s], 10 ));
//console.log("axis i="+i+", xyzAttrIdx="+xyzAttrIdx+" s="+s+": "+(i==0?coord:-50)+", "+(i==1?coord:-50)+", "+(i==2?coord:-50));
	  	axisText[hostDivID].push(
	      axisLabel(scene, axis, 
                    (i==0?coord: -visibleAxesOffsetFromOrigin-20), 
                    (i==1?coord: -visibleAxesOffsetFromOrigin-20), 
                    (i==2?coord: -visibleAxesOffsetFromOrigin-20), 
	  	            steps["print"][s],
					12, {r:0, g:0, b:255, a:1.0}, Math.PI / 8));
        
        var markMaterial = new THREE.MeshBasicMaterial();
        markMaterial.color = new THREE.Color("#444444");
	    var axisMark = new THREE.Mesh(new THREE.CubeGeometry(i==0? 5: 20, i==1? 5: 20, i==2? 5: 20), 
                                      markMaterial);
	    axisMark.position.set(i==0? coord: -visibleAxesOffsetFromOrigin, 
                              i==1? coord: -visibleAxesOffsetFromOrigin, 
                              i==2? coord: -visibleAxesOffsetFromOrigin);
	    scene.add(axisMark);
        axisText[hostDivID].push(axisMark);
	  } }
	}
  }
  
  if(!hostDivInitialized)
    // fog must be added to scene before first render
    scene.fog = new THREE.FogExp2( 0x9999ff, 0.00025 );

  animate();
  
  // Set the hostDiv to be resizable and set the callback that will recompute the 3d scatterplot's
  // size on each resizing.
  jQuery( "#"+hostDivID+"_plot" ).resizable({
      resize: function( event, ui) {
        screenSizes[plot2HostID[event.target.id]] = getScreenSize(event.target);
      }
    });
    
  /*document.addEventListener( 'mousemove', onDocumentMouseMove, false );
  
  // initialize object to perform world/screen calculations
	projector = new THREE.Projector();*/
}

// Return a {width:, height:} hash that identifies the size of the 3d scatter plot's area
function getScreenSize(hostDiv) {
  var screenSize = {width: hostDiv.offsetWidth,
                    height: hostDiv.offsetHeight};
  if(screenSize["width"] > screenSize["height"]*4/3) screenSize["width"] = screenSize["height"]*4/3;
  if(screenSize["height"] > screenSize["width"]*3/4) screenSize["height"] = screenSize["width"]*3/4;
  return screenSize;
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

  
  
  /*if(projector !== undefined) {
    for(hostDivID in cameras) { if(cameras.hasOwnProperty(hostDivID)) {
	    // create a Ray with origin at the mouse position
	    //   and direction into the scene (camera direction)
	    var vector = new THREE.Vector3( mouse.x, mouse.y, 1 );
	    projector.unprojectVector( vector, cameras[hostDivID] );
	    var ray = new THREE.Raycaster( cameras[hostDivID].position, vector.sub( cameras[hostDivID].position ).normalize() );
      
	    // create an array containing all objects in the scene with which the ray intersects
	    var intersects = ray.intersectObjects( scenes[hostDivID].children );
      
	    // INTERSECTED = the object in the scene currently closest to the camera 
	    //		and intersected by the Ray projected from the mouse position 	
	    
	    // if there is one (or more) intersections
	    if ( intersects.length > 0 )
	    {
	    	// if the closest object intersected is not the currently stored intersection object
	    	if ( intersects[ 0 ].object != INTERSECTED ) 
	    	{
	    	    // restore previous intersection object (if it exists) to its original color
	    		if ( INTERSECTED ) 
	    			INTERSECTED.material.color.setHex( INTERSECTED.currentHex );
	    		// store reference to closest object as current intersection object
	    		INTERSECTED = intersects[ 0 ].object;
	    		// store color of closest object (for later restoration)
	    		INTERSECTED.currentHex = INTERSECTED.material.color.getHex();
	    		// set a new color for closest object
	    		INTERSECTED.material.color.setHex( 0xffff00 );
	    	}
	    } 
	    else // there are no intersections
	    {
	    	// restore previous intersection object (if it exists) to its original color
	    	if ( INTERSECTED ) 
	    		INTERSECTED.material.color.setHex( INTERSECTED.currentHex );
	    	// remove previous intersection object reference
	    	//     by setting current intersection object to "nothing"
	    	INTERSECTED = null;
	    }
    } }
	}*/
	
  for(hostDivID in controls) { if(controls.hasOwnProperty(hostDivID)) {
    controls[hostDivID].update();
  } }
}

function render() 
{ 
  for(hostDivID in renderers) { if(renderers.hasOwnProperty(hostDivID)) {
    renderers[hostDivID].render( scenes[hostDivID], cameras[hostDivID] );
    // Reset the size of the renderer to keep it fixed
    renderers[hostDivID].setSize(screenSizes[hostDivID]["width"], screenSizes[hostDivID]["height"]);
  } }
}

function axisLabel(scene, axes, x, y, z, text, fontsize, borderColor, rotation)
{
 /*  var sphereGeometry = new THREE.SphereGeometry( 10, 1, 1);
   var sphereMaterial = new THREE.MeshBasicMaterial();
	 var sphere = new THREE.Mesh(sphereGeometry, sphereMaterial);
	 sphere.position.set(x,y,z);
	 var color = new THREE.Color("#000000");
   sphereMaterial.color = color;
	 scene.add(sphere);*/
	 
  var spritey = makeTextSprite(text, 
                               { fontsize: fontsize, 
							     borderColor: borderColor, 
								 backgroundColor: {r:255, g:100, b:100, a:0.8} },
							   x, y, z, rotation);
  scene.add( spritey );
  return spritey;

  /*var canvas1 = document.createElement('canvas');
  var context1 = canvas1.getContext('2d');
  context1.font = "Bold 12px Arial";
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
  
  return mesh1;*/
}

function makeTextSprite( message, parameters, x, y, z, rotation)
{
	if ( parameters === undefined ) parameters = {};
	
	var fontface = parameters.hasOwnProperty("fontface") ? 
		parameters["fontface"] : "Arial";
	
	var fontsize = parameters.hasOwnProperty("fontsize") ? 
		parameters["fontsize"] : 18;
	
	var borderThickness = parameters.hasOwnProperty("borderThickness") ? 
		parameters["borderThickness"] : 4;
	
	var borderColor = parameters.hasOwnProperty("borderColor") ?
		parameters["borderColor"] : { r:0, g:0, b:0, a:1.0 };
	
	var backgroundColor = parameters.hasOwnProperty("backgroundColor") ?
		parameters["backgroundColor"] : { r:255, g:255, b:255, a:1.0 };

	var spriteAlignment = THREE.SpriteAlignment.topLeft;
		
	var canvas = document.createElement('canvas');
	var context = canvas.getContext('2d');
	context.font = "Bold " + fontsize + "px " + fontface;
	//context.textAlign = "center";
    
    // get size data (height depends only on font size)
	var metrics = context.measureText( message );
	var textWidth = metrics.width;
	var textHeight = fontsize;
	
	/*canvas.height = textHeight;
	canvas.width = textWidth;*/
	//context.globalAlpha = 1;
	
	// background color
	context.fillStyle   = "rgba(" + backgroundColor.r + "," + backgroundColor.g + ","
								  + backgroundColor.b + "," + backgroundColor.a + ")";
	// border color
	context.strokeStyle = "rgba(" + borderColor.r + "," + borderColor.g + ","
								  + borderColor.b + "," + borderColor.a + ")";
	context.rotate( rotation );
	
	
	/* 
	context.beginPath();
	//context.rect(0, 0, textWidth, textHeight);
	context.rect(0, 0, canvas.width, canvas.height);
	//console.log("text: "+textWidth+", "+textHeight);
	//console.log("message="+message+", canvas: "+canvas.width+", "+canvas.height);
	context.fillStyle = 'yellow';
	context.fill();
	context.lineWidth = 7;
	context.strokeStyle = 'blue';
	context.stroke();*/
	
	// text color
	context.fillStyle = "rgba(0, 0, 0, 1.0)";
	context.fillText( message, borderThickness, textHeight/2 + borderThickness);
	//context.fillText( message, 0, 0);
	
	//canvas.width = 2000 + borderThickness;
	//canvas.height = textHeight;
	
	// canvas contents will be used for a texture
	var texture = new THREE.Texture(canvas) 
	texture.needsUpdate = true;

	var spriteMaterial = new THREE.SpriteMaterial(
		{ map: texture, useScreenCoordinates: false, alignment: spriteAlignment, transparent: true} );
	var sprite = new THREE.Sprite( spriteMaterial );
	sprite.scale.set(1000,500,1.0);
	//sprite.scale.set(10,5,1.0);

	// Set the sprite's position so that the center is at the x, y, z coordinates
	//sprite.position.set(x, y - textHeight, z + textWidth);
	sprite.position.set(x, y, z);
	
	return sprite;	
}

function makeImgSprite( imgFile, x, y, z)
{
	// canvas contents will be used for a texture
	var texture = THREE.ImageUtils.loadTexture( imgFile );
	texture.needsUpdate = true;

	var ballMaterial = new THREE.SpriteMaterial( { map: texture, useScreenCoordinates: false, alignment: THREE.SpriteAlignment.topLeft } );
	var sprite = new THREE.Sprite(ballMaterial);
	sprite.scale.set(20,20,1.0);
	
	// Set the sprite's position so that the center is at the x, y, z coordinates
	sprite.position.set(x, y, z);

	return sprite;	
}

// Identify the appropriate separation between adjacent context values in each dimension,
// which is a balance between showing the difference between the closest observations and
// keeping the overall distribution of observations visible
// ctxtMin/ctxtMas - the minimum and maximum values in each coordinate
function ctxtSep(hostDivID, data, ctxtMin, ctxtMax) {
  var coordList = []; // One list for each dimension of the context values in that dimension
  for(var i=0; i<3; i++) {
    var xyzAttrIdx = viz2Attr[hostDivID][i]; // the index of the data attribute being visualized using this xyz visualization
    coordList[xyzAttrIdx] = {};
  }
  
  // Populate coordList
  for(var i=0; i<3; i++)
    var xyzAttrIdx = viz2Attr[hostDivID][i]; // the index of the data attribute being visualized using this xyz visualization
    for(d in data) { if(data.hasOwnProperty(d)) {
      coordList[xyzAttrIdx][data[d][xyzAttrIdx]]=1;
  } }
  
  // Replace the hashes in coordList to be arrays of their keys, sorted so that adjacent 
  // values in the list correspond to numerically adjacent values
  for(var i=0; i<3; i++) {
    var xyzAttrIdx = viz2Attr[hostDivID][i]; // the index of the data attribute being visualized using this xyz visualization
    coordList[xyzAttrIdx] = Object.keys(coordList[xyzAttrIdx]).sort(function(a,b){return a - b});
  }
  
  // Iterate through coordList to identify the smallest distance between adjacent values
  var sep = {};
  for(var i=0; i<3; i++) {
    var xyzAttrIdx = viz2Attr[hostDivID][i]; // the index of the data attribute being visualized using this xyz visualization
    sep[xyzAttrIdx] = 1e100;
  }
  
  for(var i=0; i<3; i++) {
    var xyzAttrIdx = viz2Attr[hostDivID][i]; // the index of the data attribute being visualized using this xyz visualization

    for(var j in coordList[xyzAttrIdx]) { if(coordList[xyzAttrIdx].hasOwnProperty(j)) {
      if(j>0) {
        var diff = coordList[xyzAttrIdx][j] - coordList[xyzAttrIdx][j-1];
        sep[xyzAttrIdx] = (diff < sep[xyzAttrIdx] ? diff: sep[xyzAttrIdx]);
      }
    } }

    // If the separation is still 1e100, coordList[xyzAttrIdx] must only have one element. Thus, default sep[xyzAttrIdx] to 1.
    if(sep[xyzAttrIdx]==1e100) sep[xyzAttrIdx] = 1;
  }

  // Now adjust sep to ensure that the minimum separation we'll show is not too much smaller
  // than the overall size of the dataset in each dimension.
  for(var i=0; i<3; i++) {
    var xyzAttrIdx = viz2Attr[hostDivID][i]; // the index of the data attribute being visualized using this xyz visualization  
    sep[xyzAttrIdx] = Math.max((ctxtMax[xyzAttrIdx]-ctxtMin[xyzAttrIdx]) / 2e1, sep[xyzAttrIdx]);
  }
  
  return sep;
}

var slider2rangeID = {}; // Maps the IDs of slider divs to the IDs of their corresponding range divs
var slider2HostID  = {}; // Maps the IDs of slider divs to the IDs of the host div that contains them
var slider2Idx     = {}; // Maps the IDs of slider divs to their unique indexes within their host divs

var plot2HostID  = {}; // Maps the IDs of plot divs to the IDs of the host div that contains them

var attrDiv2Details = {}; // Maps the IDs of the attribute name divs to their corresponding details
                          // (hostID and index)

var vizDiv2Details = {}; // Maps the IDs of the visualization divs to their corresponding details
                         // (hostID and index in the vizNames array)

var viz2DivID = {}; // Maps the host div id and the index of each visualization to the ID of
                    // the div that contains it within the given host div.

// Sets up the structure of a given host div, including any controls and 3D content
function setUpHostDiv(attrNames, minVals, maxVals, 
                      numCtxtVars, numTraceAttrs, hostDivID) {
  var hostDiv = document.getElementById(hostDivID);
  hostDiv.style.position = "absolute";
  // Whenever the user clicks on the div, it'll go to the top of the z-order and become visible
  //hostDiv.onclick = function() { alert("div"); show3DViz(hostDivID); }
  hostDiv.addEventListener("click", function() { show3DViz(hostDivID); }, false);
  
  // Add to the host div separate divs for each attribute's sliders and a div 
  // for the 3d data plot
  
  var controlHTML = "";
  // Table that arrange the attribute sliders, the color gradient and the visualization boxes horizontally
  controlHTML += "<table  style=\"background-color:#ffffff; border-collapse: collapse;\">";
  controlHTML += "<col style=\"color: #ffffff; border: 1px solid #000000;\"/><col />";
  controlHTML += "<col style=\"color: #ffffff; border: 1px solid #000000;\"/><col />";
  controlHTML += "<col style=\"color: #ffffff; border: 1px solid #000000;\"/>";
  controlHTML += "<tr>";
  controlHTML += "<td style=\"font-size:22pt; text-decoration:underline;\"><img src=\"img/close.png\" onclick=\"hide3DViz('"+hostDivID+"'); event.stopPropagation();\"></td>\n";
  controlHTML += "<td style=\"font-size:22pt; text-decoration:underline;\">Visualizations</td>\n"
  controlHTML += "<td>&nbsp;</td>";
  controlHTML += "<td style=\"font-size:22pt; text-decoration:underline;\">Data Ranges</td>\n"
  controlHTML += "<td>&nbsp;</td>";
  controlHTML += "<td style=\"font-size:22pt; text-decoration:underline;\"><div id=\""+hostDivID+"_Color\"></div></td>\n";
  controlHTML += "</tr><tr><td></td>";
  
  // Div for the visualizations
  controlHTML += "<td><table>";
  
  viz2DivID[hostDivID] = {};
  for(var i in vizNames) {
    var divID = hostDivID+"_"+vizNames[i];
    controlHTML += "<tr><td><div id=\""+divID+"\" ondragover=\"allowDrop(event)\" ondrop=\"dropAttribute(event)\">"+
                   formatViz2Attr(vizNames[i], viz2Attr[hostDivID][i]>=0 ? attrNames[viz2Attr[hostDivID][i]] : "", divID+"_internal")+
                   "</div></td></tr>";
    // Record the details for both the ID of the visualization div and the ids of the span(s)
    // inside of it. Items may be dropped on any element within this div so we need to keep records
    // for all of them
    vizDiv2Details[divID]             = {hostDivID: hostDivID, index: i};
    vizDiv2Details[divID+"_internal"] = {hostDivID: hostDivID, index: i, id: divID};
    viz2DivID[hostDivID][i] = divID;
  }
  controlHTML += "</table></td>";
  controlHTML += "<td></td>";
  
  // Attribute Sliders
  controlHTML += "<td><table>";
  for(var i=0; i<(numCtxtVars+numTraceAttrs); i++) {
    controlHTML  += 
            "<tr><td><div draggable=\"true\" ondragstart=\"dragAttribute(event)\" id=\""+hostDivID+"_attr_"+i+"\" style=\"font-size:20pt;\">"+attrNames[i]+":</div></td>"+
            "<td><span id=\""+hostDivID+"_range_"+i+"\" ></span></td>"+
            "<td><div style=\"font-size:20pt;\"></div></td>"+
            "<td><div id=\""+hostDivID+"_slider_"+i+"\" style=\"width:500;\"></div></td>"+
            "</tr>";
    attrDiv2Details[hostDivID+"_attr_"+i]   = {hostDivID: hostDivID, index: i};
    slider2rangeID [hostDivID+"_slider_"+i] = hostDivID+"_range_"+i;
    slider2HostID  [hostDivID+"_slider_"+i] = hostDivID;
    slider2Idx     [hostDivID+"_slider_"+i] = i;
  }
  controlHTML += "</table></td>\n";
  controlHTML += "<td></td>";
  
  // Div into which we'll write the color gradient for the attribute chosen for the color visualization
  controlHTML += "<td width=\"20\">";
  controlHTML += "<div id=\""+hostDivID+"_gradient\"></div>";
  controlHTML += "</td>";
  
  controlHTML += "</tr></table>";
  
  // The div for the 3D scatter plot
  controlHTML += "<div id=\""+hostDivID+"_plot\" style=\"background-color:#ffffff; border-color:#000000; border-width=10px; border-style:solid; width:1000px; height:1000px\">Hi</div>";
  plot2HostID[hostDivID+"_plot"] = hostDivID;
  hostDiv.innerHTML = controlHTML;
  
  // Initialize the color display
  setColorDisplay(hostDivID);
  
  for(var i=0; i<(numCtxtVars+numTraceAttrs); i++) {
    //rangeMin[hostDivID][i] = attrType[hostDivID][i]["min"];
    //rangeMax[hostDivID][i] = attrType[hostDivID][i]["max"];
    var steps = attrType[hostDivID][i]["stepsFunc"](10);
  
    jQuery(function() {
      jQuery( "#"+hostDivID+"_slider_"+i ).slider({
        range: true,
        min: 0, 
        max: steps["count"]-1, 
        step: 1, 
        values: [0, steps["count"]-1],
        //animate: true,
        slide: function( event, ui ) {
          var hostDivID = slider2HostID[this.id];
		  var steps = attrType[hostDivID][slider2Idx[this.id]]["stepsFunc_AllData"](10);
          rangeMin[hostDivID][slider2Idx[this.id]] = steps["vals"][ui.values[0]];
          rangeMax[hostDivID][slider2Idx[this.id]] = steps["vals"][ui.values[1]];
          document.getElementById(slider2rangeID[this.id]).innerHTML = 
		    steps["print"][ui.values[0]] + "-" + steps["print"][ui.values[1]];
          
          // Refresh the 3D scatter plot
          refreshScatter3D(contents[hostDivID]["data"],
                           contents[hostDivID]["attrNames"],
                           contents[hostDivID]["minVals"],
                           contents[hostDivID]["maxVals"],
                           contents[hostDivID]["numCtxtVars"],
                           contents[hostDivID]["numTraceAttrs"],
                           hostDivID);
        }
      });
      rangeMin[hostDivID][i] = steps["vals"][0];//steps[jQuery( "#"+hostDivID+"_slider_"+i ).slider( "values", 0)];
      rangeMax[hostDivID][i] = steps["vals"][steps["count"]-1];//steps[jQuery( "#"+hostDivID+"_slider_"+i ).slider( "values", 1)];
      document.getElementById(hostDivID+"_range_"+i).innerHTML = 
              steps["print"][jQuery( "#"+hostDivID+"_slider_"+i ).slider( "values", 0)] + 
			  "-" + 
			  steps["print"][jQuery( "#"+hostDivID+"_slider_"+i ).slider( "values", 1)];
    });
  }
}

// Show/hide the 3d visualization in the given host div
function hide3DViz(hostDivID) {
  var hostDiv = document.getElementById(hostDivID);
  if(hostDiv) hostDiv.style.visibility = "hidden";
}
// To make sure that divs that have just been made visible are shown above all others, this counter maintains the maximum z value assigned to any div
var maxZ=1000;
function show3DViz(hostDivID) {
  var hostDiv = document.getElementById(hostDivID);
  if(hostDiv) {
    hostDiv.style.visibility = "visible";
    // If this div is not currently at the top of the z order, place it there
    if(hostDiv.style.zIndex != maxZ) {
      hostDiv.style.zIndex = maxZ;
      maxZ++;
    }
  }
}

// Sets the text of the div that indicates the data attribute currently mapped to the color visualization
function setColorText(hostDivID) {
  var attrNames = contents[hostDivID]["attrNames"];
  var colorDiv = document.getElementById(hostDivID+"_Color");
  var text = "Colors: ";
  if(viz2Attr[hostDivID][vizColor]>=0) text += attrNames[viz2Attr[hostDivID][vizColor]];
  colorDiv.innerHTML = text;
}

// Sets the color gradient
function setColorGradient(hostDivID) {
  var gradDiv = document.getElementById(hostDivID+"_gradient");
  var gradHTML = "";
  if(viz2Attr[hostDivID][vizColor] != -1) {
    gradHTML = "<table>";
	  var steps = attrType[hostDivID][viz2Attr[hostDivID][vizColor]]["stepsFunc"](10);
	  for(s in steps["vals"]) { if(steps["vals"].hasOwnProperty(s)) {
	    var colorIdx = Math.floor(attrType[hostDivID][viz2Attr[hostDivID][vizColor]]["scale"] ( steps["vals"][s], 10 ));
	    gradHTML += "<tr><td width=\"60\" height=\"20\" style=\"color:#ffffff; background-color:"+colors[colorIdx]+";\">"+
	                  steps["print"][s]+
 	  		        "</td></tr>";
    } }
    gradHTML += "</table>";
  }
  gradDiv.innerHTML = gradHTML;
}

// Sets the color display
function setColorDisplay(hostDivID) {
  setColorText(hostDivID);
  setColorGradient(hostDivID);
}

// Destroys a scene and all the objects within it
function destroyScene(scene) {
  jQuery.each(scene.__objects, function(idx, obj) {                               
    scene.remove(obj);                                                                                     
    if (obj.geometry) {                                                                                    
      obj.geometry.dispose();                                                                              
    }                                                                                                      
    if (obj.material) {                                                                                    
      if (obj.material instanceof THREE.MeshFaceMaterial) {                 
        jQuery.each(obj.material.materials, function(idx, obj) {                 
          obj.dispose();                                                                                   
        });                                                                                                
      } else {                                                                                             
        obj.material.dispose();                                                                            
      }                                                                                                    
    }                                                                                                      
    if (obj.dispose) {                                                                                     
      obj.dispose();                                                                                       
    }                                                                                                      
  });
}

function allowDrop(ev)
{
  ev.preventDefault();
}

function dragAttribute(ev)
{
  ev.dataTransfer.setData("attribute", ev.target.id);
}

function dropAttribute(ev)
{
  ev.preventDefault();
  // The div ID of the data attribute that was dropped on this visualization
  var attrDivID = ev.dataTransfer.getData("attribute");
  // The div ID of the div that contains the given 3D scatter plot
  var hostDivID = attrDiv2Details[attrDivID]["hostDivID"];
  // The index of the data attribute that was dropped
  var attrIdx = attrDiv2Details[attrDivID]["index"];
  // The index of the visualization onto which the data attribute was dropped
  var vizIdx = vizDiv2Details[ev.target.id]["index"];
  
  // The index of the prior data attribute that was mapped to this visualization
  var lastAttrIdx = viz2Attr[hostDivID][vizIdx];
  // The index of the prior visualization used for this data attribute
  var lastVizIdx = attr2Viz[hostDivID][attrIdx];
  // The div that contains the visual description of the prior visualization
  var lastVizDiv = undefined;
  if(lastVizIdx>=0) lastVizDiv = document.getElementById(viz2DivID[hostDivID][lastVizIdx]);
  
  // If the visualization mapping has changed
  if(vizIdx != lastVizIdx) {
    // Remove the previous mapping of data attribute to visualization and update the display
    attr2Viz[hostDivID][lastAttrIdx] = -1;
    viz2Attr[hostDivID][lastVizIdx] = -1;
    if(lastVizDiv !== undefined)
      lastVizDiv.innerHTML = formatViz2Attr(vizNames[lastVizIdx], "",
                                            viz2DivID[hostDivID][lastVizIdx]+"_internal");

    // Record and show the new mapping of data attribute to visualization
    attr2Viz[hostDivID][attrIdx] = vizIdx;
    viz2Attr[hostDivID][vizIdx]  = attrIdx;
    ev.target.innerHTML = formatViz2Attr(vizNames[vizIdx], contents[hostDivID]["attrNames"][attrIdx],
                                         viz2DivID[hostDivID][vizIdx]+"_internal");
    
    // If the color visualization has been modified, update its display
    if(vizIdx==vizColor || lastVizIdx==vizColor);
      setColorDisplay(hostDivID);
    
    // Refresh the 3D scatter plot
    refreshScatter3D(contents[hostDivID]["data"],
                     contents[hostDivID]["attrNames"],
                     contents[hostDivID]["minVals"],
                     contents[hostDivID]["maxVals"],
                     contents[hostDivID]["numCtxtVars"],
                     contents[hostDivID]["numTraceAttrs"],
                     hostDivID);
  }
}

// Returns the HTML to write into the table of visualization -> data attribute mappings
function formatViz2Attr(vizName, attrName, id) {
  return "<div id=\""+id+"\" style=\"color:#330000\; font-size:24pt;\">"+vizName+": "+attrName+"</div>";
}

// Given a value returns the number of digits after the decimal point required to encode it.
// If the value is > 1 the returned value is negative.
function numDigitsPrecision(val) {
  var cnt=0;
  if(val > 1) {
    while(val > 1) {
      val /= 10;
      cnt--;
    }
  } else if(val < 1) {
    cnt++;
    while(val < 1) {
      val *= 10;
      cnt++;
    }
  }
  
  return cnt;
}

// Given a value and the number of digits we wish to show in the value, as computed by 
// numDigitsPrecision(), returns its string representation.
function formatNum(val, digits) {
  if(digits<0) {
    // Remove digits least significant digits in val
	var mult=1; // Keeps track of the factor we've lost in val since the last power of 1000 
	            // (each power of 1000 gets a different magnitude suffix letter)
	var suffixes = ["K", "M", "G", "T", "P", "E", "Y", "Z"];
	var suffIdx=-1;
	for(var i=0; i>(0-digits); i++) {
	  // Remove a digit from val
	  val /= 10;
	  // If we can move on to the next suffix
	  if(mult%3==0 && suffIdx<suffixes.length-1) {
	    suffIdx++;
	    mult=1;
      // If we haven't reached the next suffix, add a power of 10 to mult
	  } else
	    mult *= 10;
	}
	
	// Truncate all the removed digits from Val and add in zeroes for the removed digits
	val = Number(val).toFixed(0) * mult;
    
	// Add in a suffix to represent any number of removed digits for which a suffix exists
	val = (suffIdx==-1? val : val+suffixes[suffIdx]);
	
	return val;
	
  } else if(digits>0) {
    return Number(Number(val).toPrecision(digits>10?10:digits)).toFixed(digits>10?10:digits);
  } else
    return val;
}

// Given an array of data tuples and the index of a given attribute within each tuple, checks
// if the attributes values are numeric or categorical. 
// If all values are either numeric or undefined returns {type: "numeric", min:, max:}
// If at least one defined value is non-numeric returns {type: "categorical", categories: hash with categories as keys }
//
// Returns a hash with fields:
// type: "numeric" or "categorical"
// scaleType: "log" or "linear"
// scale: function that maps scale inputs (range [Min - Max]) to scale outputs (range [minVizCoord - maxVizCoord])
// scaleFunc: function(numSteps)
//    numSteps - the number of discrete steps into which the scale should be divided
//    Returns an array of the values of the discrete steps
// If type == "numerical":
//   min/max: The minimum and maximum values of this attribute
//   minPositive: The minimum positive value of this attribute
// If type == "categorical":
//   numCategories: The total number of categories
//   categories: Array of the categories themselves
function getAttrType(data, attrIdx, hostDivID, axisSize) {
  var minVal=1e100, maxVal=-1e100;
  var minVal_AllData=1e100, maxVal_AllData=-1e100;
  var minPositiveVal=1e100;
  var minPositiveVal_AllData=1e100;
  var categories = {};
  var categoriesArray = [];
  var numCategories=-1;
  var isNumeric=true;

  // If attrType has not yet been initialized, initialize it based on all the data points we may observe
  if(rangeMin[hostDivID][attrIdx] === undefined) {
    for(d in data) { if(data.hasOwnProperty(d)) {
      // Skip tuples that we do not have a value for this data attribute
      if(data[d][attrIdx] === undefined) continue;
  	  
      // If the value is not already in categories, map the attribute index to a unique value, (we use the current size of categories)
      if(categories[data[d][attrIdx]] === undefined) {
        categories[data[d][attrIdx]] = categoriesArray.length;
        categoriesArray.push(data[d][attrIdx]);
      }
      
      // If we have not yet encountered a non-numeric value
      if(isNumeric) {
        // If this value is numeric, we update the min and max
        if(isNumber(data[d][attrIdx]) || String(data[d][attrIdx]).toLowerCase()=="nan" || String(data[d][attrIdx]).toLowerCase()=="inf") {
          if(isNumber(data[d][attrIdx])) {
            var val = parseFloat(data[d][attrIdx]); 
  	  	    
            if(val>0 && minPositiveVal > val) minPositiveVal = val;
            if(minVal > val) minVal = val;
            if(maxVal < val) maxVal = val;
          }
        // If this value is non-numeric we record that this data attribute is categorical
        } else {
          isNumeric = false;
        }
      }
    } }
    
    // Stretch the range to keep the min and max from being too close to 0 
    // (the range can only become larger as a result of this stretching)
    if(-1e-20<minVal && minVal < 0) minVal = -1e-20;
    if(0>minPositiveVal && minPositiveVal < 1e-20) minPositiveVal = 0;
    
    if(0>maxVal && maxVal < 1e-20) maxVal = 1e-20;
    if(-1e-20<maxVal && maxVal < 0) maxVal = 0;
	
    minVal_AllData = minVal;
    maxVal_AllData = maxVal;
    minPositiveVal_AllData = minPositiveVal;

    // If the attribute is numeric but consists of just NaNs and/or Infs, consider it categorical
    if(isNumeric && (maxVal<minVal))
      isNumeric = false;
	
    numCategories = categoriesArray.length;
  // If attrType has been initialized, update it to reflect only the currently selected data points
  } else {
    isNumeric = (attrType[hostDivID][attrIdx]["type"] == "numeric");
    if(isNumeric) {
      minVal = rangeMin[hostDivID][attrIdx];
      minVal_AllData = attrType[hostDivID][attrIdx]["min_AllData"];
      maxVal = rangeMax[hostDivID][attrIdx];
      maxVal_AllData = attrType[hostDivID][attrIdx]["max_AllData"];
      minPositiveVal = Math.max(minVal, attrType[hostDivID][attrIdx]["minPositive_AllData"]);
      minPositiveVal_AllData = attrType[hostDivID][attrIdx]["minPositive_AllData"];
    } else {
      categories = attrType[hostDivID][attrIdx]["categories"];
      categoriesArray = attrType[hostDivID][attrIdx]["categoriesArray"];
    }
    numCategories = attrType[hostDivID][attrIdx]["numCategories"];
  }
  
  var scaleR;
  if(isNumeric) {
    var s = createNumericScale(hostDivID, attrIdx, 0, axisSize, 
	                           minPositiveVal,         minVal,         maxVal,
							   minPositiveVal_AllData, minVal_AllData, maxVal_AllData,
							   numCategories);
    
	// Set the steps function that corresponds to all the data
	var stepsFunc_AllData;
	// If we haven't yet selected the appropriate function, use the one just returned
	if(rangeMin[hostDivID][attrIdx] === undefined) stepsFunc_AllData = s[3];
	// Otherwise, reuse the stepsFunc_AllData function previously computed since we need to keep this choice consistent across different range selections
	else                                           stepsFunc_AllData = attrType[hostDivID][attrIdx]["stepsFunc_AllData"];
	
	scaleR = {type: "numeric",
              scale: s[1], 
			  scaleType: s[0], 
			  stepsFunc: s[2],             stepsFunc_AllData: stepsFunc_AllData, 
			  min: minVal,                 min_AllData: minVal_AllData, 
			  max: maxVal,                 max_AllData: maxVal_AllData,
			  minPositive: minPositiveVal, minPositive_AllData: minPositiveVal_AllData,
			  numCategories: numCategories};
  } else {
    scaleR = {type: "categorical", 
	          scale: d3.scale.ordinal().domain(categoriesArray).rangePoints([0, axisSize]), 
			  scaleType: "categorical",
			  stepsFunc: function(numSteps) { return {vals: categoriesArray, print: categoriesArray, count:categoriesArray.length}; }, 
			  stepsFunc_AllData: function(numSteps) { return {vals: categoriesArray, print: categoriesArray, count:categoriesArray.length}; }, 
			  numCategories: numCategories, 
			  categoriesArray: categoriesArray,
			  categories: categories};
  }
  // Wrap the stepsFunc in scaleR so that we automatically pass this scaleR to the function.
  // This makes it possbile for other code to ignore the scaleR argument and only pass numSteps
  //scaleR["stepsFunc"] = function(numSteps) { scaleR["baseStepsFunc"](numSteps, scaleR); };
  
  return scaleR;
}

// numUniqueValues - the total number of unique numeric values taken by the data
// Returns a triple [scaleType, scale, scaleFunc]
// scaleType: "log" or "linear"
// scale: function that maps scale inputs (range [Min - Max]) to scale outputs (range [minVizCoord - maxVizCoord])
// scaleFunc: function(numSteps, scaleR)
//    numSteps - the number of discrete steps into which the scale should be divided
//    scaleR - a hash with keys min, max and minPositive that describe the dataset (created in getAttrType())
//    Returns an array of the values of the discrete steps
function createNumericScale(hostDivID, attrIdx, minVizCoord, maxVizCoord, 
                            MinPositive,         Min,         Max, 
							MinPositive_AllData, Min_AllData, Max_AllData, 
							numUniqueValues) {

  // If this axis is compatible with the log visualization and
  // If it is selected to be log or it is not specified and there is a huge range in the x coordinates, use a log scale
  if(Min>=0 && Max / MinPositive > 1e2) {
    // Scale
	var scaleFunc;
    var logScale = d3.scale.log()
                         .domain( [MinPositive, Max] )
                         .range( [ 0, 1 ] );

	// If this data attribute has zero observations, compress the non-zero elements of the scale and add a zero to it
	if(Min == 0) {
	  scaleFunc = 
	    function(val, numSteps) {
		  // If this value is a zero, put it at the minimal coordinate
          if(val == 0) return minVizCoord;
          // Otherwise, set the minimum visual coordinate such that we leave room for 0 values
          else {
		    var curVizMin = minVizCoord + (maxVizCoord - minVizCoord)/numLogSteps(numSteps, MinPositive, Min, Max);
			return curVizMin + logScale(val)*(maxVizCoord-curVizMin);
          }
		};
    // If there are no zero observations, the entire visual coordinate range can be used for non-zero values
	} else {
	  scaleFunc = 
	    function(val, numSteps) {
	      return minVizCoord + logScale(val)*(maxVizCoord-minVizCoord);
		};
	}
  
    return  ["log", 
	         // Scale
	         scaleFunc,
			 
			 // Steps - Visible Data
			 function(numSteps) { return logStepsFunc(numSteps, MinPositive,         Min,         Max); },
			 
			 // Steps - All Data
			 function(numSteps) { return logStepsFunc(numSteps, MinPositive_AllData, Min_AllData, Max_AllData); }
			 ];
  // Otherwise, use a linear scale
  } else {
    var domainMin, domainMax;
    // If the domain has only one element, widen it out to enable d3 to create a proper scale
    if(Min==Max) {
      domainMin = Min-1;
      domainMax = Min+1;
    } else {
      // If the region between Min and Max is far away from 0 or overlaps 0, show exactly this region
      if((Min<0 && Max>0) || (Min>0 && (Max-Min) < Max/2)) {
        domainMin = Min;
        domainMax = Max;
      // Otherwise, start the plotting region from 0
      } else {
        domainMin = 0;
        domainMax = Max;
      }
    }
	
    var linScale = d3.scale.linear()
                       .domain([ domainMin, domainMax ])
                       .range([ minVizCoord, maxVizCoord ]);
  
    return ["lin", 
	        // Scale
	        function(val, numSteps) {
			  return linScale(val);
		    },
			
			// Steps - Visible Data
			function(numSteps) { return linStepsFunc(numSteps, Min, Max); },
			
			// Steps - All Data
			function(numSteps) { return linStepsFunc(numSteps, Min_AllData, Max_AllData); }
	       ];
  }
}

function logStepsFunc(numSteps, MinPositive, Min, Max) {
  var Ratio = Math.ceil(Math.log(Max / MinPositive)/Math.log(10));

  // The multiplicative difference between adjacent step values
  var stepSize = Math.pow(10, Math.ceil(Math.log(Max / MinPositive) / 
                                        Math.log(10)) / 
                              (Math.min(Ratio, numSteps)));
  numSteps = numLogSteps(numSteps, MinPositive, Min, Max);
  
  var stepVals = []; // The raw values of all the steps
  var stepPrint = []; // The printable values of all the steps
  
  // If this data attribute has zero observations, add zero to the scale
  if(Min==0) {
    stepVals.push(0);
    stepPrint.push(0);
  }
  
  // Push all the non-zero values onto the scale
  var count = 0;
  var c=MinPositive;
  for(var i=0; i<numSteps; i++) {
    stepVals.push(c);
    stepPrint.push(Number(Number(c).toPrecision(1)).toExponential());
    count++;
	c*=stepSize;
  }
  // Ensure that the minimum and maximum step values are the true minimum and maximum
  stepVals[0]                 = Min;
  stepVals[stepVals.length-1] = Max;
  return {vals: stepVals, print: stepPrint, count: stepVals.length};
}

// Returns the number of steps that a logarithmic range should be divided into in increments of 10x factors, 
// with a maximum of maxNumSteps
function numLogSteps(maxNumSteps, MinPositive, Min, Max) {
  return Math.ceil(Math.min(maxNumSteps, 
                            Math.log(Max / MinPositive) / 
                               Math.log(10)));
}

function linStepsFunc(numSteps, Min, Max) {
  // The additive difference between the adjacent step values of adjacent displayed colors
  var stepSize = (Max - Min) / numSteps;
  if(stepSize == 0) return {vals: [Min], print: [Min], count: 1};
  
  // The number of digits of precision we need to show different value steps.
  var digits = numDigitsPrecision(stepSize);

  var stepVals  = []; // The raw values of all the steps
  var stepPrint = []; // The printable values of all the steps
  var count = 0;
  var c=Min;
  for(var i=0; i<numSteps; i++) {
    stepVals.push(c);
	stepPrint.push(formatNum(c, digits));
    count++;
	c+=stepSize;
  }
  // Ensure that the minimum and maximum step values are the true minimum and maximum
  stepVals[0]                 = Min;
  stepVals[stepVals.length-1] = Max;
	   
  return {vals: stepVals, print: stepPrint, count: stepVals.length};
}

