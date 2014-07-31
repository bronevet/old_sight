/*
** Control Flow Graph
** Copyright (C) 2014 - Hoa Nguyen
*/

import java.lang.Object;
import java.io.Writer;
import java.io.BufferedWriter;
import java.io.FileWriter;

String[] fontList;
PFont aFont;
int font_size = 10;
int statistic_viz = 0;
int iorel_info = 0;

// size of node
int nsi = 100;
// list of nodes with input and output information
String[] nodf;
String[] lnodes = new String[nsi];
// length of node
int lnodes_length = 5;
int cur_lnodes_length = 5;
// connection between nodes
String[] lconn;
// node relation (recursion or not)
String[] node_relation = new String[nsi];
// depth of nodes for graph layout
int[] node_depth = new int[nsi];
String[] depthList = new String[nsi];
int[] nodrel_len = new int[nsi];
int depth_width;
// visualization methods: sc3d, ccp, pcp, ... 
String[] vizMeth = new String[nsi];
// detail information of input and output relationship text
String[] ioInfo = new String[nsi];
String[] nVi;
// depth distance
float depth_distance;
int current_depth_length;

// node id
int[] nodeID = new int[nsi];
// names of node
String[] names = new String[nsi];
// number of inputs for each node
int[] numin = new int[nsi];
// number of outputs for each node
int[] numout = new int[nsi];
// containerID (parent)
int[] containerID = new int[nsi];
// number of connections for each node
int[] numconn = new int[nsi];
// io relation information for each node if has
String[] ionodeInfo = new String[nsi];
int[] ionodeInfo_height = new int[nsi];
// y coordinate of nodes
float[] xcnode = new float[nsi];
float[] ycnode = new float[nsi];
// width, height of nodes
float[] wnode = new float[nsi];
float[] hnode = new float[nsi];
float[] nodedepth_height = new float[nsi];
float xnode;
float ynode;
float nodeheight;
float nodewidth;
// to process the connections bewteen nodes 
int node_st, node_en;
int inp_st, out_en;
// depth of recursion
int depth_length = 0;
// rec = 1: tree view, rec = 2 for nest view
int rec_view = 0;
// meth = 1 for recursion tree, meth = 2 for nested loop, meth = 3 for sequence
int viewMeth = 3;
// to process for recursion
float recdis = 2.0;
// collapse: colap = 0: no collapse, colap = 1: collapse
int[] collapse = new int[nsi];
int node_col = -1;
// scaleFactor for zooming (1 for 100%)
float scaleFactor;
float updateWinsize;
int[] temp = new int[nsi];
  
