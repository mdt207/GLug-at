/* C* Bismillohir Rahmonir Rahiym C* */

/***********************************************************\
 * Turkiston Dasturlari (c) hijriy 1436                    *
 * Turkiston Software   (c) 2015                           *
 * GLug'at dasturi                                         *
 * Version 1.91                                            *
\***********************************************************/

/******************************************************\
 | Muqobil Dasturlar To'plami (c) 2006-2012           |
 |       Set of Alternative Software                  |
 | GLug'at tizimi                                     |
\******************************************************/
// Released under the terms of GPL version 2
// based on source code by TKL K96 207 (c)

#include "clugat.h"
#include <math.h>
#include <gtk/gtk.h>

bool  isIPA = true;
bool is_vec_fedup = false;
vector< _data > _lookFor;

//bool include_substr(Data data1, Data data2)
bool include_substr(Data haystack, Data needle)
{
   bool res = false;

   int gl_len_comp = needle.word.size();


   needle.word.compare(0, gl_len_comp, &(haystack.word)[0], gl_len_comp) == 0 ? res = true : res = false;

  return res;
}


CLugat::CLugat( int num_words):next_n(num_words),//for a just distribution
                     //found(false),
                     _order(0),
                     _errno(0),
                     curr_fpos(0), trans_fpos(0),
                     res1(vdata.begin())
  {

    ifstream ifconf( "config.txt", ios::in );

    if (ifconf.fail())  {
      _errmsg = "config.txt file not found! Defaulting...";
      _errno = 2;

    }
    else {

        int i = 0;

        char buf[WORD_SIZE];

        while( ifconf >> i  )
        {
              if(i == 5) break;
              ifconf.getline( buf, WORD_SIZE);
              //cout << i << " " << buf << endl;
         }

        ifconf >> next_n;
        if( next_n <= 0) next_n = DEFAULT_NUM_ENWORDS;

    }

    ifconf.close();
  }

 CLugat::~CLugat()
  {

     delete []_trans;
     delete []_ipa;
     ifTrans.close();
     ifIPA.close();
  }

  int  CLugat::init()
  {

    int mileStonesNum = (int)ceil( (float)MAX_NUM_WORDS / (float)next_n );
    next_n = (int)ceil( (float)MAX_NUM_WORDS / (float)mileStonesNum );

    vdata.resize(next_n);

   _lookFor.resize(1);

   _fpos.resize(     mileStonesNum);
   _fTransPos.resize(mileStonesNum);
   _fIPA_pos.resize( mileStonesNum);

    ifTrans.open("ingtur-tur", ios::in);
    if( ifTrans.fail() )
    {
     _errmsg = "Translations file \"ingtur-tur\" not found! \nExitting...";
     _errno = -1;
     cout << _errmsg << _errno<< endl;

     return _errno;
    }
    else
    {
      _trans = new char[TRANS_SIZE];
    }

    ifstream input( "ingtur-ing", ios::in );
    if (input.fail())
    {
      _errmsg = "English words list file \"ingtur-ing\" not found! \nExitting...";
      _errno = -1;
      cout << _errmsg <<  _errno << endl;

      return _errno;
    }

    ifIPA.open( "ipa.txt", ios::in );
    if (ifIPA.fail())
    {
      _errmsg = "IPA file \"ipa.txt\" not found! \nExitting...";
      _errno = 2;
      cout <<  _errmsg << endl;
      isIPA = false;
    }
    //else
    {
        _ipa = new char[WORD_SIZE];
    }


    int i = 0;

    _fpos[0] = 0; _fIPA_pos[0] = 0; _fTransPos[0] = 0;

    _data buf;

    while( !input.eof() )
    {

       input.getline(_ipa, WORD_SIZE);
       if( !ifIPA.eof() ) ifIPA.getline(buf.phone_trans, WORD_SIZE);

/*----------------------------------------------------------------------------*/
    if( !is_vec_fedup ) {

        g_strstrip( _ipa );
        buf.word.assign( _ipa );

        g_strlcat( buf.phone_trans, "\n", WORD_SIZE);


        buf.trans = ifTrans.tellg(); //Translation file ichra ko`chish manzillari

        vdata[i] = buf;

     }

/*----------------------------------------------------------------------------*/

       ifTrans.getline(_trans, TRANS_SIZE);

       i++;

        if( ( i % next_n ) == 0 )
        {

           int indx = floor( i / next_n );

           if(indx > 0 && indx < mileStonesNum) {

            _fpos[indx] =  input.tellg();

            _fTransPos[indx] =  ifTrans.tellg();

            _fIPA_pos[indx] =  ifIPA.tellg() ;

            is_vec_fedup = true;
           }
        }

    }

    input.close();
    LoadSynonyms();

    return _errno;
  }

 int CLugat::geterr(string &errmsg)
 {

    errmsg = _errmsg;
    return _errno;
 }

