var h = new HashTable();
var minX = 20000;
var minY = 20000;
var maxX = 0;
var maxY = 0;

// Returns the absolute x/y position of the given element on the page
// as a {x:, y:} hash
function getLeftPos(el) {
    var r = el.getBoundingClientRect();   
    return {x:r.left, y:r.top};
}

function getRightPos(el) {
    var r = el.getBoundingClientRect();   
    return {x:r.right, y:r.bottom};
}

function unhideLoop(forID) {
  //alert("unhideLoop - forID = "+forID);
  for (var blockID in h.items) {
      if (h.hasItem(blockID)) {
          // get blockIDs for same forID loop
          //alert('key is: ' + blockID + ', value is: ' + h.items[blockID]);
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

   // alert("maxX = " + maxX);
   // alert("maxY = " + maxY);
   // alert("minX = " + minX);
   // alert("minY = " + minY);

  //Position parameters used for drawing the rectangle
  var x = minX - 10;
  var y = minY - 30;
  var width = maxX - minX + 10;
  var height = maxY - minY + 30;
  // var x = minX - 10;
  // var y = minY - 30;
  // var width = maxX - minX + 10;
  // var height = maxY - minY + 30;
  var canvas = document.createElement('canvas'); //Create a canvas element
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
  canvas.style.pointerEvents='none'; //Make sure you can click 'through' the canvas
  document.body.appendChild(canvas); //Append canvas to body element
  var ctx = canvas.getContext('2d');
  //Draw rectangle
  ctx.lineWidth = 10;
  ctx.rect(x, y, width, height);
  ctx.fillStyle = 'border:2px';
  ctx.strokeStyle = 'yellow';
  ctx.stroke();

}

function recordLoop(forID, blockID){
  //alert("forID = " + forID + " blockID = "+blockID);  
  h.setItem(blockID, forID);
}