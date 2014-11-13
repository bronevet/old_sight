#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <string>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
using namespace std;

list<string> tokenize(char* s, const char* delimiters);
void remTrailingLinebreak(char* s);

static char hostname[10000];
static bool hostnameInitialized = false;

// Returns true is some service is current listerning on the given port and false otherwise
bool isPortUsed(int port) {
  FILE *fp;
  char path[1035];

  if(!hostnameInitialized) {
    int ret = gethostname(hostname, 10000);
    if(ret==-1) { printf("ERROR calling gethostname!"); exit(-1); }
    hostnameInitialized=true;
  }

  ostringstream cmd;
  cmd << "nc -z "<<hostname<<" "<<port;
  fp = popen(cmd.str().c_str(), "r");
  if(fp == NULL) {
    printf("Failed to run command\n" );
    exit(-1);
  }

  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    return true;
  }
  return false;
}

list<string> getAllHostnames()
{
  FILE *fp;
  char path[1035];

  /* Open the command for reading. */
  ostringstream cmd; cmd << "hostname "<<HOSTNAME_ARG;
  fp = popen(cmd.str().c_str(), "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(-1);
  }

  /* Read the output a line at a time - output it. */
  list<string> tokens;
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    //printf("%s", path);
    remTrailingLinebreak(path);
    list<string> subTokens = tokenize(path, " ");
    tokens.insert(tokens.end(), subTokens.begin(), subTokens.end());
  }
  /*for(list<string>::iterator t=tokens.begin(); t!=tokens.end(); t++)
    printf("\"%s\"\n", t->c_str());*/

  /* close */
  pclose(fp);

  return tokens;
}

list<string> tokenize(char* s, const char* delimiters) {
  list<string> tokens;
  while(char* token = strsep(&s, delimiters)) {
    if(strcmp(token, "")!=0)
      tokens.push_back(string(token));
  }
  return tokens;
}

void remTrailingLinebreak(char* s) {
  // Find the end of the line
  int i=0;
  while(s[i]!='\0') i++;
  
  // Remove the trailing \r and \n chars
  i--;
  while(i>0 && (s[i]=='\r' || s[i]=='\n')) {
    s[i] = '\0';
    i--;
  }
}
