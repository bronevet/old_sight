/*
** Control Flow Graph
** Copyright (C) 2014 - Hoa Nguyen
*/

import java.lang.Object;
import java.io.Writer;
import java.io.BufferedWriter;
import java.io.FileWriter;

String[] nodf;
//String[] lnodes;
String[] lconn;
String[] fontList;
PFont aFont;

// position of nodes
float[] pnode = new float[50];
// x coordinate of nodes
float[] xcnode = new float[50];
// width of nodes
float[] wnode = new float[50];
String[] names = new String[50];
// number of inputs for each node
int[] numin = new int[50];
int[] numout = new int[50];
// number of connections for each node
int[] numconn = new int[50];
// nested loop array
int[] nest = new int[50];
int numnest = 0;
float nestdis = 10.0;

float xnode;
float nodeheight;
float nodewidth;
// to process the connections bewteen nodes 
int node_st, node_en;
int inp_st, out_en;
// to process for recursion
float xrec_bf, yrec_bf;
float xrec_st, yrec_st, xrec_en, yrec_en;
float rec_height, rec_width;
float recdis = 2.0;
PrintWriter outfile;
String[] outf = new String[100];
String[] lnodes = new String[100];
String[] vizMeth = new String[100];
String[] ioInfo = new String[100];
int lnodes_length = 5;
int cur_lnodes_length = 5;
int[] outn = new int[100];
int depth = 0;
int curDepth = 0;
int rec = 0;
// meth = 0 for recursion tree, meth = 1 for nested loop
int meth = 0;
// collapse: cola = 0: no collapse, cola = 1: collapse
int[][] cola = new int[100][100];
int i_redraw = -1, j_redraw = -1, k_redraw = -1;
float scaleFactor;
float updateWinsize;
String[] nVi;

void setup() 
{ 
  // size of window
  //size(1800, 800);
  size(1200, 1100);
  
  background(255);
  smooth();
  // font size
  fontList = PFont.list();
  aFont = createFont(fontList[0], 10, true);
  textFont(aFont);
  
   // read and write new file
  nodf = loadStrings("node.txt");
  // connection between input and output of nodes   
  lconn = loadStrings("inout.txt");
  
  String[] nfun0 = split(nodf[nodf.length-1],':');
  // modules data
  if(nfun0.length < 5)
  {
    meth = 3;
    // sequence data
    lnodes_length = nodf.length;
    for(int i=0; i< lnodes_length; i++)
    {
      String[] nfun = split(nodf[i],':');
      lnodes[int(nfun[0])] = nfun[1]+":"+nfun[2]+":"+nfun[3];
    }
    vizMeth = loadStrings("dat.txt");
    String[] vizInf = split(vizMeth[0],','); 
    String[] nVi = split(vizInf[2],':');
    
    /*
    for(int k=0; k<nVi.length(); k++)
    {
      addvizMeth(nVi[k]);
    }
    */
    ioInfo = loadStrings("ioInfo.txt");
  }
  else
  {
    //String[] nfun0 = split(nodf[nodf.length-1],':');
    depth = int(nfun0[2]);
    lnodes_length = depth+1;
    
    for(int i=(nodf.length-1); i>=0; i--)
    {
      String[] nfun = split(nodf[i],':');
      int dep = int(nfun[2]);
      int fu = int(nfun[1])+1;
      //nodf[i] = nfun[1]+":"+nfun[3]+":"+nfun[4]+":"+nfun[2];
      
      if(outf[dep] == null)
         outf[dep] = "";
      
      outn[dep] += 1;
         
      if((outn[dep]%2) == 0 && (outn[dep]>0))
        outf[dep]+=",";
      else
        outf[dep]+=":";  
       
      if(fu<3)
        for(int m=1; m<(depth-dep); m++)
          for (int l=1; l<=m; l++)
            outf[dep+m] += ":0,0";
 
      outf[dep] += fu;
    }
    // recursion 
    if(meth == 0)
      for(int j=0; j<= depth; j++)
        lnodes[j] = "R"+outf[j]; 
    // nested loop
    else
      for(int j=0; j<= depth; j++)
        lnodes[j] = "NR"+outf[j];
  }
  
 for(int i=0; i<lnodes_length; i++)
  {
    String[] inode = split(lnodes[i],':');
    if(nfun0.length < 5)
    {
      // name of nodes
      names[i] = inode[0];
      // number of input
      numin[i] = int(inode[1]);
      // number of output
      numout[i] = int(inode[2]);
    }
  }
  
  for(int i=0; i<lnodes_length; i++)
  {
    rec = 0;
    int nodr = 0;
    String[] inode = split(lnodes[i],':');
    
    for(int m=1; m<numnest; m++)
      if(i == nest[m])
        nodr = 1;
    
    if(nodr == 0)
      numnest = 0;
    
    for(int j=0; j<inode.length; j++)
    {
      // if it is nested loop
      if(inode[j].equals("N") == true)
      {
        numnest = 1;
        nest[numnest-1] = i;
        j += 1;
        if(j>2)
        {
          numnest += 1;
          if (numnest != 0)
            nest[numnest-1] = int(inode[j]);
        }
      }
       
      if(inode[j].equals("R") == true)
      {
        numnest = -1;
        rec = 1;
        cola[i][j] = 0;
        cola[i][inode.length+j] = 0;
      }
      if(inode[j].equals("NR") == true)
      {
        numnest = -1;
        rec = 2;
        cola[i][j] = 0;
        cola[i][inode.length+j] = 0;
      }
    }
  }
  
  scaleFactor = 1;
  
  if(rec == 1 || rec == 2)
  {
    // definde node width and height
    nodeheight = 32;
    nodewidth = 120;
    int newWid = int(nodewidth*pow(2,lnodes_length-3)+nodewidth);
    int newHei = int(3*nodeheight*depth);
    size(newWid, newHei);
  }
   
  noLoop();
}
 
