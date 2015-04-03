/* C* Bismillohir Rahmonir Rahiym C*/

// Turkiston Dasturlari (c) 2015
// Muqobil Dasturlar To'plami (c) 2006-2012
// released under the terms of GPL version 2

#pragma once

#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <iterator>

#include <vector>
#include <set>
#include <map>
#include <cstring>

#include <glib.h>

//defined by relativist ultrix
#define WORD_SIZE 64
#define TRANS_SIZE 8192;

#define DEFAULT_NUM_ENWORDS  7318//9148  //MAX_NUM_WORDS div 4 & round up
#define MAX_NUM_WORDS        36590

using namespace std;

typedef struct Data
 {
   string word;
   char phone_trans[WORD_SIZE];
   unsigned long trans;
 }_data;

class CLugat
{

public:

  friend bool include_substr(Data data1, Data data2);

  CLugat(int num_words=DEFAULT_NUM_ENWORDS);
  ~CLugat();

  int init();
  char* format(char *delim);

  /*-----------------------------------------------------------*/
  bool process (char * _data );
  int  process(unsigned int row);
  int  process_file( char *word, int fpos_index, short dir = 2 );

  /*-----------------------------------------------------------*/
  const char*    Get1MatchWord( short which);
  vector<_data>& GetDataSeq();
  int            GetFposNum();
  int            GetNextN();
  unsigned int   GetRow();
  char*          GetTrans( bool );
  const char*    GetIPA(short);
  set <string>   get_setof_syns( short which);
  short          get_syn_order( string );

  int geterr(string &errmsg);

  /*-----------------------------------------------------------*/
  int  LoadNextN( int fpos_index, int n  = 0 );
  void LoadSynonyms();

  /*-----------------------------------------------------------*/
  void SetNextN( short n );
  void SetRow  ( unsigned int );

private:

  char *_trans, *_ipa;
  vector< _data > vdata;
  vector< _data >::iterator Iter, res1;
  unsigned int _row, next_n;
  vector< unsigned long > _fpos, _fTransPos, _fIPA_pos;//for locating in file
  vector < set <string> > aset_syn; //array of sets of synonyms
  vector < map<string, short> > amap_syn;
  unsigned long int curr_fpos, _trans_pos, trans_fpos, ipa_pos, _order;
  ifstream ifTrans, ifIPA;
  int  _errno;
  string _errmsg;
};
