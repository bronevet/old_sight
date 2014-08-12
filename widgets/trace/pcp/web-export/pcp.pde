/*
** Parallel Coordinates Plots
** Copyright (C) 2014 - Hoa Nguyen
*/

import java.lang.Object;
import java.io.Writer;
import java.io.BufferedWriter;
import java.io.FileWriter;

float step = 1;
float eps = 0.2;
// number of dimensions
//int num = 9;
int num = 5;
float startP = 0.1;
float endP = 0.1;

String[] lines;
int dimLoc[] = new int[50];
float dimPos[] = new float[50];
int dim_order[] = new int[50];
float dimAng[] = new float[50];
float dim_min[] = new float[50];
float dim_max[] = new float[50];
int numdim = 0;
int d0, d1;

String[] fontList;
PFont aFont;

int selected = -1;
int dim_selected;
String dimNames[] = new String[50];

void setup() 
{ 
  // set up data parameters from trace.js and cscp.js file
  dataset();
  
  int numContext = int(numContextAttrs);
  int numTrace = int(numTraceAttrs);
  num = numContext+numTrace;
  
  // size of window
  size(1200, 300);
  background(255);
  smooth();
   
  // font size
  fontList = PFont.list();
  aFont = createFont(fontList[0], 10, true);
  textFont(aFont);
  
  // set up data from js file
  String dat = data;
  String[] list = split(dat, ',');
  
  float[] minVa = split(minVals, ',');
  float[] maxVa = split(maxVals, ',');
  int k = list.length/(numContext + numTrace);
  
  lines = new String[k];
  String aNames = attrNames;
  String[] listNam = split(aNames, ',');
  String[] listNames = new String[num];
  
  int index = 0;
  for(int i=0; i<k; i++)
  {
    lines[i] = ""; 
    //println("list["+i+"]="+list[i]);
    for(int j= 0; j<(numContext + numTrace); j++)
    {
      //println("minVa["+j+"]="+minVa[j]+"and maxVa["+j+"]="+maxVa[j]);
      if(list[i*(numContext + numTrace)+j].matches("\\d+") == true)
      {
        //println("false - list["+i*(numContext + numTrace)+j+"]="+list[i*(numContext + numTrace)+j]);
      
        if(minVa[j] != maxVa[j])
        {
          if(i == 0)
          {
            listNames[index] = listNam[j];
            index += 1;
          }
          
          lines[i] += list[i*(numContext + numTrace)+j];
          if(j < (numContext + numTrace - 1))
            lines[i] += " ";
        }
        else
        {
           if(i==0)
             num -= 1;
        }
      }
      else
      {
        if(i==0)
          num -= 1;
      }
    }
    //println("num = "+num);
    //println("lines["+i+"]="+lines[i]);
  }
  
  // set up dimensions' names
 
  for(int i=0; i<num; i++)
  {
    String[] listN = split(listNames[i],':');
    dimNames[i] = listN[listN.length-1];
  }
  
  
  // set up number and position of dimensions
  for(int i = 0; i < num; i++)
  {
    dimLoc[i] = i;
    dimPos[i] = lerp(0, width, (float(i)+0.5)/float(num));
    dim_order[i] = i;
  }
  
  // set up data from file
  //lines = loadStrings("physics.dat");
  
  // Finding minimum and maximum of each dimension
  String[] item0 = splitTokens(lines[0]);  
  numdim = item0.length;
  
  //println("numdim = "+numdim);
  //println("lines.length = "+lines.length);

  for(int i = 0; i < numdim; i += 1)
  {
    dim_min[i] = float(item0[i]);
    dim_max[i] = float(item0[i]);
    
    // find max and min of each dimension
    for(int j = 0; j < lines.length; j += step) 
    {
      String[] items = splitTokens(lines[j]);    
      float x = float(items[i]);    
      if(x < dim_min[i])
        dim_min[i] = x;    
      if(dim_max[i] < x)
        dim_max[i] = x;     
    }
    if( dim_min[i] == dim_max[i] )
        dim_max[i] = dim_min[i]+0.0001;
    //println("dimmin["+i+"]= "+dim_min[i]);
    //println("dimmax["+i+"]= "+dim_max[i]);
  }
  
  // End of finding min, max of each dimension
  
  noLoop();
}
 
