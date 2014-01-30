var initialized=false;
var clock;
// Maps each hostID to the properties of the 3D scene within it
var scenes = {}, cameras={}, controls={}, renderers={}, screenSizes={};

// Maps each hostID to the hash of the arguments passed to showScatter3D() to define its contents
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
var spheres = {};
var sphereMaterials = {};
var arrows = {};
var sphereShown = {};
var arrowShown = {};

// The span in pixels of the data in each dimension. The axis for that dimension will be scaled
// to make dat fit into this many pixels.
var axisSize = 500;

// The unique IDs of each visualization and their printable names
var vizNames = {};
var vizX       = 0; vizNames[vizX      ] = "X";
var vizY       = 1; vizNames[vizY      ] = "Y";
var vizZ       = 2; vizNames[vizZ      ] = "Z";
var vizColor   = 3; vizNames[vizColor  ] = "Color";
var vizXVector = 4; vizNames[vizXVector] = "XVector";
var vizYVector = 5; vizNames[vizYVector] = "YVector";
var vizZVector = 6; vizNames[vizZVector] = "ZVector";

// For each host div, maps the index of each visualization (x, y, z, color, vectorX, vectorY, vectorZ) 
// to the data dimensions assigned to it and vice versa.
var viz2Attr = {};
var attr2Viz = {};

function scrollListener(e) {
  //alert(e);  
  e.preventDefault();
}

