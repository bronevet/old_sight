#include "cd_profile.h"
using namespace cd;

/// main function start!!------------------------------------------------------------------------------------
int main(int argc, char** argv)
{

  //CDHandle* cd_root;// = new CDHandle();
  CDErrType cd_err_t;
	
  CDHandle* cd_root = Init(cd_err_t, false);
  cout << "cd_root: "<<cd_root;
  getchar();
	/////---cout<<"domain start"<<endl;
	
  dbg << "Root"<<endl;
  double varA[100];
  double varB[200];
  double varC[300];
  double varD[400];
  double varE[500];
  double varF[600];

  CDHandle* cd1;		// at 1st level, children are 4
  CDHandle* cd2;	// at 2nd level, children are 16
  
	cd_root->Begin(context("ID", "Root"), "Root");

  cd_root->Preserve(varC, sizeof(varC), kCopy, "varC");
  cd_root->Preserve(varD, sizeof(varD), kCopy, "varD");
  cd_root->Preserve(varF, sizeof(varF), kCopy, "varF");


  dbg << "Root begun"<<endl;
  
  cd1 = cd_root->Create(kStrict, &cd_err_t);
  dbg << "CD 1 created"<<endl;

  for(int i=0; i<5; i++) {
    cd1->Begin(context("i", i, "ID", "A"), "Loop");
    dbg << "CD "<<i<<" 1A begun"<<endl;
    cout << "CD "<<i<<" 1A begun"<<endl;

    cd1->Preserve(varA, sizeof(varA), kCopy, "varA");
    cd1->Preserve(varB, sizeof(varB), kCopy, "varB");
    cd1->Preserve(varC, sizeof(varC), kRef, "varC");
    cd1->Preserve(varD, sizeof(varD), kRef, "varD");
    cd1->Preserve(varF, sizeof(varF), kRef, "varF");

    cd1->Detect();
    cd1->Complete();

    cd1->Begin(context("i", i, "ID", "B"), "Loop");
    dbg << "CD 1B begun"<<endl;

    cd1->Preserve(varA, sizeof(varA), kCopy, "varA");
    cd1->Preserve(varB, sizeof(varB), kCopy, "varB");
    cd1->Preserve(varC, sizeof(varC), kRef, "varC");
    cd1->Preserve(varD, sizeof(varD), kRef, "varD");
    cd1->Preserve(varF, sizeof(varF), kRef, "varF");


      cd2 = cd1->Create(kStrict, &cd_err_t);
      dbg << "CD 2 created "<<endl;

      cd2->Begin(context("i", i, "ID", "X"), txt()<<"Kernel "<<2);
      dbg << "CD 2a begun"<<endl;
      cd2->Preserve(varA, sizeof(varA), kRef, "varA");
      cd2->Preserve(varB, sizeof(varB), kRef, "varB");
      cd2->Preserve(varC, sizeof(varC), kCopy, "varC");
      cd2->Preserve(varE, sizeof(varE), kCopy, "varE");	// it is not inclusive

      cd2->Detect();
      cd2->Complete();

      cd2->Begin(context("i", i, "ID", "X"), txt()<<"Kernel "<<2);
      dbg << "CD 2b begun"<<endl;
      cd2->Preserve(varA, sizeof(varA), kRef, "varA");
      cd2->Preserve(varB, sizeof(varB), kRef, "varB");
      cd2->Preserve(varC, sizeof(varC), kCopy, "varC");
      cd2->Preserve(varE, sizeof(varE), kCopy, "varE");	// it is not inclusive

      

      cd2->Detect();

      cd2->Complete();


      cd2->Destroy();

		cd1->Detect();
    cd1->Complete();
    }
    cd1->Destroy();

  cd_root->Detect();

  cd_root->Complete();
	cd_root->Destroy();


  return 0;

}

