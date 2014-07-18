function getPos(el) {
    // yay readability
    for (var lx=0, ly=0;
         el != null;
         lx += el.offsetLeft, ly += el.offsetTop, el = el.offsetParent);
    return ly;
}

var orderedDivs = new SkipList(10);
var orderedDivsSize=0;

function insertOrderedDiv(divRef, ID) {
  ID = ID["scalarCausalClock"];
  // If orderedDivs is empty, initialize it with the given div
  if(orderedDivsSize==0) {
    orderedDivs.Insert(ID, {divs: [divRef], locs: [getPos(divRef)]});
  } else {
    var cur = orderedDivs.Search(ID);
    // If there is no entry in orderedDivs with an ID that is equal to or 
    // below ID, add this div to orderedDivs below the current least element
    if(cur == undefined) {
      //least = orderedDivs.least();
      //orderedDivs.insert({ID: ID, divs: [divRef], below:undefined, above: least, locs: [getPos(divRef)]});
      orderedDivs.Insert(ID, {divs: [divRef], locs: [getPos(divRef)]});
    }
    // If the given ID has previously been observed, add this div to its record
    else {
      cur['divs'].push(divRef);
      cur['locs'].push(getPos(divRef));
    }
  }
  
  orderedDivsSize++;
}


function layoutOrderedDivs() {
  //orderedDivs.Map(function(key, value) { alert(key+" => "+value); });
  var watermark=0;
  orderedDivs.Map(function(ID, cur) { 
      /*if(watermark<0) {
        watermark = Math.max.apply(null, cur['locs']);
      }*/
      
      var newWatermark = watermark;
      for(var i=0; i<cur['divs'].length; i++) {
        if(cur['locs'][i] < watermark) {
          cur['divs'][i].style.height = watermark - cur['locs'][i];
          console.log("Setting height of "+cur['divs'][i]+" to "+(watermark - cur['locs'][i]));
        } else
          newWatermark = Math.max(newWatermark, cur['locs'][i]);
      }
      
      watermark = newWatermark;
    });
}