void setup() 
{ 
  // size of window
  size(1200, 1100);
  //e8
  //size(1500, 800);
  background(255);
  smooth();
  // font size
  fontList = PFont.list();
  aFont = createFont(fontList[0], font_size, true);
  textFont(aFont);
  
  // initialize parameters
  scaleFactor = 1;
  // x,y coordinate for root node
  // for border
  //ynode = 50;
  viewMeth = 3;
  
  // node information: ModuleID:ModuleName:num_input:num_output:ContainterModuleID(parent)
  lnodes = loadStrings("node.txt");
  lnodes_length = lnodes.length;
  // connection between input and output of nodes   
  lconn = loadStrings("inout.txt");
  // visualization methods: sc3d, ccp, pcp, ... 
  vizMeth = loadStrings("dat.txt");
  if(vizMeth.length>0)
    statistic_viz = 1;
  // detail information of input and output relationship text
  ioInfo = loadStrings("ioInfo.txt");
  if(ioInfo.length>0)
    iorel_info = 1;
  
  // compoute node information 
  for(int i=0; i< lnodes_length; i++)
  {
    String[] inode = split(lnodes[i],':');
    // id of nodes
    nodeID[i] = int(inode[0]);
    // name of nodes
    names[i] = inode[1];
    // number of input
    numin[i] = int(inode[2]);
    // number of output
    numout[i] = int(inode[3]);
    // container id
    containerID[i] = int(inode[4]);
    // initialize depth node data
    //node_relation[i] = nodeID[i]+":"; 
    node_relation[i] = i+":"; 
    node_depth[i] = ""; 
    depthList[i] = "";
    temp[i] = 0;
    hnode[i] = 0;
    wnode[i] = 0;
    ycnode[i] = 0;
    xcnode[i] = 0;
    // if has recursion data - use default nest method
    if(containerID[i] >= 0)
      viewMeth = 2;
     
    collapse[i] = 0;  
    // compute io information 
    if(iorel_info == 1)
    {
      String[] ioMod = split(ioInfo[i],';'); 
      int numInf = int(ioMod[1]);
      if(numInf > 0)
      {
        if(lnodes_length > 8)
          ionodeInfo_height[i] = numInf+3;
        else
          ionodeInfo_height[i] = numInf+4;
        
        String[] io_Inf = split(ioMod[2],',');
        ionodeInfo[i] = "";
        for(int k=0; k<numInf; k++)
          ionodeInfo[i] += io_Inf[k]+"\n";
      }
    }
  }
  // compute node relation data
  // compute depth node data and depth_length
  
  for(int i=0; i< lnodes_length; i++)
    // recursion data
    if(containerID[i] > 0)
      for(int j=0; j< lnodes_length; j++)
        if(containerID[i] == nodeID[j])
          node_relation[j] += i+":"; 
  int nuDep = 0;
  depth_length = 0;
  depth_width = 0;
  
  for(int i=0; i<lnodes_length; i++)
  {
    String[] depnod = split(node_relation[i],":");
    nodrel_len[i] = depnod.length - 1;
    if(depth_width < nodrel_len[i])
      depth_width = nodrel_len[i];
    
    // recursion data
    if(containerID[i] < 0)
    {
      depth_length = nuDep+1;
      node_depth[i] = nuDep; 
      nuDep += 1;      
    }
    else
    {
      for(int j=0; j<lnodes_length; j++)
        if(containerID[i] == nodeID[j])
        {
          node_depth[i] = node_depth[j]+1;
          if(depth_length < (node_depth[i]+1))
            depth_length = node_depth[i]+1;
        }
    }
  }
 
  for(int i=0; i<lnodes_length; i++)
  {
    String[] depnod = split(node_relation[i],":");
      
    for(int k=0; k<nodrel_len[i]; k++)
      int ino = int(depnod[k]);
      if(containerID[ino]>0 && nodrel_len[ino]<2 && node_depth[ino]<(depth_length-2))
        temp[ino] = 1;
  }
  
  for(int m=0; m<lnodes_length; m++)
  { 
    if(containerID[m] < 0 && nodrel_len[m] < 2)
    {
     depthList[node_depth[m]] += m+ ":";  
    }
    else
    {
      if(containerID[m] <= 0)
        depthList[node_depth[m]] += m+ ":";
      String[] dep = split(node_relation[m],":");
      for(int k=1; k<nodrel_len[m]; k++)
      {
        int deno = int(dep[k]);
        depthList[node_depth[deno]] += deno+ ":"; 
        if(temp[deno] == 1)
        {
          for(int t=node_depth[deno]+1; t<depth_length; t++)
            depthList[t] += "-1:";
        }
      }
    }
    nodedepth_height[node_depth[m]] = 0;
  }
  // set up current depth length
  current_depth_length = depth_length;
  
  noLoop();
}
 
