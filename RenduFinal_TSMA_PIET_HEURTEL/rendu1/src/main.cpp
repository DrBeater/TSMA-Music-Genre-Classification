#include <string>
#include <vector>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include "mfcc.h"
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

 //MFCC FILES

static int file_select(const struct dirent   *entry) 
{

  if (strstr(entry->d_name, ".mfcc") != NULL) 
    {
      if (strlen(strstr(entry->d_name, ".mfcc")) == 5)
	return (1);
      else
	return (0);
    }
  else
    return (0);
}


float computeScore (double *mfcc1, double *mfcc2)
{
  float D=0.0;

  int i;
  for(i=0 ; i<MFCC_SIZE ; i++){
    D += fabs(mfcc1[i]-mfcc2[i]);
  }

  return D;
}



float compare (double *mfcc1, string input2)
{
  /* read MFCC */
  double mfcc2[MFCC_SIZE];

    /* Process */
  ifstream f_mfcc (input2.data(), ios::in);
  if (f_mfcc.is_open())
    {
      double val;
      int i=0;
      
      f_mfcc >> val;      
      while (!f_mfcc.eof())
	{
	  mfcc2[i] = val;
	  i++;
	  f_mfcc >> val;
	}
      
      if (i != MFCC_SIZE)
	cerr << "Error in mfcc file " << input2 << endl;
    }
  else 
    {
      cerr << "Unable to open mfcc file " << endl;
    }
  
  float D = 0.0;

  D =  computeScore (mfcc1, mfcc2);
    
  return D;
}

/* ------------------------------------------ */
/* MFCC */

int mfcc_compare(double* mfcc1, string input2)
{
  string total("");

  struct dirent **eps;
  int n=0;
  float score = 0.0;

  n = scandir (input2.data(), &eps, file_select, alphasort);
  
  if (n >= 0)
    {
      /* Directory Processing */
      
      /* Verify directory name*/
      if (input2[input2.size()-1]!= '/')
	  input2+="/";
      
      int cnt=0;

      cout << endl;
      
      
      for (cnt = 0; cnt < n; cnt++)
       {
	 score = compare(mfcc1, input2+string(eps[cnt]->d_name));
	 
	 cout << "    -" << string(string(eps[cnt]->d_name)) << " : " << score << endl;
	 
	 free(eps[cnt]);
       }
     
      free(eps);
    }
  else if (access(input2.c_str(), R_OK | F_OK) == 0)
    {
      /* File Processing */
      score = compare(mfcc1, input2);
      cout <<  endl << "    -" << input2 << " : " << score << endl;
    }
  else 
    {
      cerr << endl << "Input 2 : File or Directory name error" << endl;
    }

  return EXIT_SUCCESS;
}
  

int main(int argc, char* argv[])
{
  if (argc<=2)
    {
      cerr << "Usage : ./compare <input mfcc dir1> <input mfcc dir2>"<<endl;
      return 1;
    }

  struct dirent **eps;
  int n=0;
  
  string input1=string(argv[1]);
  
  n = scandir (input1.data(), &eps, file_select, alphasort);

  if (n >= 0)
    {
      /* Directory Processing */
      
      /* Verify directory name*/
      if (input1[input1.size()-1]!= '/')
	  input1+="/";
      
      cerr << "Query file processing " << endl;
      
      int cnt=0;
      
      for (cnt = 0; cnt < n; cnt++)
       {
	 /* read MFCC */
	 double mfcc[MFCC_SIZE];
	 
	 /* Process */
	 string s = input1+string(eps[cnt]->d_name);
	 ifstream f_mfcc (s.data(), ios::in);
	 if (f_mfcc.is_open())
	   {
	     double val;
	     int i=0;
	     
	     f_mfcc >> val;      
	     while (!f_mfcc.eof())
	       {
		 mfcc[i] = val;
		 i++;
		 f_mfcc >> val;
	       }
      
	     if (i != MFCC_SIZE)
	       cerr << "Error in mfcc file " << string(argv[1]) << endl;
	   }
	 else 
	   {
	     cerr << "Unable to open mfcc file " << endl;
	   }

	 cout << s << ": ";
	 mfcc_compare(mfcc, string(argv[2]));


	 free(eps[cnt]);
       }
     
      free(eps);
    }
  else if (access(input1.c_str(), R_OK | F_OK) == 0)
    {
      /* File Processing */
	 /* read MFCC */
	 double mfcc[MFCC_SIZE];
	 
	 /* Process */
	 string s = input1;
	 ifstream f_mfcc (s.data(), ios::in);
	 if (f_mfcc.is_open())
	   {
	     double val;
	     int i=0;
	     
	     f_mfcc >> val;      
	     while (!f_mfcc.eof())
	       {
		 mfcc[i] = val;
		 i++;
		 f_mfcc >> val;
	       }
      
	     if (i != MFCC_SIZE)
	       cerr << "Error in mfcc file " << string(argv[1]) << endl;
	   }
	 else 
	   {
	     cerr << "Unable to open mfcc file " << endl;
	   }

	 cout << s << ": ";
	 mfcc_compare(mfcc, string(argv[2]));

    }
  else 
    {
      cerr << endl << "Input 2 : File or Directory name error" << endl;
    }

  return EXIT_SUCCESS;
}


