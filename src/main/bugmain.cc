#include <ptree.h>
#include <map>
#include <string>
#include <token-tree-map.h>
#include <clone-context-php.h>
#include <algorithm>
#include <cctype>

using namespace std;

map<string,int> name2id;
map<int,string> id2name;
#ifdef JAVA
string identifierTypeName = "ID_TK";
#else
#ifdef PHP
string identifierTypeName = "T_VARIABLE";
#else
string identifierTypeName = "IDENTIFIER";
#endif
#endif

static const char *cxtNodes[] = {
#ifdef JAVA 
#include "jcontextualNodes.h"
#else
#ifdef PHP
#include "../ptgen/php5/phpcontextualNodes.h"
#else
#include "ccontextualNodes.h"
#endif
#endif
};

/* For debugging use only */
ParseTree* global_tree_for_debugging;

static bool cloneClusterFiltered(TokenTreeMap & cls) {
  /* Some managieral problems caused a lot of confusion about which
     code is for which filters...and the several versions of the
     filters (especially for filter 4) caused the most trouble, the
     data (# bugs detected) in the paper may be a little off (not
     essential though. fix it when time is appropriate). 

     The code for filters should be better organized to avoid the
     above confusions...at least, use command-line options instead of
     re-compiling the code for enabling different filters.
  */

  if ( cls.rank>=4 /* <=> cls.buggy_score[1]>0 */ && cls.buggy_score[1]>2 ) /* Filter ALL */
    cls.rank -= 4;
  if ( cls.rank==0 // || cls.buggy_score[0]==0
       // individual filters should or should not preserve cases where other things are buggy?
       || ( /* cls.rank<4 && */ cls.buggy_score[0]==-8 ) /* Filter 1: none vs. none. 1 */
       || ( /* cls.rank<4 && */ (cls.buggy_score[0]==-4 || cls.buggy_score[0]==-5) ) /* Filter 2: loop/switch vs. none; deep "if" vs. none. 7.2 */
       || ( /* cls.rank<4 && */ (cls.buggy_score[0]==0 || cls.buggy_score[0]==1) && cls.buggy_score[3]<1 ) /* Filter 3: loop vs. cond+none. 7.1 */
       // filter 4 is in token-tree-map.C. 
       || ( /* cls.rank==4 && */ cls.buggy_score[1]>2 ) /* Filter 5: nVARs differ too much. 4.1 */
       || ( cls.buggy_score[2]<10 ) /* Filter 6: clones are too spatially close. 4.2 */
       )
    return true;
  else
    return false;
}

/* Decide whether to filter a clone cluster: for ONE cluster only;
   need a shell script to pass every clone cluster to this class. */
int main( int argc, char **argv )
{
  if ( argc>2 ) {
    cerr << "Usage: " << argv[0] << " [filter ID=0] [a clone cluster from stdin]" << endl
	 << "\t stdin is the input channel; stdout is the output channel." << endl;
    return 1;
  }
/*
  int fid = 0;
  if ( argc==2 )
    fid = atoi(argv[1]);
  if ( fid<0 || fid>2 )
    fid = 0;
*/
  id_init();
  TokenTreeMap::init_shared_data();
#ifdef JAVA
  TokenTreeMap_Java tt;
#else
#ifdef PHP
  ContextInconsistency_PHP tt;
#else
  TokenTreeMap tt;
#endif
#endif
  tt.initNodes(cxtNodes);
  tt.createFN2Tree();

/*
  bool filtered = false;
  switch (fid) {
  case 0:			// iff all filters agree:
    filtered = tt.isAllFiltered();
    break;
  case 1:
    isFilteredI(tt, 1, filtered);
    break;
  case 2:
    isFilteredI(tt, 2, filtered);
    break;
  case 3:
    isFilteredI(tt, 3, filtered);
    break;
  default:
    filtered = tt.isAnyFiltered();
    break;
  }
*/

  if ( !cloneClusterFiltered(tt) ) {
#ifdef HTML
    cout << "Rank score: " << tt.rank << " * " << tt.clusterbuffer.size() << " =" << tt.rank*tt.clusterbuffer.size()
       << "  buggy score: ";
    for (int i=0; i < NUM_BUGGY_SCORES; i++)
      cout << tt.buggy_score[i] << " ";
    cout << endl;
    for (int i=0; i < tt.clusterbuffer.size(); i++) {
      tt.clusterbuffer[i].out2html(cout);
    }
    cout << "<p></p>" << endl;
#else
    tt.outputCluster(cout);
    cout << endl;
#endif
  }

  return 0;
}
