/*
** Correlation Coordinates Plots
** Copyright (C) 2014 - Hoa Nguyen
*/

import java.lang.Object;
import java.io.Writer;
import java.io.BufferedWriter;
import java.io.FileWriter;

float step = 1;
float eps = 0.2;

float axisW = 4;
float rangeLeaf = 0.15;
float radius = 0.8;
float radbr = 0.14;

/*
float axisW = 5;
float rangeLeaf = 0.15;
float radius = 0.8;
float radbr = 0.08;
*/

//float radbr = 0.15;
// number of dimensions
//int num = 9;
int num = 9;
float startP = 0.2;
float endP = 0.2;

String[] lines;
int dimLoc[] = new int[50];
String dimNames[] = new String[50];
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
float dim_selected_x;
float dim_selected_y;
String dimName_center;

void setup() 
{ 
  // set up data parameters from trace.js and cscp.js file
  dataset();
  
  int numContext = int(numContextAttrs);
  int numTrace = int(numTraceAttrs);
  num = numContext+numTrace;
  
  // size of window
  size(900, 900);
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
    //dimNames[i] = listNames[i];
    String[] listN = split(listNames[i],':');
    dimNames[i] = listN[listN.length-3]+ ":" + listN[listN.length-2]+ ":"+listN[listN.length-1];
  }
  
  // set up number and angle of dimensions
  for(int i = 0; i < num; i++)
  {
    //dimLoc[i] = i+11;
    dimLoc[i] = i;
    dimAng[i] = i*360/(num-1);
    if(i == (num - 1))
       dimAng[i] = 0; 
       
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
  dimName_center = dimNames[num-1];
  //noLoop();
  
}
 
void draw() 
{
  background(255);
    
  stroke(224, 255, 0, 30);
  fill(224, 255, 0, 30);
  strokeWeight(3);
  //ellipse(width/2-radius*width/2, height/2-radius*height/2, radius*width/2, radius*height/2);
  //ellipse((1-radius)*width/2, (1-radius)*height/2, radius*width/2, radius*height/2);
  ellipse(width/2, height/2, radius*width, radius*height);

  sf_focus(0);
  stroke(224, 255, 0, 30);
  fill(0);
  strokeWeight(3);

  textAlign(CENTER);
  //text("d"+dimLoc[num-1], width/2, height/2);
  //text("d"+dimLoc[num-1]+"-"+dimNames[num-1], width/2 - 10, height/2);
  text("d"+dimLoc[num-1]+"-"+dimNames[dimLoc[num-1]], width/2 - 10, height/2);

  sf_context(1);

  textAlign(LEFT);
  fontList = PFont.list();
  aFont = createFont(fontList[0],9, true);
  textFont(aFont);
  
}