void draw() 
{
  background(255);
  nodeheight = 1.8*font_size;
  //nodeheight = height/(8*depth_length);
  //nodewidth = width/(depth_width + 1);
  if(lnodes_length > 8)
  {
    nodewidth = 28*font_size;
    depth_distance = 2*font_size;
  }
  else
  {
    nodewidth = 40*font_size;
    depth_distance = 6*font_size;
  }
  
  //depth_distance = height/60;
  xnode = depth_distance;
  // update the lnodes_length here for resize window 
  //nodeheight = 1.8*font_size;

  // update depth length
  int changedep = 1;
  for(int j=depth_length-1; j>=0; j--)
  {
    String[] depli = split(depthList[j], ":");
    for(int k=0; k<(depli.length-1);k++)
    {
      if(int(depli[k])>=0)
      {
        // update current depth length
        if(collapse[int(depli[k])] != 2)
        {
          changedep = 0;
        }
      }
    }
    
    if(changedep == 0)
    {
      current_depth_length = j+2;
      j = -1;
    }
  }
  depth_length = current_depth_length;
  
  // update depth width
  depth_width = 0;
  for(int i=0; i<lnodes_length; i++)
  {
    String[] depnod = split(node_relation[i],":");
    if((depnod.length-1) < (depth_length-2))
      if(depth_width < (depnod.length-1))
        depth_width = (depnod.length-1);
  }
  
  // width, height of nodes
  for(int i=0; i<lnodes_length; i++)
  { 
    // compute height of node
    hnode[i] = nodeheight + depth_distance;
    if(numin[i]>0 || numout[i]>0)
      hnode[i] += nodeheight;
    if(statistic_viz == 1)
      hnode[i] += nodeheight;
    if(iorel_info==1)
      hnode[i] += ionodeInfo_height[i]*font_size;
    
    // update nodedepth_height
    if(nodedepth_height[node_depth[i]] < hnode[i])
      nodedepth_height[node_depth[i]] = hnode[i];
     
    // compute wide of node
    wnode[i] = nodewidth;
  }
  
  for(int i=0; i<lnodes_length; i++)
  {   
    // position of nodes
    if(containerID[i] < 0)
    {
      if(viewMeth == 2)
        xcnode[i] = xnode+depth_length*font_size;
      else
        xcnode[i] = xnode + (width-xnode)/(depth_width+3);
    }
    else
    {
      for(int j=0; j<depth_length; j++)
      {
        String[] delis = split(depthList[j], ":");
        for(int k=0;k<(delis.length-1);k++)
          if(i == int(delis[k]))
          {
            if(viewMeth == 2)
              xcnode[i] = xnode + k*(width-xnode)/(delis.length-1) + j*font_size;
            else
              xcnode[i] = xnode + k*(width-xnode)/(delis.length-1);
          }
      }
    }
    if(node_depth[i]==0)
      ycnode[i] = 2*depth_distance;
    else
    {
      float ytmp=0;
      for(int m=0; m<(node_depth[i]-1);m++)
        ytmp += nodedepth_height[m];
      ycnode[i] = ytmp + nodedepth_height[node_depth[i]-1]+2*depth_distance;
    }
  }
    
  if(viewMeth == 2)
  {
    // update xcnode
    for(int i=0; i<lnodes_length; i++)
    {
      // update xcnode
      if(nodrel_len[i]>2)
      {
        String[] depnod = split(node_relation[i],":");
        xcnode[i] = xcnode[int(depnod[1])] - font_size;
      }
    }
    // update width of node
    for(int j=depth_length-3; j>=0; j--)
    {
      String[] depli = split(depthList[j], ":");
      for(int k=0; k<(depli.length-1);k++)
      {
        if(int(depli[k])>=0)
        {
          for(int i=0; i<lnodes_length; i++)
          {
            if(i == int(depli[k]))
            {
              String[] depnod = split(node_relation[i],":");
              if(nodrel_len[i]>2)
                wnode[i] =  xcnode[int(depnod[depnod.length-2])] - xcnode[int(depnod[1])] + wnode[int(depnod[depnod.length-2])] + font_size;
              else if(nodrel_len[i] == 2)
                wnode[i] = wnode[int(depnod[1])] + font_size;
            }
          }
        }
      }
    }
  }
  
  int newWid, newHei;
  if(lnodes_length > 8)
  {
    newWid = int(100+nodewidth*(depth_width+1));
    newHei = depth_distance;
  }
  else
  {
    newWid = int(100+nodewidth*(depth_width+2));
    newHei = depth_distance;
  }
  for(int k=0; k< (depth_length); k++)
  {
     newHei += int(hnode[k]);
  }
 
  
  if(scaleFactor == 1)
    size(newWid, newHei);
 
  if(viewMeth == 1 || viewMeth == 2)
    draw_methButton(viewMeth);
  // Draw nodes
  for(int i=(lnodes_length -1); i>=0; i--)
  {
    // draw nodes
    draw_nodes(i, xcnode[i], ycnode[i], wnode[i], hnode[i]);
  }
  
  // Draw connection between input and output of nodes
  for(int i=0; i<lconn.length; i++)
  {
    if(i<lconn.length)
    {
      String[] io_conn = split(lconn[i],',');
      numconn[i] = 0;
      for(int j=0; j<io_conn.length; j++)
      {
        numconn[i] += 1;
        String[] tconn = split(io_conn[j],':');
        
        // Node start
        String[] con_st = split(tconn[0],'_');  
        // Node end
        String[] con_en = split(tconn[1],'_');
        for(int k=0; k<lnodes_length; k++)
        {
          if(nodeID[k] == int(con_st[0]))
            node_st = k;
          if(nodeID[k] == int(con_en[0]))
            node_en = k;
        }
        
        inp_st = int(con_st[2]);
        out_en = int(con_en[2]);
        
        float xtmp_st = xcnode[node_st] + (inp_st+0.5)*wnode[node_st]/(numin[node_st]+numout[node_st]);
        float xtmp_en = xcnode[node_en] + (numin[node_en]+out_en-0.5)*wnode[node_en]/(numin[node_en]+numout[node_en]);
        
        // Draw connection bewteen two nodes
        if(collapse[node_st]!=2 && collapse[node_en]!=2)
        {
           // straight arrow connection
          if(node_depth[node_st] != node_depth[node_en])
            //draw_arrow(xtmp_st, ycnode[node_st]+nodeheight, xtmp_en, ycnode[node_en]); 
               draw_curvearrow(xtmp_st, ycnode[node_st]+nodeheight, xtmp_en, ycnode[node_en],3);
         
          // curve
          else if(node_en == node_st)
          {
            xtmp_en = xcnode[node_en] + (numin[node_en]+out_en+0.5)*wnode[node_en]/(numin[node_en]+numout[node_en]);
            draw_curvearrow(xtmp_st, ycnode[node_st], xtmp_en, ycnode[node_en],2);
          }
          else
          {
            if(xtmp_st < xtmp_en)
               xtmp_en = xcnode[node_en] + (numin[node_en]+out_en+0.5)*wnode[node_en]/(numin[node_en]+numout[node_en]);
            draw_curvearrow(xtmp_st, ycnode[node_st], xtmp_en, ycnode[node_en],2);
          }
        }
       }  
     }
   }
}