// load next N words into list starting at fpos in file
 int CLugat::LoadNextN( int fpos_index, int n)
 {
    int i = 0;

   {
        _data  buf;
       n == 0 ? n = this->next_n : n = n;


       ifstream input( "ingtur-ing", ios::in );
       if (input.fail())
       {
          _errmsg = "English words list file \"ingtur-ing\" not found! \nExitting...";
          _errno = -1;
          return _errno;
       }

       curr_fpos = _fpos[fpos_index];

       input.seekg(curr_fpos);

       ifTrans.clear(); ifTrans.seekg(ios::beg);
       ifIPA.clear();   ifIPA.seekg( ios::beg );

       trans_fpos = _fTransPos[fpos_index];

       ifTrans.seekg(trans_fpos);
       trans_fpos = ifTrans.tellg();
/*-------------------------------------------------------------------------------*/
       fpos_index <= _fIPA_pos.size() ? ipa_pos = _fIPA_pos[ fpos_index ] : ipa_pos = -1; //ipa_pos = _fIPA_pos[0];

       isIPA = false;

       if ( ipa_pos >= 0) {
            ifIPA.seekg(ipa_pos);
            ipa_pos = ifIPA.tellg();
            isIPA = true;
       }

/*------------------------------------------------------------------------------------*/
       while( i != n )
       {
         if( input.eof() ) { /*_eof = true;*/ break; }

        input.getline( _ipa, WORD_SIZE);

        g_strstrip( _ipa );
        buf.word.assign( _ipa );

        curr_fpos = input.tellg();

        if( isIPA )
        {
            if( !ifIPA.eof() )
            {


                ifIPA.getline( buf.phone_trans, WORD_SIZE);
                g_strlcat( buf.phone_trans, "\n", WORD_SIZE);

            }

        }

        buf.trans = ifTrans.tellg();
        ifTrans.getline(_trans, TRANS_SIZE);

        vdata[i] = buf;

         ++i;

       }

     input.close();
   }

    return i;//factual amount of words loaded in vector
 }

 void CLugat::LoadSynonyms()
 {
    /*Tokenize*/
    gchar** tokens;
    short num_tokens;

    set <string> set_syn;
    map <string, short> map_syn;

    string str;
    ifstream ifSyns;
    ifSyns.open("synonyms.dat", ios::in);

    if( !ifSyns.fail() )
    {
	while( !ifSyns.eof() )
        {
           //syns.clear();
           set_syn.clear();
           map_syn.clear();

           getline(ifSyns, str); //get it from file

           /*tokenize it*/
           tokens = g_strsplit( (const gchar *)&str[0], " ", -1);
           num_tokens = g_strv_length(tokens);


           //convert it to set
           for(int i = 0; i < num_tokens; i++)
           {

             int  last_index = strlen(tokens[i])-1;
             char last_letter = tokens[i][last_index];

             if ( g_ascii_isdigit( last_letter ) )
             {
                 str.resize(last_index);

                 g_strlcpy((gchar*)&str[0], tokens[i], last_index+1);

                 map_syn[str] = g_ascii_digit_value(last_letter)-1;

                 set_syn.insert(str);
             }
             else
             {
                map_syn[(string)tokens[i]] = 0;

                set_syn.insert((string)tokens[i]);
             }
           }

           aset_syn.push_back(set_syn);
           //
           amap_syn.push_back( map_syn );

           g_strfreev( tokens );//free up mem
        }

    }
    else{
      _errmsg = "synonyms.dat file not found. Exitting...";
      _errno  = 2;
      return;
    }

    ifSyns.close();


 }

 set <string> CLugat::get_setof_syns( short which)
 {
   map<string, short>::const_iterator cur;
   set<string> set_syn;


   for(int c = 0; c < amap_syn.size()-1; c++)
   {
     switch(which)
     {
         case 0:{
                    cur = amap_syn[c].find( res1->word );

                    if( cur != amap_syn[c].end() )
                    {
                        set_syn = aset_syn[c];
                        break;
                    }
                } break;
         case 1:{
                 if( amap_syn[c].count( vdata[_row].word ) )
                 {
                     set_syn = aset_syn[c];
                     break;
                 }
                }break;
     }
   }

   return set_syn;
 }

 short  CLugat::get_syn_order( string syn)
 {
     map<string, short>::const_iterator cur;
     int c;

     for(c = 0; c < amap_syn.size()-1; c++)
     {
         cur  = amap_syn[c].find(syn);
         if( cur != amap_syn[c].end() )
         {
            _order = (*cur).second;
            return _order;
         }
     }

     return -1;
 }

 int CLugat::process(unsigned int row)
 {
   _row = row;
   this->_trans_pos = vdata[row].trans;
   return _trans_pos;
 }

 bool CLugat::process(char * _data )
  {
    int len = strlen(_data);
    _lookFor[0].word.assign(_data,len);

    if(len > 0)
    {

        try
        {
            res1 = find_first_of( vdata.begin(), vdata.end(), _lookFor.begin(), _lookFor.end(), include_substr);
        }
        catch(...)
        {
            cout << "seg fault" << endl;
        }
    }

    if(res1 == vdata.end())  return false;
    else {

        res1    += _order; _order = 0;
        _row     = res1 - vdata.begin();

       this->_trans_pos = res1->trans;

       return true;
     }
  }

 //search in file, unless found in vector
 int CLugat::process_file( char *word, int fpos_index, short dir )
 {

   if( dir == 2 )
   {
     string buf;
     buf.assign( word );

     string::const_iterator iter1 = vdata[0].word.begin();
     string::const_iterator iter2 = buf.begin();

     string::const_iterator iter_end1 = vdata[0].word.end();
     string::const_iterator iter_end2 = buf.end();

     lexicographical_compare( iter2, iter_end2, iter1, iter_end1 ) ? dir = 0:dir = 1;

   }

  int m;
  int buf_index = fpos_index;
  bool found = false;

    switch(dir)
     {
       case 0: m = -1; break;//from up
       case 1: m =  1; break;//to down

     }

     do
     {

        buf_index += m;
        LoadNextN( buf_index );
        found = process( word );

        if( (buf_index < 0) || ( buf_index > _fpos.size()-1 ) )   break;

     } while( !found );

    if ( !found )
    {
      buf_index = fpos_index;
      LoadNextN( buf_index );
    }

    return buf_index;
 }


 void CLugat::SetRow  ( unsigned int n)
 {
     _row = n;
 }

 void CLugat::SetNextN( short n )
 {
   this->next_n = n;
 }

 int CLugat::GetNextN()
 {
   return next_n;
 }

 int CLugat::GetFposNum()
 {
   return _fpos.size()-1;
 }

 const char* CLugat::Get1MatchWord( short which)
 {
   const char* buf;
   switch(which)
   {
     case 0:  buf = res1->word.c_str();
     case 1:  buf = vdata[_row].word.c_str();
   }
   return buf;
 }

 char* CLugat::GetTrans( bool isFormatted)
 {
    ifTrans.clear();
    ifTrans.seekg(_trans_pos);
    ifTrans.getline(_trans, TRANS_SIZE);

    return isFormatted ? format(";"): _trans;
 }

//ingilizcha kalimalarni tallafuzi
const char* CLugat::GetIPA( short which)
{
   return (const char*) vdata[_row].phone_trans;
}

 unsigned int CLugat::GetRow()
 {
   return _row;
 }

 vector<_data>& CLugat::GetDataSeq()
 {
     return vdata;
 }

 char * CLugat::format(char *delim)
 {
    gchar** tokens;
    gchar *gstr = NULL;
    string str;

    if (_trans_pos != -1 )
    {
        tokens = g_strsplit( _trans, delim, -1);
        short num_tokens = g_strv_length(tokens);

        for(int i = 0; i < num_tokens; i++)
        {
            int c = i;
            ++c;
            str.assign( (char*)g_strdup_printf("%d.", c) ) ;

            tokens[i] = g_strconcat((gchar*)str.c_str() , tokens[i], (gchar*)delim, NULL);

        }

        gstr = g_strjoinv(NULL, tokens);

        g_strfreev( tokens ); //free up
        g_stpcpy((gchar*)_trans, (char*)gstr);

        g_free(gstr);
    }

    return (char*) g_strdelimit(_trans, delim, '\n' );
 }