void sf_focus(int sf)
{
  // draw focus view of snowflake vis
  for(int i = 0; i < (num-1); i++)
  {
    d0 = dimLoc[num - 1];
    d1 = dimLoc[i];
    float xc0 = width*xcCompute(dimAng[i], 0.1);
    float yc0 = height*ycCompute(dimAng[i], 0.1);
    float xc1 = width*xcCompute(dimAng[i], radius);
    float yc1 = height*ycCompute(dimAng[i], radius);
    float xt, yt;
    if(i == 0)
    {
      xt = width*xcCompute(dimAng[i], radius-0.12);
      yt = height*ycCompute(dimAng[i], radius-0.12);
    }
    else
    {
      xt = width*xcCompute(dimAng[i], radius-0.08);
      yt = height*ycCompute(dimAng[i], radius-0.08);
    }
    
    draw_ccp(d0, d1, xc0, yc0, xc1, yc1, xt, yt, sf);
    
    stroke(#898484);
    strokeWeight(1);
    fill(255);
    ellipse(xc1, yc1, 20,20);
  }
}

void sf_context(int sf)
{
  int m = int(num/2);
  float b = 180/(m-2);
  float b1;
  int end0, starti, endi, endj, cr, br;
  float xc0, yc0, xc1, yc1, xt, yt;
      
  if(num%2 == 0)
  {
    end0 = m - 1;
    end1 = m - 2;
    b1 = 180/(m-2);
    starti = m;
    endi = 2*m - 2;
    //endj = 1;
    endj = 0;
  }
  else
  {
    end0 = m-1;
    end1 = m-1;
    b1 = 180/(m-1);
    starti = m;
    endi = 2*m - 1;
    endj = 1;
  }
  
  // draw context view
  for(int i = 0; i <= end0; i++) 
  {
    cr = dimLoc[i];
    for(int j = 0; j <= end1; j++)
    { 
      br = dimLoc[i+j+1];
      xc0 = width*xcCompute(dimAng[i], radius);
      yc0 = height*ycCompute(dimAng[i], radius);
      xc1 = xc0 + width*xcCompute(dimAng[i]+b1*j-90, radbr) - width/2;
      yc1 = yc0 + height*ycCompute(dimAng[i]+b1*j-90, radbr) - height/2;
      xt = xc0 + width*xcCompute(dimAng[i]+b1*j-90, radbr+0.02) - width/2;
      yt = yc0 + height*ycCompute(dimAng[i]+b1*j-90, radbr+0.02) - height/2;
      
      draw_ccp(cr, br, xc0, yc0, xc1, yc1, xt, yt, sf);
    }
  }
  
  if(num != 5)
  {
    for(int i = starti; i <= endi; i++)
    {
      cr = dimLoc[i];
      for(int j = 0; j <= (i-m-endj); j++)
      { 
        br = dimLoc[j];
        xc0 = width*xcCompute(dimAng[i], radius);
        yc0 = height*ycCompute(dimAng[i], radius);
        if(num%2 == 0)
        {
          xc1 = xc0 + width*xcCompute(dimAng[i]+b*(2*m-i+j-2) - 90, radbr) - width/2;
          yc1 = yc0 + height*ycCompute(dimAng[i]+b*(2*m-i+j-2) - 90, radbr) - height/2;
          xt = xc0 + width*xcCompute(dimAng[i]+b*(2*m-i+j-2) - 90, radbr+0.02) - width/2;
          yt = yc0 + height*ycCompute(dimAng[i]+b*(2*m-i+j-2) - 90, radbr+0.02) - height/2;
        }
        else
        {
          xc1 = xc0 + width*xcCompute(dimAng[i]+b*(2*m-i+j-2) + b - 90, radbr) - width/2;
          yc1 = yc0 + height*ycCompute(dimAng[i]+b*(2*m-i+j-2) + b - 90, radbr) - height/2;
          xt = xc0 + width*xcCompute(dimAng[i]+b*(2*m-i+j-2) + b - 90, radbr+0.02) - width/2;
          yt = yc0 + height*ycCompute(dimAng[i]+b*(2*m-i+j-2) + b - 90, radbr+0.02) - height/2;
        }
        draw_ccp(cr, br, xc0, yc0, xc1, yc1, xt, yt, sf);
      } 
      
      for(int j = i+1; j <= endi; j++)
      { 
        br = dimLoc[j];
        xc0 = width*xcCompute(dimAng[i], radius);
        yc0 = height*ycCompute(dimAng[i], radius);
        xc1 = xc0 + width*xcCompute(dimAng[i]+b*(j-i-1) - 90, radbr) - width/2;
        yc1 = yc0 + height*ycCompute(dimAng[i]+b*(j-i-1) - 90, radbr) - height/2;
        xt = xc0 + width*xcCompute(dimAng[i]+b*(j-i-1) - 90, radbr+0.02) - width/2;
        yt = yc0 + height*ycCompute(dimAng[i]+b*(j-i-1) - 90, radbr+0.02) - height/2;
        
        draw_ccp(cr, br, xc0, yc0, xc1, yc1, xt, yt, sf);
      }
    }
  }
  else
  {
      int i = 3; 
      int j = 0;
      cr = dimLoc[i];
      br = dimLoc[j];
      xc0 = width*xcCompute(dimAng[i], radius);
      yc0 = height*ycCompute(dimAng[i], radius);
      
      xc1 = xc0 + width*xcCompute(dimAng[i]-90, radbr) - width/2;
      yc1 = yc0 + height*ycCompute(dimAng[i]-90, radbr) - height/2;
      xt = xc0 + width*xcCompute(dimAng[i]-90, radbr+0.02) - width/2;
      yt = yc0 + height*ycCompute(dimAng[i]-90, radbr+0.02) - height/2;
    
      draw_ccp(cr, br, xc0, yc0, xc1, yc1, xt, yt, sf);
      
      i = 2;
      j = 3;
      cr = dimLoc[i];
      br = dimLoc[j];
      
      xc0 = width*xcCompute(dimAng[i], radius);
      yc0 = height*ycCompute(dimAng[i], radius);
      xc1 = xc0 + width*xcCompute(dimAng[i] - 90, radbr) - width/2;
      yc1 = yc0 + height*ycCompute(dimAng[i] - 90, radbr) - height/2;
      xt = xc0 + width*xcCompute(dimAng[i] - 90, radbr+0.02) - width/2;
      yt = yc0 + height*ycCompute(dimAng[i] - 90, radbr+0.02) - height/2;
      
      draw_ccp(cr, br, xc0, yc0, xc1, yc1, xt, yt, sf);
  }
}


void draw_ccp(int v0, int v1, float xc0, float yc0, float xc1, float yc1, float xtext, float ytext, int sfn)
{
  float c = pearCorr(v0, v1);
  stroke(#898484);
  strokeWeight(1);
  fill(#898484);
    
  if(c < -eps)
  {
    beginShape(TRIANGLE_STRIP);
    vertex(xc0, yc0);
    vertex(xc1 - axisW, yc1 - axisW);
    vertex(xc1 + axisW, yc1 + axisW);
    endShape();
  }
  else if(c > eps)
  {
    beginShape(TRIANGLE_STRIP);
    vertex(xc1, yc1);
    vertex(xc0 - axisW, yc0 - axisW);
    vertex(xc0 + axisW, yc0 + axisW);
    endShape();
  }
  else
    line(xc0, yc0, xc1, yc1);  

  // draw data points for CCP 
  
  fill(0);
  //stroke(0,200,0);
  strokeWeight(3);
  if(c < -eps)
    stroke(0, 0, -c*255);
  else if(c > eps)
    stroke(c*255, 0, 0);
  else
    stroke(0, 0, 0);
  
  for(int i = 0; i < lines.length; i+=step)  
  {
    String[] items = splitTokens(lines[i]);    
    float xr = float(items[v0]);
    float yr = float(items[v1]);
    float x = (xr - dim_min[v0])/(dim_max[v0] - dim_min[v0]);
    float y = (yr - dim_min[v1])/(dim_max[v1] - dim_min[v1]);
    float x0 = lerp(xc0 + (xc1-xc0)*startP, xc1 - (xc1-xc0)*endP, x);
    float x1 = lerp(xc0 + (xc1-xc0)*startP, xc1 - (xc1-xc0)*endP, y);    
    float y0 = lerp(yc0 + (yc1 - yc0)*startP, yc1 - (yc1 - yc0)*endP, x);
    float y1 = lerp(yc0 + (yc1 - yc0)*startP, yc1 - (yc1 - yc0)*endP, y);
   
    if(c < -eps)
    {
      float xm = (xc0 + (xc1-xc0)*startP+xc1 - (xc1-xc0)*endP)/2;
      float ym = (yc0 + (yc1 - yc0)*startP + yc1 - (yc1 - yc0)*endP)/2;
      point(x0 +(y1+y0-2*ym)*rangeLeaf, y0 - (x1 + x0 - 2*xm)*rangeLeaf);
    }
    else
      point(x0 +(y1 - y0)*rangeLeaf, y0 - (x1 - x0)*rangeLeaf);            
  }
   
  fill(0);  
  textAlign(CENTER);
  if(sfn == 0)
    text("d"+v1+ "-"+dimNames[v1], xtext, ytext);
  else
    text("d"+v1, xtext, ytext);
  
}

 
void mousePressed() 
{
  int closest = 0;
  float xclosest = 0.0, yclosest = 0.0;
  float xc = 0.0, yc = 0.0;
  float xt0, yt0;

  float selpr = sqrt((mouseX - width/2)*(mouseX - width/2) + (mouseY - height/2)*(mouseY - height/2));
  xt0 = width*xcCompute(0, radius);
  yt0 = height*ycCompute(0, radius);
  float rad = sqrt((xt0 - width/2)*(xt0 - width/2) + (yt0 - height/2)*(yt0 - height/2));
  
  // outside small leaves
  if (abs(selpr - rad) < 60)
  {
    for(int i = 0; i < num; i++)
    {  
      xc = width*xcCompute(dimAng[i], radius);
      yc = height*ycCompute(dimAng[i], radius);
      xclosest = width*xcCompute(dimAng[closest], radius);
      yclosest = height*ycCompute(dimAng[closest], radius);

      if( ( abs( xc - mouseX ) <= abs( xclosest - mouseX ) ) && (abs( yc - mouseY ) <= abs( yclosest - mouseY ) ) && abs(AngleCompute(xc, yc) - AngleCompute(mouseX, mouseY)) <= abs(AngleCompute(xclosest, yclosest) - AngleCompute(mouseX, mouseY)))
         closest = i;
    }
    if (closest == (num-1))
      closest = 0;
 
    if( ( abs( xclosest - mouseX ) < 100 ) && ( abs( yclosest - mouseY ) < 100 ))
    {
      selected = closest;
      dim_selected = closest;
      dim_selected_x = mouseX;
      dim_selected_y = mouseY;
     
      // swap selected dim with (num-1)th dim.
      int tp0 = dimLoc[dim_selected];
      dimLoc[dim_selected] = dimLoc[num-1];
      dimLoc[num-1]= tp0;
      
      // swap labels
      /*
      String tp1 = dimNames[dim_selected];
      dimNames[dim_selected] = dimNames[num-1];
      dimNames[num-1]= tp1;
      */
 
      //for(int i = 0; i < num; i++)
      //   dimAng[i] = i*360/(num-1);
      
      redraw();
    }
  }
}

void mouseDragged() 
{
  // mouseX, mouseY
  // pmouseX, pmouseY
  /*
  if(selected != -1)
  {
      dimAng[dim_selected] = AngleCompute(mouseX, mouseY);
      dim_selected_x = mouseX;
      dim_selected_y = mouseY;

      // when selected dim is near the center
      float selpr = sqrt((mouseX - width/2)*(mouseX - width/2) + (mouseY - height/2)*(mouseY - height/2));

      // when selected dim is near the center
      if(abs(selpr) < 80)
      {
          // swap between dim_selected and dim - 1
          int tp0 = dimLoc[dim_selected];
          dimLoc[dim_selected] = dimLoc[num-1];
          dimLoc[num-1]= tp0;
          
          int tp = dim_order[dim_selected];
          dim_order[dim_selected] = dim_order[num-1];
          dim_order[num-1]= tp;
          
          // sorting
          for (int m = 0;  m < num - 2;  m++)
          {
              for (int j = 0;  j < num -2 - m;  j++)
              {
                  if (dim_order[j] > dim_order[j+1])
                  {
                      // swap between dim[j] and dim[j+1]
                      tp0 = dimLoc[dim_selected];
                      dimLoc[dim_selected] = dimLoc[num-1];
                      dimLoc[num-1]= tp0;
          
                      tp = dim_order[j];
                      dim_order[j] = dim_order[j+1];
                      dim_order[j+1]= tp;
                  }
              }
          }

          dim_selected = num - 1;
          dim_selected_x = radius;
          dim_selected_y = 0.0;
      }

      for(int i = 0; i < num; i++)
         dimAng[i] = i*360/(num-1);
      
      redraw();
  }
  */
}

void mouseReleased() 
{
  /*
  if(selected != -1)
  {
    // update dimLoc at dimensions
      for(int i = 0; i < num; i++)
          dimAng[i] = i*360/(num-1);
      
      selected = -1;
      redraw();
  }
  */
}

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