void draw() 
{
  background(255);
  
  // parallel coordinates plots
   for(int i = 0; i < (num-1); i++)
  {
    //d0 = dimLoc[i];
    //d1 = dimLoc[i+1];

    // draw PCP lines
    draw_pcp(i,i+1);
        
    // draw axis
    stroke(#898484);
    strokeWeight(3);
    fill(#898484);

    if(i == selected)
      stroke(#FF0000);
      
    line(dimPos[i], startP*height, dimPos[i], (1.0-endP)*height);  
    if(i == (num-2))
    {
      if(selected == (num-1))
        stroke(#FF0000);  
      else
        stroke(#898484);
      line(dimPos[i+1], startP*height, dimPos[i+1], (1.0-endP)*height);
    }  
    
    fill(0);  
    textAlign(CENTER);
    String tex = "";
    //tex = "d"+dimLoc[i]+" - " + dimNames[i];
    //tex = "d"+dimLoc[i];
    //if(i == 0)
       tex = dimNames[i];
    
    text(tex, dimPos[i]+10, startP*height - 5);
    //text("d"+dimLoc[i], dimPos[i]+10, startP*height - 5);
    if(i == (num-2))
    {
      tex = dimNames[num-1];
      text(tex, dimPos[i+1]+10, startP*height - 5);
      //text("d"+int(dimLoc[num-1]), dimPos[i+1]+10, startP*height - 5);
    }
  }
}

// pcp
void draw_pcp(int v0, int v1)
{
  d0 = dimLoc[v0];
  d1 = dimLoc[v1];
    
  float c = pearCorr(d0, d1);

  // draw data points for CCP 
  fill(0);
  strokeWeight(1);
  // transparency = 50/255
  if(c < -eps)
    stroke(0, 0, -c*255, 50);
  else if(c > eps)
    stroke(c*255, 0, 0, 50);
  else
    stroke(0, 0, 0, 50);

  for(int i = 0; i < lines.length; i+=step)  
  {
    String[] items = splitTokens(lines[i]);    
    int v01 = d0;
    int v11 = d1;
    float xr = float(items[v01]);
    float yr = float(items[v11]);
    float x = lerp(startP*height, (1-endP)*height, (xr - dim_min[v01])/(dim_max[v01] - dim_min[v01]));
    float y = lerp(startP*height, (1-endP)*height, (yr - dim_min[v11])/(dim_max[v11] - dim_min[v11]));
    
    line(dimPos[v0], x, dimPos[v1], y);
  }
}

void mousePressed() 
{
  int closest = 0;
  
  for(int i = 1; i < num; i++)
  { 
    if(abs( dimPos[i] - mouseX ) < abs( dimPos[closest] - mouseX ))
       closest = i;
  }
  if(abs(dimPos[closest] - mouseX) < 10)
  {
    selected = closest;
    redraw();
  }
}

void mouseDragged() 
{
  // mouseX, mouseY
  // pmouseX, pmouseY

  if(selected != -1)
  {
      dimPos[selected] = mouseX;

      boolean repeat;
      do {
          repeat = false;
          if( selected > 0 && dimPos[selected] < dimPos[ selected-1 ])
          {
              int tp0 = dimLoc[selected];
              dimLoc[selected] = dimLoc[ selected-1 ];
              dimLoc[ selected-1 ] = tp0;
              
              float tp1 = dimPos[selected];
              dimPos[selected] = dimPos[ selected-1 ];
              dimPos[ selected-1 ] = tp1;
              
              String tp2 = dimNames[selected];
              dimNames[selected] = dimNames[selected - 1];
              dimNames[selected - 1] = tp2;
              
              selected--;
              repeat = true;
          }
          if( selected < (num-1) && dimPos[ selected ] > dimPos[ selected+1 ] )
          {
              int tp0 = dimLoc[selected];
              dimLoc[selected] = dimLoc[ selected+1 ];
              dimLoc[ selected+1 ] = tp0;
              
              float tp1 = dimPos[selected];
              dimPos[selected] = dimPos[ selected+1 ];
              dimPos[ selected+1 ] = tp1;
              
              String tp2 = dimNames[selected];
              dimNames[selected] = dimNames[selected + 1];
              dimNames[selected + 1] = tp2;
              
              selected++;
              repeat = true;
          }
      } while(repeat);

      for(int i = 0; i < num; i++)
      {
          if( i != selected )
              dimPos[i] = lerp(0, width, (float(i)+0.5)/float(num));
      }
      redraw();
  }
  else
  {
      int closest_left  = 0;
      int closest_right = num-1;
      for(int i = 1; i < (num - 1); i++){
          if( dimPos[i] < mouseX && abs( dimPos[i]-mouseX ) < abs( dimPos[closest_left]-mouseX ) ){
              closest_left = i;
          }
          if( dimPos[i] >= mouseX && abs( dimPos[i]-mouseX ) < abs( dimLoc[closest_right]-mouseX ) ){
              closest_right = i;
          }
      }
  }
}

void mouseReleased() 
{
  if(selected != -1)
  {
      dimPos[ selected ] = mouseX;
      for(int i = 0; i < num; i++)
          dimPos[i] = lerp(0, width, (float(i)+0.5)/float(num));
      
      selected = -1;
      redraw();
  }
}

// end pcp

void draw_scp()
{
  stroke(0,200,0);
  pushMatrix();
  for(int i = 0; i < lines.length; i+=step)  
  {
    String[] items = splitTokens(lines[i]);    
    float xr = float(items[d0]);
    float yr = float(items[d1]);
    float x = (xr - dim_min[d0])/(dim_max[d0] - dim_min[d0]);
    float y = (yr - dim_min[d1])/(dim_max[d1] - dim_min[d1]);
    point(width*y, height*x);
  }
  popMatrix();
}

float pearCorr(int v0, int v1)
{
  int n = lines.length;
  float meanx = 0, meany = 0;
  float num = 0, denx = 0, deny = 0;
  
  for(int j = 0; j < n; j += 1) 
  {
    String[] items = splitTokens(lines[j]);   
    float x = float(items[v0]);
    float y = float(items[v1]); 
    meanx += x;
    meany += y;
  }
  meanx = meanx/n;
  meany = meany/n;
  
  for(int j = 0; j < n; j += 1) 
  {
    String[] items = splitTokens(lines[j]);    
    float x = float(items[v0]);
    float y = float(items[v1]);
    num += (x - meanx)*(y - meany);
    denx += (x - meanx)*(x - meanx);
    deny += (y - meany)*(y - meany);
  }
  return num/(sqrt(denx)*sqrt(deny));
}

// compute x coordinate base on angle alpha of each dimension's position and defined radius of circle
float xcCompute(float alp, float r)
{
  return (-r*cos(alp*3.14/180)+1)/2;
}


// compute y coordinate base on angle alpha of each dimension's position and defined radius of circle
float ycCompute(float alp, float r)
{
  return (-r*sin(alp*3.14/180)+1)/2;
}

float AngleCompute(float xs, float ys)
{
    float sA = 0.0;
    float pi = 3.14;
    float cx = width/2;
    float cy = height/2;

    float ag = abs(atan(abs(ys - cy)/(xs - cx))*180/pi);
    
    if (xs <= cx && ys <= cy)
        sA = ag;
    else if (xs > cx && ys <= cy)
        sA = 180 - ag;
    else if(xs > cx && ys > cy)
        sA = ag + 180;
    else if (xs <= cx && ys > cy)
        sA = 360 - ag;
    
    return sA;
}