function showScatter3D(data, attrNames, minVals, maxVals, 
                       numCtxtVars, numTraceAttrs, hostDivID) {
  // Records whether we've already initialized the state of this hostDiv
  var hostDivInitialized = true;
  
  var hostDiv = document.getElementById(hostDivID);
  
  // If this is the first time we're trying to display this host div
  if(typeof contents[hostDivID] === "undefined") {
    hostDivInitialized = false;
    
    // Cache the arguments to showScatter3D
    contents[hostDivID] = {data: data,
                           attrNames: attrNames,
                           minVals: minVals,         maxVals: maxVals,
                           numCtxtVars: numCtxtVars, numTraceAttrs: numTraceAttrs};
    
    // Initialize the spheres array for this host div
    spheres[hostDivID] = [];
    sphereMaterials[hostDivID] = [];
    arrows[hostDivID] = [];
    sphereShown[hostDivID] = [];
    arrowShown[hostDivID] = [];
    
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
    if(numCtxtVars >= 5 || numTraceAttrs >= 2) { viz2Attr[hostDivID][vizXVector] = curAttrIdx; attr2Viz[hostDivID][curAttrIdx] = vizXVector; curAttrIdx++; }
    else                 { viz2Attr[hostDivID][vizYVector] = -1; }
    if(numCtxtVars >= 6 || numTraceAttrs >= 3) { viz2Attr[hostDivID][vizYVector] = curAttrIdx; attr2Viz[hostDivID][curAttrIdx] = vizYVector; curAttrIdx++; }
    else                 { viz2Attr[hostDivID][vizYVector] = -1; }
    if(numCtxtVars >= 7 || numTraceAttrs >= 4) { viz2Attr[hostDivID][vizZVector] = curAttrIdx; attr2Viz[hostDivID][curAttrIdx] = vizZVector; curAttrIdx++; }
    else                 { viz2Attr[hostDivID][vizZVector] = -1; }
    
    // Determine whether each data attribute is numeric or categorical
    attrType[hostDivID] = [];
    for(var attrIdx=0; attrIdx<(numCtxtVars+numTraceAttrs); attrIdx++)
      attrType[hostDivID].push(getAttrType(data, attrIdx));
    
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
  }
  
  // add the camera to the scene
  if(!hostDivInitialized)
    scene.add(camera);
  
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
    
  // most objects displayed are a "mesh":
  //  a collection of points ("geometry") and
  //  a set of surface parameters ("material")  
 
  // Create the gradient to be used to color the tiles
  var colors = gradientFactory.generate({
      from: "#0000FF",
      to: "#FF0000",
      stops: 1000
  });
  
  // Show the color gradient 
  if(viz2Attr[hostDivID][vizColor]>=0) {
    var gradDiv = document.getElementById(hostDivID+"_gradient");
    var gradHTML = "<table>";
    
    // The number of separate colors we'll show in the gradient
    var numSteps = 10;
    // The difference between the values of adjacent displayed colors
    //var stepSize = (maxVals[viz2Attr[hostDivID][vizColor]] - minVals[viz2Attr[hostDivID][vizColor]])/numSteps;
    var stepSize = (attrType[hostDivID][viz2Attr[hostDivID][vizColor]]["max"] - 
                    attrType[hostDivID][viz2Attr[hostDivID][vizColor]]["min"]) / numSteps;
    //
    // The number of digits of precision we need to show different value steps.
    var digits = numDigitsPrecision(stepSize);
    
    for(var c=attrType[hostDivID][viz2Attr[hostDivID][vizColor]]["min"], colorIdx=0; 
        c<attrType[hostDivID][viz2Attr[hostDivID][vizColor]]["max"]; 
        c += stepSize, colorIdx+=1000/numSteps) {
      gradHTML += "<tr><td width=\"60\" height=\"20\" style=\"color:#ffffff; background-color:"+colors[colorIdx]+";\">";
      if(digits > 1 && digits < 5) gradHTML += c.toFixed(digits); //sprintf("%."+digits+"d", c);
      else if(digits < 2) gradHTML += c.toExponential(); //sprintf("%e", c);
      else                gradHTML += c;
      gradHTML += "</td></tr>";
    }
    gradHTML += "</table>";
    //alert(gradHTML);
    gradDiv.innerHTML = gradHTML;
    //alert(gradDiv.innerHTML);
  }
  
  // Identify the minimum separation between adjacent context values in each dimension
  var sep = ctxtSep(hostDivID, data, minVals, maxVals);
  var maxCoord = []; // Array that keeps track of the maximum value taken by any coordinate
  for(var i=0; i<3; i++) maxCoord[i] = -1e100;
  var maxAllviz2Attr = -1e100; // The maximum value among all values along all coordinates
  
  // Scaling factor to translate from the current data dimension to the coordinate system
  // of the 3D scatter plot to make sure that all the data in this dimension fits into
  // axisSize units in the plot. We use the same scaling factor for all the numeric 
  // spatial dimensions to make sure that if they are directly comparable to each other, we
  // do not skew one against the others.
  var dimScaling = 1e100;
  for(var i=0; i<3; i++) {
    var xyzAttrIdx = viz2Attr[hostDivID][i]; // the index of the data attribute being visualized using this xyz visualization
	
	// If the current axis in numeric
	if(attrType[hostDivID][viz2Attr[hostDivID][xyzAttrIdx]]["type"] == "numeric") {
		// If the current visual dimension has a data attribute mapped to it
		if(xyzAttrIdx>=0) {
		  // Update the common dimScaling to the the minimum of the scaling factor along each dimension
		  var curDimScaling = axisSize / (attrType[hostDivID][viz2Attr[hostDivID][xyzAttrIdx]]["max"] - attrType[hostDivID][viz2Attr[hostDivID][xyzAttrIdx]]["min"]);
		  if(dimScaling > curDimScaling) dimScaling = curDimScaling;
		}
	}
  }
  
  for(d in data) { if(data.hasOwnProperty(d)) {
      // Create / Load the sphere for the current observation
    var sphere, sphereMaterial, arrow;
    if(hostDivInitialized) {
      sphere = spheres[hostDivID][d];
      sphereMaterial = sphereMaterials[hostDivID][d];
      // If any of the data dimensions are mapped to the vector
      //if(viz2Attr[hostDivID][vizXVector]>=0 || viz2Attr[hostDivID][vizYVector]>=0 || viz2Attr[hostDivID][vizZVector]>=0)
        arrow = arrows[hostDivID][d];
    } else {
      var sphereGeometry = new THREE.SphereGeometry( 3, 8, 8);
      // use a "lambert" material rather than "basic" for realistic lighting.
      //   (don't forget to add (at least one) light!)
      //var sphereMaterial = new THREE.MeshLambertMaterial();
      sphereMaterial = new THREE.MeshBasicMaterial();
      sphereMaterials[hostDivID].push(sphereMaterial);
      
      sphere = new THREE.Mesh(sphereGeometry, sphereMaterial);
      spheres[hostDivID].push(sphere);
      
      // If any of the data dimensions are mapped to the vector
      //if(viz2Attr[hostDivID][vizXVector]>=0 || viz2Attr[hostDivID][vizYVector]>=0 || viz2Attr[hostDivID][vizZVector]>=0) {
        arrow = new THREE.ArrowHelper(new THREE.Vector3(0, 0, 0), new THREE.Vector3(0, 100, 0), 50, 0x884400);
        arrows[hostDivID].push(arrow);
      //}
      
      sphereShown[hostDivID][d] = false;
      arrowShown[hostDivID][d] = false;
    }
    
    // Determine whether all the coordinates are within the bounds set by the controls
    var withinBounds = true;
    for(var i=0; i < numCtxtVars+numTraceAttrs; i++) {
      if(rangeMin[hostDivID][i] > data[d][i] || data[d][i] > rangeMax[hostDivID][i]) {
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
        if(xyzAttrIdx>=0) {
		  // If the current axis in numeric
		  if(attrType[hostDivID][viz2Attr[hostDivID][xyzAttrIdx]]["type"] == "numeric") {
		    var curDimScaling = axisSize / (maxVals[xyzAttrIdx] - minVals[xyzAttrIdx]);
			pos[i] = dimScaling * (data[d][xyzAttrIdx] - minVals[xyzAttrIdx]);///sep[xyzAttrIdx];
		  } else if(attrType[hostDivID][viz2Attr[hostDivID][xyzAttrIdx]]["type"] == "categorical") {
		    pos[i] = attrType[hostDivID][viz2Attr[hostDivID][xyzAttrIdx]]["categories"][data[d][xyzAttrIdx]] * 
			        (axisSize / attrType[hostDivID][viz2Attr[hostDivID][xyzAttrIdx]]["numCategories"]);
		  }
        // Otherwise, set the value in this dimension to zero
        } else
          pos[i] = 0;
        
        // Update maxCoord
        maxCoord[i] = (pos[i] > maxCoord[i]? pos[i] : maxCoord[i]);
        maxAllviz2Attr = (maxCoord[i] > maxAllviz2Attr? maxCoord[i]: maxAllviz2Attr);
      }
      // Set the sphere's position
      sphere.position.set(pos[0], pos[1], pos[2]);

      // Update the sphere's color
      var colorIdx=0; // The index of the sphere's color within the gradient
      var colorAttrIdx = viz2Attr[hostDivID][vizColor]; // the index of the data attribute being visualized using color
      if(maxVals[colorAttrIdx] - minVals[colorAttrIdx] > 0) 
        colorIdx = Math.floor(((data[d][colorAttrIdx] - minVals[colorAttrIdx]) / (maxVals[colorAttrIdx]- minVals[colorAttrIdx]))*1000);
      var color = new THREE.Color(colors[colorIdx]);
      sphereMaterial.color = color;

      // Add the sphere to the scene if it is not already being shown
      if(!sphereShown[hostDivID][d]) {
        scene.add(sphere);
        // The sphere is now being shown
        sphereShown[hostDivID][d] = true;
      }

      // If any of the data dimensions are mapped to the vector
      if(viz2Attr[hostDivID][vizXVector]>=0 || viz2Attr[hostDivID][vizYVector]>=0 || viz2Attr[hostDivID][vizZVector]>=0) {
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
      }
      
    // If this sphere should not be shown because it is outside the selected bounds
    } else {
      // If the sphere is currently being shown, remove it and its associated arrow 
      // (if any) from the scene
      if(sphereShown[hostDivID][d]) {
        scene.remove(sphere);
        
        // The sphere is now not being shown
        sphereShown[hostDivID][d] = false;
      }
      
      if(arrowShown[hostDivID][d]) {
        scene.remove(arrow);
        
        // The arrow is now not being shown
        arrowShown[hostDivID][d] = false;
      }
    }
  } }
  
  if(!hostDivInitialized) {
    // create a set of coordinate axes to help orient user
    //    specify length in pixels in each direction
    var axes = new THREE.AxisHelper(maxAllviz2Attr);
    scene.add( axes );

    axisLabel(scene, axes, 160+axisSize/*maxCoord[0]*/, -20,         0,              viz2Attr[hostDivID][vizX]>=0 ? attrNames[viz2Attr[hostDivID][vizX]]: "X");
    axisLabel(scene, axes, 130,             axisSize/*maxCoord[1]*/, 0,              viz2Attr[hostDivID][vizY]>=0 ? attrNames[viz2Attr[hostDivID][vizY]]: "Y");
    axisLabel(scene, axes, 130,             -20,         10+axisSize/*maxCoord[2]*/, viz2Attr[hostDivID][vizZ]>=0 ? attrNames[viz2Attr[hostDivID][vizZ]]: "Z");

    // fog must be added to scene before first render
    scene.fog = new THREE.FogExp2( 0x9999ff, 0.00025 );
  }

  animate();
  
  // Se the hostDiv to be resizable and set the callback that will recompute the 3d scatterplot's
  // size on each resizing.
  //$( "#"+hostDivID+"_plot" ).resizable({
  $( hostDivID+"_plot" ).resizable({
      resize: function( event, ui) {
        screenSizes[plot2HostID[event.target.id]] = getScreenSize(event.target);
      }
    });
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

  for(hostDivID in controls) { if(controls.hasOwnProperty(hostDivID)) {
    controls[hostDivID].update();
  } }
  //stats.update();
}

