/*
Copyright 2014, The University of Texas at Austin 
All rights reserved.
 
THIS FILE IS PART OF THE CONTAINMENT DOMAINS RUNTIME LIBRARY

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met: 

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer. 

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution. 

3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "../lib/cd_profile.h"
using namespace cd;

/// main function start!!------------------------------------------------------------------------------------
int main(int argc, char** argv)
{

  //CDHandle* cd_root;// = new CDHandle();
  CDErrType cd_err_t;
	
  CDHandle* cd_root = Init(cd_err_t, false);

	/////---cout<<"domain start"<<endl;
	

  double varA[100];
  double varB[200];
  double varC[300];
  double varD[400];
  double varE[500];
  double varF[600];

  CDHandle* cd1[4];		// at 1st level, children are 4
  CDHandle* cd2[16];	// at 2nd level, children are 16
  
	cd_root->Begin();

  cd_root->Preserve(varC, sizeof(varC), kCopy, "varC");
  cd_root->Preserve(varD, sizeof(varD), kCopy, "varD");
  cd_root->Preserve(varF, sizeof(varF), kCopy, "varF");


  for(int i=0; i<4; i++){
    cd1[i] = cd_root->Create(kStrict, &cd_err_t);
  }

  for(int i=0; i<4; i++){
    cd1[i]->Begin();
  }

  for(int i=0; i<4; i++){
    cd1[i]->Preserve(varA, sizeof(varA), kCopy, "varA");
    cd1[i]->Preserve(varB, sizeof(varB), kCopy, "varB");
    cd1[i]->Preserve(varC, sizeof(varC), kRef, "varC");
    cd1[i]->Preserve(varD, sizeof(varD), kRef, "varD");
    cd1[i]->Preserve(varF, sizeof(varF), kRef, "varF");
  }

  for(int i=0; i<4; i++){
    cd1[i]->Detect();
	}
	//while(1);
	for(int i=0; i<4; i++){
    cd1[i]->Complete();
  }

  for(int i=0; i<4; i++){
    cd1[i]->Begin();
  }

  for(int i=0; i<4; i++){
    cd1[i]->Preserve(varA, sizeof(varA), kCopy, "varA");
    cd1[i]->Preserve(varB, sizeof(varB), kCopy, "varB");
    cd1[i]->Preserve(varC, sizeof(varC), kRef, "varC");
    cd1[i]->Preserve(varD, sizeof(varD), kRef, "varD");
    cd1[i]->Preserve(varF, sizeof(varF), kRef, "varF");
  }

  for(int i=0; i<4; i++){
    for(int j=0; j<4; j++){
      /////---cout<<"cd1["<<i<<"]'s addr: "<<cd1[i]<<endl;

      cd2[j+4*i] = cd1[i]->Create(kStrict, &cd_err_t);

      
      /////---cout << "cd2[" << j+4*i<<"] is created: " << cd2[j+4*i] << endl;

      /////---cout << "cd2["<<j+4*i<<"]'s parent handle: "<< cd2[j+4*i]->parent_cd_ <<endl;

    }
  }

  for(int i=0; i<4; i++){
    for(int j=0; j<4; j++){

      /////---cout<<"cd2["<<j+4*i<<"]: "<<"from cd"<<i<<endl;
      cd2[j+4*i]->Begin();
      cd2[j+4*i]->Preserve(varA, sizeof(varA), kRef, "varA");
      cd2[j+4*i]->Preserve(varB, sizeof(varB), kRef, "varB");
      cd2[j+4*i]->Preserve(varC, sizeof(varC), kCopy, "varC");
      cd2[j+4*i]->Preserve(varE, sizeof(varE), kCopy, "varE");	// it is not inclusive

      cd2[j+4*i]->Detect();
      cd2[j+4*i]->Complete();

      cd2[j+4*i]->Begin();
      cd2[j+4*i]->Preserve(varA, sizeof(varA), kRef, "varA");
      cd2[j+4*i]->Preserve(varB, sizeof(varB), kRef, "varB");
      cd2[j+4*i]->Preserve(varC, sizeof(varC), kCopy, "varC");
      cd2[j+4*i]->Preserve(varE, sizeof(varE), kCopy, "varE");	// it is not inclusive

    }
  }

	for(uint64_t j=0; j<16; j++){
    cd2[j]->Detect();
	}

	for(uint64_t j=0; j<16; j++){
    cd2[j]->Complete();
    /////---cout << "complete is done " << j << endl;
  }

  for(uint64_t j=0; j<16; j++){
		/////---cout << "destroy starts for cd2[" << j << "]: "<< cd2[j] <<endl;
    cd2[j]->Destroy();

  }
	/////---cout<<"merong!!!!!!!!!!!!!!-------------"<<endl;
  for(uint64_t i=0; i<4; i++){
		cd1[i]->Detect();
	}
	for(uint64_t i=0; i<4; i++){
    cd1[i]->Complete();
  }
	/////---cout<<"merong222222!!!!!!!!!!!!!!-------------"<<endl;
  for(uint64_t i=0; i<4; i++){
		/////---cout << "destroy starts for cd1 " << i << endl;
    cd1[i]->Destroy();
  }

  cd_root->Detect();

  cd_root->Complete();

	cd_root->Destroy();

  /////---cout<<"the end--------------"<<endl;

  return 0;

}