void mousePressed() 
{ 
  for(int i=0; i<lnodes_length; i++)
  {
    String[] vizInf = split(vizMeth[i],','); 
    String[] nVi = split(vizInf[2],':');
    int nvi_len = nVi.length;
    for(int k=0; k<nvi_len; k++)
    {
      float stY, enY, stX, enX;
      stY = ycnode[i]+2*nodeheight;
      enY = ycnode[i]+3*nodeheight;
      stX = (xcnode[i] + k*wnode[i]/nvi_len);
      enX = (xcnode[i] + (k+1)*wnode[i]/nvi_len);
      if(mouseX >= stX && mouseX <= enX && mouseY >= stY && mouseY <= enY)
        clickModule(vizInf[0], int(vizInf[1])+k*100, names[i], (100*mouseX/width+10), 100*mouseY/height);
    }
    
    if(viewMeth == 1 || viewMeth == 2)
    {
      String[] depno = split(node_relation[i], ":");
          
      if(mouseX >= (xcnode[i]) && mouseX <= (xcnode[i]+wnode[i]) && mouseY >= ycnode[i] && mouseY <= (ycnode[i]+2*nodeheight))
      {
        // collapse
        if(collapse[i] == 0)
        {
          if(i == int(depno[0]))
          {
            collapse[i] = 1;
            for(int m=1; m<(depno.length-1); m++)
              collapse[int(depno[m])] = 2;
          }
        }
        // expand
        else if(collapse[i] == 1)
        {
          collapse[i] = 0;
            for(int m=1; m<(depno.length-1); m++)
            {
              if(nodrel_len[int(depno[m])]>1)
                collapse[int(depno[m])] = 1;
              else
                collapse[int(depno[m])] = 0;
            }
        }
        redraw();
      }
      // hidden
      if(collapse[i] == 2)
      {
        for(int m=1; m<(depno.length-1); m++)
          collapse[int(depno[m])] = 2;
        redraw();
      }
    }
  }
 
  // check what method is selected for recursion visualization
  if(mouseX >= 10 && mouseX <= 90 && mouseY >= 5 && mouseY <= 25)
  {
    if(viewMeth == 1)
      viewMeth = 2;
    else
      viewMeth = 1;
    redraw();
  }
}

void mouseDragged() 
{
  // mouseX, mouseY
  // pmouseX, pmouseY
}

void mouseReleased() 
{

}