void draw() 
{
  background(255);
  
  cur_lnodes_length = lnodes_length;
  
  int tcur_lnodes_length = lnodes_length;
 if(rec ==1 || rec ==2)
 { 
  for(int i=0; i<lnodes_length; i++)
  {
    int changelen = 1;
    String[] inode = split(lnodes[i],':');
    
    for(int j=1; j<inode.length; j++)
    {  
      String[] recnod = split(inode[j],',');
      
      if(rec == 1 || rec == 2)
      {
        int mt;
        if(i_redraw == 0)
        {
           mt = 2*j;
           j_redraw = j;
        }
        else
           mt = 2*j+k_redraw;
        
        if(i_redraw == i && j_redraw == j && cola[i][mt] == 1)
        {
          // expand
          cola[i][mt] = 0;
          int pre_a = mt;
          pre_a = 2*pre_a-1;     
          
          if(int(recnod[k_redraw])== 4)
          {
            cola[i+1][pre_a-1] = 1;
            cola[i+1][pre_a] = 0;
          }
          else if(int(recnod[k_redraw]) > 4)
          {
            cola[i+1][pre_a-1] = 1;
            cola[i+1][pre_a] = 1;
          }
          else 
          {
            cola[i+1][pre_a-1] = 0;
            cola[i+1][pre_a] = 0;
          }
            
          if((i+2)<lnodes_length)  
          {
            for(int a=i+2; a<lnodes_length; a++)
            {  
              pre_a = 2*pre_a-1;      
              for(int b=0;b<pow(2,a-i); b++)
                //if((pre_a-b) < (lnodes_length)*inode.length)
                  cola[a][pre_a-b] = 2;
            }
          }
        }
        else if(i_redraw == i && j_redraw == j && cola[i][mt] == 0)
        {
          // collapse
          cola[i][mt] = 1;  
          int pre_a = mt;
          for(int a=i+1; a<lnodes_length; a++)
          {  
            pre_a = 2*pre_a-1;      
            for(int b=0;b<pow(2,a-i); b++)
              //if((pre_a-b) < (lnodes_length+1)*(inode.length+1))
                cola[a][pre_a-b] = 2;
          }
        }
        else if(cola[i][mt] == 1)
          cola[i][mt] = 1;
        else if(cola[i][mt] == 2)
          cola[i][mt] = 2;
        else
          cola[i][mt] = 0;
        // het thu tam  
          
        if(int(recnod[0])>2)
        {
          if(cola[i][2*j] == 0)
            changelen = 0;
          //println("cola["+i+"]["+2*j+"]="+cola[i][2*j]);
        }
        if(i>0)
        {
          if(int(recnod[1])>2)
          {
            if(cola[i][2*j+1] == 0)
              changelen = 0;
            //println("cola["+i+"]["+(2*j+1)+"]="+cola[i][2*j+1]);
          }
        }
      }
    }
    if(changelen == 1)
    {
      tcur_lnodes_length = i+1;
      i = lnodes_length;
    }
    //println("i="+i+" and changelen="+changelen);
  }
  //println("tcur_lnodes_length="+tcur_lnodes_length);
 } 
  // ket thuc thu
  if(rec == 1 || rec == 2)
  {
    int newWid = int(nodewidth*pow(2,tcur_lnodes_length-3)+nodewidth);
    int newHei = int(3*nodeheight*depth);
    if(scaleFactor == 1)
      size(newWid, newHei);
    //println("width="+width);
  }
  
  // modules data
  String[] nfun0 = split(nodf[nodf.length-1],':');
  if(nfun0.length < 5)
  {
    xnode = 60;
    nodeheight = 24;
    nodewidth = width/1.5;
  }
  else
  {
    // recursion/nested data 
    xnode = width;
    //nodeheight = 32;
    //nodewidth = 120;
  }
  
  for(int i=0; i<tcur_lnodes_length; i++)
  {
    // position, width of nodes
    pnode[i] = height*i/(tcur_lnodes_length+1)+nodeheight;  
    wnode[i] = nodewidth;
    xcnode[i] = xnode;
  }
  
  rec_height = nodeheight;
  rec_width = nodewidth/6;
  xrec_bf = xnode/2;
  yrec_bf = rec_height;
  
  // if data has recursion
  if(rec == 1 || rec == 2)
  {
    draw_methButton(rec);
  }
  
  // Draw input and output of each node
  for(int i=0; i<lnodes_length; i++)
  {
    // draw nodes
    if (rec == 0)
        draw_nodes(i, numnest);
    // recursion
    else if(rec == 1)
    {
      float nn = pow(2,i);
      String[] ino = split(lnodes[i],':');
      int po = 1;
        
      for(int j=1; j<ino.length; j++)
      {
        String[] recnod = split(ino[j],',');
        
        if(i==0)
        {
            // draw first node
            draw_recnode(int(recnod[0]), xrec_bf, pnode[i], rec_width, rec_height, i, cola[i][2*j]);
            
            if(lconn.length > 0)
              draw_curvearrow(xrec_bf+rec_width, pnode[i]+rec_height/2, xnode+nodewidth/2, pnode[int(recnod[0])-1]+nodeheight/2, 2);
        }
        else
        {
            // draw other nodes
            if(int(recnod[0])>0)
            {
              xrec_st = po*2*xrec_bf/(nn+1);
              yrec_st = pnode[i];
              
              draw_recnode(int(recnod[0]), xrec_st, yrec_st, rec_width, rec_height, i, cola[i][2*j]);
              
              float pp = 1;
              if(po>1)
                pp = po-(j-1);
              
              if(int(recnod[0])>1 && cola[i][2*j]!=2)
                draw_arrow(pp*2*xrec_bf/(pow(2,i-1)+1)+rec_width/2,pnode[i-1]+rec_height, xrec_st+rec_width/2, pnode[i]); 
        
              xrec_en = ((po+1)*2*xrec_bf)/(nn+1);
              yrec_en = pnode[i];
                
              draw_recnode(int(recnod[1]), xrec_en, yrec_en, rec_width, rec_height, i, cola[i][2*j+1]);
              //cola = 0;
              if(int(recnod[1])>0 && cola[i][2*j+1]!=2)
                draw_arrow(pp*2*xrec_bf/(pow(2,i-1)+1)+rec_width/2,pnode[i-1]+rec_height, xrec_en+rec_width/2, pnode[i]); 
            }
            po += 2;
        }
      }
    }
    // nested loop
    else if(rec == 2)
    {
      float nn = pow(2,i);
      String[] ino = split(lnodes[i],':');
      int po = 1;
        
      for(int j=1; j<ino.length; j++)
      {
        String[] recnod = split(ino[j],',');
        if(i==0)
        {
            // draw frist node
            draw_nestrec(int(recnod[0]), recdis, pnode[i], 2*xrec_bf-2*recdis, rec_height, i, cola[i][2*j]);
            if(lconn.length > 0)
              draw_curvearrow(xrec_bf+rec_width, pnode[i]+rec_height/2, xnode+nodewidth/2, pnode[int(recnod[0])-1]+nodeheight/2, 2);
        }
        else
        {
            if(int(recnod[0])!=0)
            {
              // position (x,y) of start node - left side              
              xrec_st = (po-1)*2*xrec_bf/nn + (i+1)*recdis;
              yrec_st = pnode[i];
              // position (x,y) of end node - right side
              xrec_en = (po*2*xrec_bf)/nn + (i-1)*recdis;
              yrec_en = pnode[i];
              // previous po   
                
              // left side
              draw_nestrec(int(recnod[0]), xrec_st, yrec_st, (2*xrec_bf)/nn - (i+2)*recdis, rec_height, i, cola[i][2*j]);
              
              // right side
              draw_nestrec(int(recnod[1]), xrec_en, yrec_en, (2*xrec_bf)/nn - (i+2)*recdis, rec_height, i, cola[i][2*j+1]);
            }
            po += 2;
        }
      }
    }
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
        node_st = int(con_st[0]);
        inp_st = int(con_st[2]);
          
        // Node end
        String[] con_en = split(tconn[1],'_');
        node_en = int(con_en[0]);
        out_en = int(con_en[2]);
        //println("node_st="+node_st+"inp_st="+inp_st+"node_en="+node_en+"out_en="+out_en + "numnest ="+numnest );
        
        // Draw connection bewteen two nodes
        // straight arrow connection
        if(node_en == (node_st + 1) && inp_st == out_en)
          draw_arrow(xcnode[node_st]+wnode[node_st]/2, pnode[node_st]+((numin[node_st]+1)*nodeheight), xcnode[node_en]+wnode[node_en]/2, pnode[node_en]); 
        // left curve
        else if(node_en == node_st)
          draw_curvearrow(xcnode[node_st], pnode[node_st]+(numin[node_st]*nodeheight + nodeheight/2), xcnode[node_en], pnode[node_en]+(out_en*nodeheight+nodeheight/2), 1);
        // right curve
        else
          draw_curvearrow(xcnode[node_st]+wnode[node_st], pnode[node_st]+(numin[node_st]*nodeheight + nodeheight/2), xcnode[node_en]+wnode[node_en], pnode[node_en]+(out_en*nodeheight+nodeheight/2), 0);
       }  
     }
   }
}