function render() 
{ 
  for(hostDivID in renderers) { if(renderers.hasOwnProperty(hostDivID)) {
    renderers[hostDivID].render( scenes[hostDivID], cameras[hostDivID] );
    // Reset the size of the renderer to keep it fixed
    renderers[hostDivID].setSize(screenSizes[hostDivID]["width"], screenSizes[hostDivID]["height"]);
  } }
}

function axisLabel(scene, axes, x, y, z, text)
{
  var canvas1 = document.createElement('canvas');
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
  
  // Add to the host div separate divs for each attribute's sliders and a div 
  // for the 3d data plot
  
  var controlHTML = "";
  // Table that arrange the attribute sliders, the color gradient and the visualization boxes horizontally
  controlHTML += "<table  style=\"border-collapse: collapse\">";
  controlHTML += "<col style=\"color: #ffffff; border: 1px solid #000000;\"/><col />";
  controlHTML += "<col style=\"color: #ffffff; border: 1px solid #000000;\"/><col />";
  controlHTML += "<col style=\"color: #ffffff; border: 1px solid #000000;\"/>";
  controlHTML += "<tr>";
  controlHTML += "<td style=\"font-size:22pt; text-decoration:underline;\">Visualizations</td>\n"
  controlHTML += "<td>&nbsp;</td>";
  controlHTML += "<td style=\"font-size:22pt; text-decoration:underline;\">Data Ranges</td>\n"
  controlHTML += "<td>&nbsp;</td>";
  controlHTML += "<td style=\"font-size:22pt; text-decoration:underline;\">Colors: "+attrNames[viz2Attr[hostDivID][vizColor]]+"</td>\n"
  controlHTML += "</tr><tr>";
  
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
  controlHTML += "<div id=\""+hostDivID+"_plot\" style=\"border-color:#000000; border-width=10px; border-style:solid; height:600px\"></div>";
  plot2HostID[hostDivID+"_plot"] = hostDivID;
  hostDiv.innerHTML = controlHTML;
  
  rangeMin[hostDivID] = [];
  rangeMax[hostDivID] = [];
  for(var i=0; i<(numCtxtVars+numTraceAttrs); i++) {
    $(function() {
      $( "#"+hostDivID+"_slider_"+i ).slider({
        range: true,
        min: minVals[i],
        max: maxVals[i],
        step: (maxVals[i] - minVals[i])/100,
        values: [ minVals[i], maxVals[i] ],
        //animate: true,
        slide: function( event, ui ) {
          var hostDivID = slider2HostID[this.id];
          rangeMin[hostDivID][slider2Idx[this.id]] = ui.values[0];
          rangeMax[hostDivID][slider2Idx[this.id]] = ui.values[1];
          document.getElementById(slider2rangeID[this.id]).innerHTML = ui.values[0] + " - " + ui.values[1];
          
          // Refresh the 3D scatter plot
          showScatter3D(contents[hostDivID]["data"],
                        contents[hostDivID]["attrNames"],
                        contents[hostDivID]["minVals"],
                        contents[hostDivID]["maxVals"],
                        contents[hostDivID]["numCtxtVars"],
                        contents[hostDivID]["numTraceAttrs"],
                        hostDivID);
        }
      });
      rangeMin[hostDivID][i] = $( "#"+hostDivID+"_slider_"+i ).slider( "values", 0);
      rangeMax[hostDivID][i] = $( "#"+hostDivID+"_slider_"+i ).slider( "values", 1);
      document.getElementById(hostDivID+"_range_"+i).innerHTML = 
              $( "#"+hostDivID+"_slider_"+i ).slider("values", 0) + " - " + 
              $( "#"+hostDivID+"_slider_"+i ).slider("values", 1);
    });
  }
}