void keyPressed()
{
  if (key == 'i' || key == UP)
  {
    scaleFactor += 20;
    font_size += 1;
    if(width+int(scaleFactor)>100 && height+int(scaleFactor/10) > 100)
    {
      //size(width+int(scaleFactor), height+int(scaleFactor/10));
      size(width+int(scaleFactor), height+int(scaleFactor/5));
      background(255);
      smooth();
      redraw();
    }
    else
    {
      scaleFactor = 1;
      redraw();
    }
  }
  if (key == 'o'|| key == DOWN)
  {
    scaleFactor -= 20;
    font_size -= 1;
    if(width+int(scaleFactor)>100 && height+int(scaleFactor/10) > 100)
    {
      //size(width+int(scaleFactor), height+int(scaleFactor/10));
      size(width+int(scaleFactor), height+int(scaleFactor/5));
      background(255);
      smooth();
      redraw();
    }
    else
    {
      scaleFactor = 1;
      redraw();
    }
  }
  // esc key
  if (key == 27)
  {
    scaleFactor = 1;
    size(1500, 1200);
    background(255);
    smooth();
    redraw();
  }
}

void mouseWheel(MouseEvent e)
{
  scaleFactor += e.getAmount();
  if(width+int(scaleFactor)>100 && height+int(scaleFactor/10) > 100)
  {
    size(width+int(scaleFactor), height+int(scaleFactor/10));
    background(255);
    smooth();
    redraw();
  }
  else
      scaleFactor = 1;
}

void draw_methButton(int metho)
{
    stroke(0);
    strokeWeight(1);
    fill(255, 100, 255);  
    rect(10, 5, 80, 20);
    //textSize(15);
    textSize(font_size*1.5);
    fill(50);
    textAlign(CENTER);
    if(metho == 1)
      text("Nested Viz", 50, 22);
    else if(metho == 2)
      text("Tree Viz", 50, 22);
}


void draw_nodes(int inod, float xc, float yc, float wn, float hn)
{ 
  int numio = numin[inod] + numout[inod];
 
  // start draw node box
  textAlign(CENTER);
  textSize(font_size);
 
  if(collapse[inod] != 2)
  {
     // 1.1 draw inputs
    if(numin[inod] > 0)
    {
      for(int i=0; i<numin[inod]; i++)
      {
        stroke(0);
        strokeWeight(1);
        //colorMode(HSB);
        //fill(node_depth[inod]*255/(depth_length+1), 150, 255);
        fill(230);
        rect(xc+i*wn/numio, yc, wn/numio, nodeheight);
        
        fill(50);
        text("In"+i, xc + (i+0.5)*wn/numio, yc+nodeheight/1.5);
      }
    }
    
    // 1.2 draw outputs
    if(numout[inod] > 0)
    {
      for(int i=numin[inod]; i<numio; i++)
      {
        // draw outputs
        stroke(0);
        strokeWeight(1);
        //colorMode(HSB);
        //fill(node_depth[inod]*255/(depth_length+1), 150, 255);
        fill(200);
        rect(xc+i*wn/numio, yc, wn/numio, nodeheight);
        
        fill(50);
        text("Out"+(i-numin[inod]), xc + (i+0.5)*wn/numio, yc+nodeheight/1.5);
      }
    }
   
    // 2. draw name of module
    stroke(0);
    strokeWeight(1);
    colorMode(HSB);
    fill(node_depth[inod]*255/(depth_length+1), 150, 255);
    rect(xc, yc+nodeheight, wn, nodeheight);
    
    fill(50);
    text(names[inod], xc+wn/2, yc+nodeheight+nodeheight/1.5);
    
    // 3. draw visualization methods box
    String[] vizInf = split(vizMeth[0],','); 
    String[] nVi = split(vizInf[2],':');
    int nvilen = nVi.length;
    for(int k=0; k<nvilen; k++)
    {
      stroke(0);
      strokeWeight(1);
      colorMode(HSB);
      fill(k*255/nvilen, 20, 250);
      rect(xc + k*wn/nvilen, yc+2*nodeheight, wn/nvilen, nodeheight);
     
      fill(50);
      text(nVi[k], xc + (k+1/2)*wn/nvilen, yc+ 2*nodeheight + nodeheight/1.5);
    }
  
    // 4. draw input, output information
    if(iorel_info == 1)
    {
      stroke(0);
      strokeWeight(1);
      fill(250);
      rect(xc, yc+3*nodeheight, wn, (ionodeInfo_height[inod])*font_size);
     
      fill(50);
      text(ionodeInfo[inod], xc, yc+3*nodeheight + font_size/2, wn, (ionodeInfo_height[inod])*font_size);
    }
    // end of draw node box
  }
  
  if(collapse[inod] == 0)
  {
    // draw data recursion - nest if has
    if(viewMeth == 2)
    {
      if(nodrel_len[inod]>1)
      {
        // draw nest box
         noFill();
         float len = 0;
         for(int k=node_depth[inod]; k< (current_depth_length-1);k++)
         {
           len += nodedepth_height[k];
         }
         rect(xc, yc+hn-depth_distance, wn, len-hn);
      }
    }
    // draw data recursion - tree if has
    if(viewMeth == 1)
    {
      if(nodrel_len[inod]>1)
      {
        // draw arrow connection of tree view
        String[] depnod = split(node_relation[inod],":");
        for(int k=1; k<nodrel_len[inod];k++)
        {
          fill(100, 200, 255);
          draw_arrow(xcnode[int(depnod[0])] + wnode[int(depnod[0])]/2, ycnode[int(depnod[0])] + hnode[int(depnod[0])] - depth_distance, xcnode[int(depnod[k])]+wnode[int(depnod[k])]/2, ycnode[int(depnod[k])]);
        }      
      }
    }
    
  }
  if(collapse[inod] == 1)
  {
     // draw "+" to show that node is collapsed 
     textSize(2*font_size);
     fill(50);
     text("+", xc + 2*wn/3, yc+nodeheight+nodeheight/1.5); 
  }
}

