// Returns the absolute x/y position of the given element on the page
// as a {x:, y:} hash
function getPos(el) {
    for (var lx=0, ly=0;
         el != null;
         lx += el.offsetLeft, ly += el.offsetTop, el = el.offsetParent);
    return {x:lx, y:ly};
}

// Maps clock names to the comparators to be used with each one
var comparators = {};

// Called to register the comparison function to be used for the clock with the given name
function registerComparator(clockName, compFunc) {
  comparators[clockName] = compFunc;  
}

// Uses all the functions available in comparators to compare the two given clocks to each other
function compareClocks(time1, time2) {
  // First check if time1 or time2 are placeholder minimim/maximum values used as IDs
  // at the head and tail node of the skip list
  
  // If time1/2==Number.MIN_VALUE then is it less-than anything else, except itself
  if(time1==Number.MIN_VALUE) {
    if(time2==Number.MIN_VALUE) return 0;
    else return -1;
  }
  if(time2==Number.MIN_VALUE) 
    return 1;
  
  // If time1/2==Number.MAX_VALUE then is it greater-than anything else, except itself
  if(time1==Number.MAX_VALUE) {
    if(time2==Number.MAX_VALUE) return 0;
    else return 1;
  }
  if(time2==Number.MAX_VALUE)
    return -1;
  
  // If both time1 and time2 are regular non-placeholder IDs
  
  // Count the number of clocks according to which time1 is less-than/greater-than/equal-to time2
  var numLess   =0;
  var numEqual  =0;
  var numGreater=0;
  
  for(clockName in comparators) { if(comparators.hasOwnProperty(clockName)) {
    // If the given clock exists in both times, it can be used to compare them
    if((clockName in time1) && (clockName in time2)) {
      // Increment a relation counter based on the relationship reported by this clock
      var rel = comparators[clockName](time1[clockName], time2[clockName]);
      if(rel<0)       numLess++;
      else if(rel==0) numEqual++;
      else            numGreater++;
    }
  }}
  
  // If time1 is strictly less-than time2, return -1
  if(numLess>0 && numEqual==0 && numGreater==0) return -1;
  // If time1 is strictly greater-than time2, return 1
  if(numGreater>0 && numEqual==0 && numLess==0) return 1;
  // If they're not strictly ordered then report that they're equal (return 0).
  // This is not a perfect solution since this relation is reflexive but not transitive but
  // we'll keep it for now until we upgrade our skiplists to deal with partial rather than
  // total orders.
  return 0;
}

// Skiplist that keeps all the divs for which insertOrderedDiv() is called ordered according 
// to their IDs. Uses compareClocks() to order IDs.
var orderedDivs = new SkipList(10, .25, compareClocks);
// The number of elements in orderedDivs
var orderedDivsSize=0;

function insertOrderedDiv(divRef, ID) {
  // If orderedDivs is empty, initialize it with the given div
  if(orderedDivsSize==0) {
    orderedDivs.Insert(ID, {divs: [divRef]});
  } else {
    var cur = orderedDivs.Search(ID);
    // If there is no entry in orderedDivs with an ID that is equal to or 
    // below ID, add this div to orderedDivs below the current least element
    if(cur == undefined) {
      //least = orderedDivs.least();
      //orderedDivs.insert({ID: ID, divs: [divRef], below:undefined, above: least, locs: [getPos(divRef)]});
      orderedDivs.Insert(ID, {divs: [divRef]});
    }
    // If the given ID has previously been observed, add this div to its record
    else {
      cur['divs'].push(divRef);
    }
  }
  
  orderedDivsSize++;
}

// Spatially lays out the ordered divs such that if one div causally follows another,
// it is placed lower down the page
// startID - the ID from which the layout process should start. Useful for redrawing the layout
//           after some piece of the output gets resized.
function layoutOrderedDivs(startID) {
    orderedDivs.Map(function(ID, cur) { 
      var newWatermark = watermark;
      for(var i=0; i<cur['divs'].length; i++) {
        cur['divs'][i].style.height = 0;
      }
    });
  
  //orderedDivs.Map(function(key, value) { alert(key+" => "+value); });
  var watermark=0;
  orderedDivs.Map(function(ID, cur) { 
      /*if(watermark<0) {
        watermark = Math.max.apply(null, cur['locs']);
      }*/
      
      var newWatermark = watermark;
      for(var i=0; i<cur['divs'].length; i++) {
        var curPos = getPos(cur['divs'][i]);
        if(curPos.y < watermark) {
          cur['divs'][i].style.height = watermark - curPos.y+10;
          //if(!(i in cur['origPos'])) { cur['origPos'][i] = curPos.y; }
          //console.log(i+": ID="+ID["scalarCausalClock"]+", Setting height of "+cur['divs'][i]+" to "+(watermark - curPos.y));
        } else
          newWatermark = Math.max(newWatermark, curPos.y);
      }
      
      watermark = newWatermark;
    });
}



