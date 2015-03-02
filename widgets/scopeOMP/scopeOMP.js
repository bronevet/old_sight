var h = new HashTable();
var minX = 20000;
var minY = 20000;
var maxX = 0;
var maxY = 0;
var canvas, ctx;
var x, y, width, height;

var cli = 0;

function getLeftPos(el) {
    var r = el.getBoundingClientRect();   
    return {x:r.left, y:r.top};
}

function getRightPos(el) {
    var r = el.getBoundingClientRect();   
    return {x:r.right, y:r.bottom};
}

function unhideLoop(forID) {
  var fID = forID;
  for (var blockID in h.items) {
    if (h.hasItem(blockID)) {
      // get blockIDs for same forID loop
      var childDiv = document.getElementById("div"+blockID);
      var curLeftPos = getLeftPos(childDiv);
      var curRightPos = getRightPos(childDiv);

      if(minX > curLeftPos.x)
        minX = curLeftPos.x;
      if(minY > curLeftPos.y)
        minY = curLeftPos.y;

      if(maxX < curRightPos.x)
        maxX = curRightPos.x;
      if(maxY < curRightPos.y)
        maxY = curRightPos.y;     
    }
  }

  //Position parameters used for drawing the rectangle
  x = minX - 10;
  y = minY - 20;
  width = maxX - minX + 10;
  height = maxY - minY + 20;
    
  //Create a canvas element
  canvas = document.createElement('canvas'); 
  //Set canvas width/height
  canvas.style.width='100%';
  canvas.style.height='100%';
  //Set canvas drawing area width/height
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;
  //Position canvas
  canvas.style.position='absolute';
  canvas.style.left=0;
  canvas.style.top=0;
  canvas.style.zIndex=100000;
  canvas.style.pointerEvents='none'; 
  document.body.appendChild(canvas); 

  ctx = canvas.getContext('2d');
  //Draw rectangle
  ctx.lineWidth = 5;
  ctx.rect(x, y, width, height);
  ctx.fillStyle = 'border:2px';
  ctx.strokeStyle = 'green';
  ctx.stroke();


  for (var blockID in h.items) {
    if (h.hasItem(blockID)) {
      // get blockIDs for same forID loop
      var childDiv = document.getElementById("div"+blockID);

      childDiv.addEventListener('click', function() {         
          ctx.clearRect(0, 0, canvas.width, canvas.height);
          ctx.restore();

          //Create a canvas element
          var div = document.createElement('div'); 
          div.style.width = width;
          div.style.height = 40;
          div.style.background = "green";
          div.style.color = "black";
          div.innerHTML = "OpenMP For";
          div.style.position = "absolute";
          div.style.left = x;
          div.style.top = y;
          document.body.appendChild(div);

          hideLoop();
          div.addEventListener('click', function() { 
            div.parentNode.removeChild(div);            
            hideLoop();            
            //unhideLoop(fID);
            //Create a canvas element
            canvas = document.createElement('canvas'); 
            //Set canvas width/height
            canvas.style.width='100%';
            canvas.style.height='100%';
            //Set canvas drawing area width/height
            canvas.width = window.innerWidth;
            canvas.height = window.innerHeight;
            //Position canvas
            canvas.style.position='absolute';
            canvas.style.left=0;
            canvas.style.top=0;
            canvas.style.zIndex=100000;
            canvas.style.pointerEvents='none'; 
            document.body.appendChild(canvas); 

            ctx = canvas.getContext('2d');
            //Draw rectangle
            ctx.lineWidth = 5;
            ctx.rect(x, y, width, height);
            ctx.fillStyle = 'border:2px';
            ctx.strokeStyle = 'green';
            ctx.stroke();

          }, false);

      }, false);
    }
  }
    
}

function recordLoop(forID, blockID){  
  h.setItem(blockID, forID);
}

function hideLoop() {
  for (var blockID in h.items) {
    if (h.hasItem(blockID)) {
      var parentDiv = document.getElementById("div"+blockID);  
      // Hide the parent div
      var oldVisible = (parentDiv.className=='unhidden')
      parentDiv.className=(oldVisible?'hidden':'unhidden');
      
      objChanged(parentDiv);        


      // Get all the tables
      var childTbls = document.getElementsByTagName("table");
      var conditionTbl = new RegExp("table"+blockID+"[_-]([0-9]*[_-])*");
      for (var i=0; i<childTbls.length; i++){ 
        var child = childTbls[i];
        // Set the visibility status of each child table to be the same as its parent div
        //console.log("child.id="+child.id+" blockID="+blockID+" eq="+("table"+blockID==child.id));
        if ("table"+blockID==child.id)
          child.className=parentDiv.className;
      }
      
      var childDivs = document.getElementsByTagName("div");
      var conditionDiv = new RegExp("div"+blockID+"[_-]([0-9]*[_-])*");
      for (var i=0; i<childDivs.length; i++){
        var child = childDivs[i];
        if (child.id!=undefined && child.id.match(conditionDiv))
          objChanged(child);
      }
    }
  }
}