// draw arrow from point (x1, y1) to point (x2, y2)
void draw_arrow(float x1, float y1, float x2, float y2) 
{
    line(x1, y1, x2, y2);
    pushMatrix();
    translate(x2, y2);
    float a = atan2(x1-x2, y2-y1);
    rotate(a);
       
    line(0, 0, -5, -5);
    line(0, 0, 5, -5);
    popMatrix();
} 

void draw_curvearrow(float x1, float y1, float x2, float y2, int leri) 
{ 
    float d = dist(x1, y1, x2, y2);
    float xc1, xc2, yc1, yc2;
   
    noFill();
    //colorMode(HSB);
    //stroke(25);
    stroke(255*2*d/width, 250, 250);
    strokeWeight(1);
    
    // left side
    if(leri == 1)
    {
        xc1 = x1 - d/4;
        xc2 = xc1;
        yc1 = y1 - d/3;
        yc2 = y1 - 2*d/3;
    }  
    // right side
    else if(leri == 0)
    {
        xc1 = x1 + d/6;
        xc2 = xc1;
        // go back
        if(node_st > node_en)
        {
          yc1 = y1 - d/3;
          yc2 = y1 - 2*d/3;
        }
        // go forward
        else
        {
          yc1 = y1 + d/3;
          yc2 = y1 + 2*d/3;
        }
    }
    else
    {
      if(y1==y2)
      {
        if(x1 < x2)
        {
          xc1 = 2*(x1+x2)/5;
          yc1 = y1 - d/15;
          xc2 = 3*(x1+x2)/5;
          yc2 = y1 - d/15;
        }
        else
        {
          xc1 = 3*(x1+x2)/5;
          yc1 = y1 - d/15;
          xc2 = 2*(x1+x2)/5;
          yc2 = y1 - d/15;
        }
      }
      else
      {
        xc1 = (x1+x2)/2;
        yc1 = y1;
        xc2 = x2;
        yc2 = (y1+y2)/2;
      }
    }
       
    // draw curve
    bezier(x1, y1, xc1, yc1, xc2, yc2, x2, y2);
    //arc(x1, y1, x2-x1, 2*nodeheight, -PI, 0);
    
    // draw arrow
    pushMatrix();
    float a;
    
    if(leri == 2)
    {
      translate(x1, y1);
      a = atan2(xc1-x1, y1-yc1);
    }
    else
    {
      translate(x2, y2);
      a = atan2(xc2-x2, y2-yc2);
    }
    
    rotate(a);
    line(0, 0, -5, -5);
    line(0, 0, 5, -5);
    popMatrix();
} 