// Destroys a scene and all the objects within it
function destroyScene(scene) {
  $.each(scene.__objects, function(idx, obj) {                               
    scene.remove(obj);                                                                                     
    if (obj.geometry) {                                                                                    
      obj.geometry.dispose();                                                                              
    }                                                                                                      
    if (obj.material) {                                                                                    
      if (obj.material instanceof THREE.MeshFaceMaterial) {                 
        $.each(obj.material.materials, function(idx, obj) {                 
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
  
    // Refresh the 3D scatter plot
    showScatter3D(contents[hostDivID]["data"],
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
  return "<span id=\""+id+"\" style=\"color:#330000; font-size:20pt;\">"+vizName+": "+attrName+"</span>"; //"<span style=\"text-color:#000033; font-size:20pt;\">"+vizName+"</span>: "+
         //"<span style=\"text-color:#330000; font-size:20pt;\">"+attrName+"</span>";
}

// Given a value returns the number of digits after the decimal point required to encode it.
// If the value is > 1 the returned value is negative.
function numDigitsPrecision(val) {
  var cnt=1;
  if(val > 1) {
    while(val > 1) {
      val /= 10;
      cnt++;
    }
  } else if(val < 1) {
    while(val < 1) {
      val *= 10;
      cnt++;
    }
  }
  
  return cnt;
}

// Given an array of data tuples and the index of a given attribute within each tuple, checks
// if the attributes values are numeric or categorical. 
// If all values are either numeric or undefined returns {type: "numeric", min:, max:}
// If at least one defined value is non-numeric returns {type: "categorical", categories: hash with categories as keys }
function getAttrType(data, attrIdx) {
  var minVal=1e100, maxVal=-1e100;
  var categories = {};
  var numCategories = 0;
  var isNumeric=true;
  for(d in data) { if(data.hasOwnProperty(d)) {
    // Skip tuples that we do not have a value for this data attribute
    if(data[d][attrIdx] === undefined) continue;
    
	// If the value is not already in categories, map the attribute index to a unique value, (we use the current size of categories)
    if(categories[data[d][attrIdx]] === undefined) {
		categories[data[d][attrIdx]] = numCategories;
		numCategories++;
	}

    // If we have not yet encountered a non-numeric value
    if(isNumeric) {
      // If this value is numeric, we update the min and max
      if(isNumber(data[d][attrIdx])) {
        if(minVal > data[d][attrIdx]) minVal = parseFloat(data[d][attrIdx]);
        if(maxVal < data[d][attrIdx]) maxVal = parseFloat(data[d][attrIdx]);
      // If this value is non-numeric we record that this data attribute is categorical
      } else {
        isNumeric = false;
      }
    }
  } }

  if(isNumeric) return {type: "numeric", min: minVal, max: maxVal};
  else          return {type: "categorical", categories: categories, numCategories: numCategories};
}
