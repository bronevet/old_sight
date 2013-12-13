var colorSelectorFuncs = {};

function colorSelectorInit(selectorID, startColorRGB, endColorRGB) {
  colorSelectorFuncs[selectorID] = {};
  colorSelectorFuncs[selectorID]["values"] = {};
  colorSelectorFuncs[selectorID]["numValues"] = 0;
  colorSelectorFuncs[selectorID]["startColorRGB"] = startColorRGB;
  colorSelectorFuncs[selectorID]["endColorRGB"] = endColorRGB;
}

function colorSelector(selectorID, value, func) {
  // If this is the first time the current value has been recorded, map it to a fresh array
  if(!(value in colorSelectorFuncs[selectorID]["values"])) {
    colorSelectorFuncs[selectorID]["values"][value] = [];
    colorSelectorFuncs[selectorID]["numValues"]++;
  }
  // Save the given function under this value's key
  colorSelectorFuncs[selectorID]["values"][value].push(func);
}

function colorSelectorDisplay() {
  for(selectorID in colorSelectorFuncs) { if(colorSelectorFuncs.hasOwnProperty(selectorID)) {
    /*var startHSV   = rgb2hsv(colorSelectorFuncs[selectorID]["startColorRGB"][0], colorSelectorFuncs[selectorID]["startColorRGB"][1], colorSelectorFuncs[selectorID]["startColorRGB"][2]);
    var endHSV = rgb2hsv(colorSelectorFuncs[selectorID]["endColorRGB"][0],   colorSelectorFuncs[selectorID]["endColorRGB"][1],   colorSelectorFuncs[selectorID]["endColorRGB"][2]);*/
    var startRGB = colorSelectorFuncs[selectorID]["startColorRGB"];
    var endRGB = colorSelectorFuncs[selectorID]["endColorRGB"];
    
    // Advance colors across different values
    var sortedValues = Object.keys(colorSelectorFuncs[selectorID]["values"]).sort()
    for(valIdx in sortedValues) { //if(colorSelectorFuncs[selectorID]["values"].hasOwnProperty(val)) {
      var val = sortedValues[valIdx];
      /*var H = startHSV[0] + i*((endHSV[0]-startHSV[0])/colorSelectorFuncs[selectorID]["numValues"]);
      var S = startHSV[1] + i*((endHSV[1]-startHSV[1])/colorSelectorFuncs[selectorID]["numValues"]);
      var V = startHSV[2] + i*((endHSV[2]-startHSV[2])/colorSelectorFuncs[selectorID]["numValues"]);*/
      var RGB = [startRGB[0] + valIdx*((endRGB[0]-startRGB[0])/(colorSelectorFuncs[selectorID]["numValues"]-1)),
                 startRGB[1] + valIdx*((endRGB[1]-startRGB[1])/(colorSelectorFuncs[selectorID]["numValues"]-1)),
                 startRGB[2] + valIdx*((endRGB[2]-startRGB[2])/(colorSelectorFuncs[selectorID]["numValues"]-1))];
      //var RGB = hsv2rgb(H, S, V);
      var HTML = "#"+Dec2Hex(Math.round(RGB[0]*255))+
                     Dec2Hex(Math.round(RGB[1]*255))+
                     Dec2Hex(Math.round(RGB[2]*255));

      // Set all the spans with the current value to the current color
      for(i in colorSelectorFuncs[selectorID]["values"][val]) {
        colorSelectorFuncs[selectorID]["values"][val][i](HTML);
      }

    }//}
  }}
}

function Dec2Hex(dec) {
    var hex = dec.toString(16);
    return hex.length == 1 ? "0" + hex : hex;
}

// From http://jsres.blogspot.com/2008/01/convert-hsv-to-rgb-equivalent.html
function hsv2rgb(h,s,v) {
// Adapted from http://www.easyrgb.com/math.html
// hsv values = 0 - 1, rgb values = 0 - 255
var r, g, b;
var RGB = [];
if(s==0){
  RGB[0]=RGB[1]=RGB[2]=Math.round(v*255);
}else{
  // h must be < 1
  var var_h = h * 6;
  if (var_h==6) var_h = 0;
  //Or ... var_i = floor( var_h )
  var var_i = Math.floor( var_h );
  var var_1 = v*(1-s);
  var var_2 = v*(1-s*(var_h-var_i));
  var var_3 = v*(1-s*(1-(var_h-var_i)));
  if(var_i==0){ 
    var_r = v; 
    var_g = var_3; 
    var_b = var_1;
  }else if(var_i==1){ 
    var_r = var_2;
    var_g = v;
    var_b = var_1;
  }else if(var_i==2){
    var_r = var_1;
    var_g = v;
    var_b = var_3
  }else if(var_i==3){
    var_r = var_1;
    var_g = var_2;
    var_b = v;
  }else if (var_i==4){
    var_r = var_3;
    var_g = var_1;
    var_b = v;
  }else{ 
    var_r = v;
    var_g = var_1;
    var_b = var_2
  }
  //rgb results = 0 ÷ 255  
  RGB[0]=Math.round(var_r * 255);
  RGB[1]=Math.round(var_g * 255);
  RGB[2]=Math.round(var_b * 255);
  }
return RGB;  
};

// From http://www.javascripter.net/faq/rgb2hsv.htm
function rgb2hsv (r,g,b) {
 var computedH = 0;
 var computedS = 0;
 var computedV = 0;

 //remove spaces from input RGB values, convert to int
 var r = parseInt( (''+r).replace(/\s/g,''),10 ); 
 var g = parseInt( (''+g).replace(/\s/g,''),10 ); 
 var b = parseInt( (''+b).replace(/\s/g,''),10 ); 

 if ( r==null || g==null || b==null ||
     isNaN(r) || isNaN(g)|| isNaN(b) ) {
   alert ('Please enter numeric RGB values!');
   return;
 }
 if (r<0 || g<0 || b<0 || r>255 || g>255 || b>255) {
   alert ('RGB values must be in the range 0 to 255.');
   return;
 }
 r=r/255; g=g/255; b=b/255;
 var minRGB = Math.min(r,Math.min(g,b));
 var maxRGB = Math.max(r,Math.max(g,b));

 // Black-gray-white
 if (minRGB==maxRGB) {
  computedV = minRGB;
  return [0,0,computedV];
 }

 // Colors other than black-gray-white:
 var d = (r==minRGB) ? g-b : ((b==minRGB) ? r-g : b-r);
 var h = (r==minRGB) ? 3 : ((b==minRGB) ? 1 : 5);
 computedH = 60*(h - d/(maxRGB - minRGB));
 computedS = (maxRGB - minRGB)/maxRGB;
 computedV = maxRGB;
 return [computedH,computedS,computedV];
}