void updateNum()
{
  
  // thu
  int tcur_lnodes_length = lnodes_length;
  for(int i=0; i<lnodes_length; i++)
  {
    int changelen = 1;
    String[] inode = split(lnodes[i],':');
    
    for(int j=1; j<inode.length; j++)
    {  
      String[] recnod = split(inode[j],',');
      if(rec == 1)
      {
        if(int(recnod[0])>0)
        {
          if(cola[i][2*j] == 0)
            changelen = 0;
          println("cola["+i+"]["+2*j+"]="+cola[i][2*j]);
        }
        if(i>0)
        {
          if(int(recnod[1])>0)
          {
            if(cola[i][2*j+1] == 0)
              changelen = 0;
            println("cola["+i+"]["+(2*j+1)+"]="+cola[i][2*j+1]);
          }
        }
      }
      if(rec == 2)
      {
        //cola[i][j] = 0;
        //cola[i][inode.length+j] = 0;
      }
    }
    if(changelen == 1)
    {
      tcur_lnodes_length = i;
      i = lnodes_length;
    }
    //println("i="+i+" and changelen="+changelen);
  }
  
  println("tcur_lnodes_length="+tcur_lnodes_length);
  
  // ket thuc thu
}
void mousePressed() 
{ 
  if(meth == 3)
  {
    //println("meth"+meth);
  
    for(int i=0; i<lnodes_length; i++)
    {
      String[] vizInf = split(vizMeth[i],','); 
      String[] nVi = split(vizInf[2],':');
    
      for(int k=0; k<nVi.length(); k++)
      {
        float stY, enY;
        if(numout[i] > 0)
        {
          stY = pnode[i]+(numin[i]*nodeheight)+nodeheight;
          enY = pnode[i]+(numin[i]*nodeheight)+2*nodeheight;
        }
        else
        {
          stY = pnode[i]+(numin[i]*nodeheight);
          enY = pnode[i]+(numin[i]*nodeheight)+nodeheight;
        }
        if(mouseX >= (xnode + k*nodewidth/nVi.length()) && mouseX <= (xnode + k*nodewidth/nVi.length()+nodewidth/nVi.length()) && mouseY >= stY && mouseY <= enY)
        {
          //println("k="+k+":"+nVi[k]+"vizInf[0]="+vizInf[0]+"vizInf[1]"+vizInf[1]);
          //println(int(vizInf[1])+k*100);
          clickModule(vizInf[0], int(vizInf[1])+k*100, names[i], (100*mouseX/width+10), 100*mouseY/height);
        }  
      }
    }
  }
  // recursion - tree
  if(rec == 1)
  {
    for(int i=0; i<lnodes_length; i++)
    {
      String[] ino = split(lnodes[i],':');
      int po = 1;
      float nn = pow(2,i);
      
      for(int j=1; j<ino.length; j++)
      {
        String[] recnod = split(ino[j],',');
            
        if(i==0)
        {
            // draw first node
            if(mouseX >= xrec_bf && mouseX <= (xrec_bf+rec_width) && mouseY >= pnode[i] && mouseY <= (pnode[i]+rec_height))
            {
              //cola = 1;
              i_redraw = i; 
              j_redraw = j;
              k_redraw = 0;
              redraw();
            }
        }
        else
        {
            // draw other nodes
            if(int(recnod[0])!=0)
            {
              xrec_st = po*2*xrec_bf/(nn+1);
              yrec_st = pnode[i];
              //draw_recnode(int(recnod[0]), xrec_st, yrec_st, rec_width, rec_height, i);
              float pp = 1;
              if(po>1)
                pp = po-(j-1);
             
              xrec_en = ((po+1)*2*xrec_bf)/(nn+1);
              yrec_en = pnode[i];
              
              if(mouseX >= xrec_st && mouseX <= (xrec_st+rec_width) && mouseY >= yrec_st && mouseY <= (yrec_st+rec_height))
              {
                //cola = 1;
                i_redraw = i; 
                j_redraw = j;
                k_redraw = 0;
                redraw();
              }
              if(mouseX >= xrec_en && mouseX <= (xrec_en+rec_width) && mouseY >= yrec_en && mouseY <= (yrec_en+rec_height))
              {
                //cola = 1;
                i_redraw = i; 
                j_redraw = j;
                k_redraw = 1;
                redraw();
              }
            }
            po += 2;
        }
      }
    }  
  }
  // nested loop
  else if (rec == 2)
  {
    for(int i=0; i<lnodes_length; i++)
    {
      String[] ino = split(lnodes[i],':');
      int po = 1;
      float nn = pow(2,i);
      
      for(int j=1; j<ino.length; j++)
      {
        String[] recnod = split(ino[j],',');
            
        if(i==0)
        {
            // draw first node
            if(mouseX >= xrec_bf && mouseX <= (xrec_bf+2*xrec_bf-2*recdis) && mouseY >= pnode[i] && mouseY <= (pnode[i]+rec_height))
            {
              //cola = 1;
              i_redraw = i; 
              j_redraw = j;
              k_redraw = 0;
              redraw();
            }
        }
        else
        {
            // draw other nodes
            if(int(recnod[0])!=0)
            {
              // position (x,y) of start node - left side              
              xrec_st = (po-1)*2*xrec_bf/nn + (i+1)*recdis;
              yrec_st = pnode[i];
              // position (x,y) of end node - right side
              xrec_en = (po*2*xrec_bf)/nn + (i-1)*recdis;
              yrec_en = pnode[i];

              if(mouseX >= xrec_st && mouseX <= (xrec_st+ (2*xrec_bf)/nn - (i+2)*recdis) && mouseY >= yrec_st && mouseY <= (yrec_st+rec_height))
              {
                //cola = 1;
                i_redraw = i; 
                j_redraw = j;
                k_redraw = 0;
                redraw();
              }
              if(mouseX >= xrec_en && mouseX <= (xrec_en+ (2*xrec_bf)/nn - (i+2)*recdis) && mouseY >= yrec_en && mouseY <= (yrec_en+rec_height))
              {
                //cola = 1;
                i_redraw = i; 
                j_redraw = j;
                k_redraw = 1;
                redraw();
              }
            }
            po += 2;
        }
      }
    }  
  }
  
  // check what method is selected for recursion visualization
  if(mouseX >= 10 && mouseX <= 90 && mouseY >= 5 && mouseY <= 25)
  {
    if(rec == 1)
    {
      rec = 2;
    }
    else
    {
      rec = 1;
    }
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
    scaleFactor += 10;
    if(width+int(scaleFactor)>100 && height+int(scaleFactor/10) > 100)
    {
      size(width+int(scaleFactor), height+int(scaleFactor/10));
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
    scaleFactor -= 10;
    if(width+int(scaleFactor)>100 && height+int(scaleFactor/10) > 100)
    {
      size(width+int(scaleFactor), height+int(scaleFactor/10));
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
    size(1800, 800);
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
    textSize(15);
    fill(50);
    textAlign(CENTER);
    if(metho == 1)
      text("Nested Viz", 50, 22);
    else if(metho == 2)
      text("Tree Viz", 50, 22);
}

void draw_recnode(int inod, float xn, float yn, float nw, float nh, int de, int col)
{
    if(col != 2)
    {
      stroke(0);
      strokeWeight(1);
        
      colorMode(HSB);
      fill(de*255/(depth+1), 150, 255);  
      rect(xn, yn, nw, nh);
      textSize(10);
      fill(50);
      textAlign(CENTER);
      text("M"+(inod-1), xn + nw/2, yn + nh/3);
    } 
    if(col == 1)
    {
      textSize(20);
      text("+", xn + nw/2, yn + 4*nh/5);  
    }
}

void draw_nestrec(int inod, float xn, float yn, float nw, float nh, int de, int col)
{
    if(col != 2)
    {
      // draw node
      stroke(0);
      strokeWeight(1);
      //fill(255);
      colorMode(HSB);
      fill(de*255/(depth+1), 150, 255);
      rect(xn, yn, nw, nh);
     
      
      textSize(10);
      fill(50);
      textAlign(CENTER);
      text("M"+(inod-1), xn + nw/2, yn + nh/1.5);    
    }
    if(col == 0)
    {
       // draw nested box
      if(inod > 2)
      {
        noFill();
        rect(xn, yn+nh, nw, pnode[depth] - yn + nh + inod*recdis);
      }
    }
    if(col == 1)
    {
      textSize(20);
      text("+", xn + 3*nw/4, yn + nh/1.5); 
    }
}

void draw_nodes(int inod, int nonest)
{ 
  //println("inod = "+inod+" numnest="+numnest);
  // no nested loop
  if(nonest == 0)
  {
    // draw inputs
    if(numin[inod] > 0)
    {
      for(int i=0; i<numin[inod]; i++)
      {
        stroke(0);
        strokeWeight(1);
        //fill(255);
        colorMode(HSB);
        fill(inod*255/lnodes_length, 150, 255);
        rect(xnode, pnode[inod]+(i*nodeheight), nodewidth, nodeheight);
        
        textAlign(CENTER);
        textSize(10);
        fill(50);
        
        text(names[inod]+"_In"+i, xnode + nodewidth/2, pnode[inod]+(i*nodeheight)+nodeheight/1.5);
      }
    }
    
    // draw outputs
    // draw visualization methods box
    String[] vizInf = split(vizMeth[0],','); 
    String[] nVi = split(vizInf[2],':');
    
    if(numout[inod] > 0)
    {
      // draw outputs
      stroke(0);
      strokeWeight(1);
      //fill(255);
      colorMode(HSB);
      fill(inod*255/lnodes_length, 250, 255);
        
      rect(xnode, pnode[inod]+(numin[inod]*nodeheight), nodewidth, nodeheight);
      
      textSize(10);
      fill(50);
      textAlign(CENTER);
      text(names[inod]+"_Out0", xnode + nodewidth/2, pnode[inod]+(numin[inod]*nodeheight)+nodeheight/1.5);
      
      // draw visualization methods box
      for(int k=0; k<nVi.length(); k++)
      {
        stroke(0);
        strokeWeight(1);
        //fill(255);
        colorMode(HSB);
        fill(k*255/nVi.length(), 20, 250);
        
        rect(xnode + k*nodewidth/nVi.length(), pnode[inod]+(numin[inod]*nodeheight)+nodeheight, nodewidth/nVi.length(), nodeheight);
       
        textSize(10);
        fill(50);
        textAlign(CENTER);
        text(nVi[k], xnode + (k+1/2)*nodewidth/nVi.length(), pnode[inod]+(numin[inod]*nodeheight)+nodeheight+nodeheight/1.5);
      }
    }
    else
    {
      // draw visualization methods box
      for(int k=0; k<nVi.length(); k++)
      {
        stroke(0);
        strokeWeight(1);
        //fill(255);
        colorMode(HSB);
        fill(k*255/nVi.length(), 20, 250);
        rect(xnode + k*nodewidth/nVi.length(), pnode[inod]+(numin[inod]*nodeheight), nodewidth/nVi.length(), nodeheight);
       
        textSize(10);
        fill(50);
        textAlign(CENTER);
        text(nVi[k], xnode + (k+1/2)*nodewidth/nVi.length(), pnode[inod]+(numin[inod]*nodeheight)+nodeheight/1.5);
      }
    }
    
    // draw input, output information
      int nodi = inod;
      for(int m=0; m<ioInfo.length();m++)
      {
        String[] ioi = split(ioInfo[m],';'); 
        if(int(ioi[0]) == inod)
          nodi = m;
      }
      String[] ioMod = split(ioInfo[inod],';'); 
      int numInf = int(ioMod[1]);
      if(numInf > 0)
      {
        String[] ioInf = split(ioMod[2],',');
        String inf = "";
        for(int k=0; k<numInf; k++)
        {
          inf += ioInf[k]+"\n";
        }
        if(numout[nodi] > 0)
        {
          stroke(0);
          strokeWeight(1);
          fill(255);
          rect(xnode, pnode[nodi]+(numin[nodi]*nodeheight)+2*nodeheight, nodewidth, numInf*nodeheight/1.5);
         
          textSize(10);
          fill(50);
          textAlign(CENTER);
          text(inf, xnode + nodewidth/2, pnode[nodi]+(numin[nodi]*nodeheight)+2*nodeheight+nodeheight/1.5);
          }
        else
        { 
          stroke(0);
          strokeWeight(1);
          fill(255);
          rect(xnode, pnode[nodi]+(numin[nodi]*nodeheight)+nodeheight, nodewidth, numInf*nodeheight/1.5);
         
          textSize(10);
          fill(50);
          textAlign(CENTER);
          text(inf, xnode + nodewidth/2, pnode[nodi]+(numin[nodi]*nodeheight)+nodeheight+nodeheight/1.5);          
        }
      }
  }
  // nested loop
  else if (nonest > 0)
  {
    int nnest = 0;
    float nheight = 0.0;
    float nwidth = 0.0;
    
    for(int m=0; m<numnest; m++)
      if(inod == nest[m])
        nnest = m;
    
    for(int k=0; k<nnest; k++)
    {
        int ino = nest[k];
        nheight += (numin[ino]+1)*nodeheight + nestdis;
    }
    
    float xnestnode = 3*xnode/4 + nnest*2*nestdis;
    float pnestnode = pnode[nest[0]] + nheight;
    float nodenestwidth = 3*nodewidth/2 - 2*nnest*nestdis;
    
    // update position node array
    pnode[inod] = pnestnode;
    // update width node array
    wnode[inod] = nodenestwidth;
    // update x coordinate node array;
    xcnode[inod] = xnestnode;
    
    // draw inputs
    for(int i=0; i<numin[inod]; i++)
    {
      stroke(0);
      strokeWeight(1);
      fill(255);
      rect(xnestnode, pnestnode+(i*nodeheight), nodenestwidth, nodeheight);
      
      textAlign(CENTER);
      textSize(10);
      fill(50);
      text("M"+(inod+1)+"_In"+i, xnestnode + nodenestwidth/2, pnestnode+(i*nodeheight)+nodeheight/1.5);
    }
    // draw outputs
    stroke(0);
    strokeWeight(1);
    fill(255);
    rect(xnestnode, pnestnode+(numin[inod]*nodeheight), nodenestwidth, nodeheight);
    
    textSize(10);
    fill(50);
    textAlign(CENTER);
    text("M"+(inod+1)+"_Out0", xnestnode + nodenestwidth/2, pnestnode+(numin[inod]*nodeheight)+nodeheight/1.5);
    
    // draw nest box
    if(nnest == (numnest - 1))
    {
      stroke(25);
      strokeWeight(1);
      noFill();
      float nhei = 0.0;
      for(int k=nnest; k<numnest; k++)
      {
          int ino = nest[k];
          nhei += (numin[ino]+1)*nodeheight + nestdis;
          //println("k = "+ k+" nhei = " + nhei);
      }
      
      rect(xnestnode - 2*nestdis, pnestnode - nestdis, nodenestwidth + 2*nestdis, nhei);
      
      for(int k=1; k<(numnest-1); k++)
          rect(xnestnode - 2*nnest*nestdis, pnestnode - (numin[nest[k]]+1)*nodeheight - 2*nestdis, nodenestwidth + nnest*2*nestdis,((numin[nest[k]]+1)*nodeheight)+nhei+nestdis);
    }
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
    noFill();
    stroke(25);
    strokeWeight(1);
    
    float d = dist(x1, y1, x2, y2);
    float xc1, xc2, yc1, yc2;
    //println("leri = "+leri);
    // if it is in left side
    if(leri == 1)
    {
        xc1 = x1 - d/4;
        xc2 = xc1;
        yc1 = y1 - d/3;
        yc2 = y1 - 2*d/3;
    }  
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
      xc1 = (x1+x2)/2;
      yc1 = y1;
      xc2 = x2;
      yc2 = (y1+y2)/2;
    }
       
    // draw curve
    bezier(x1, y1, xc1, yc1, xc2, yc2, x2, y2);
    
    // draw arrow
    pushMatrix();
    translate(x2, y2);
    float a;
    a = atan2(xc2-x2, y2-yc2);
    
    /*
    if(xc1 != x2 && y2 !=yc2) 
        a = atan2(xc2-x2, y2-yc2);
    else
        a = atan2(100, 100);
    */
    
    rotate(a);
    line(0, 0, -5, -5);
    line(0, 0, 5, -5);
    popMatrix();
  
} 


