//
// Created by lukas on 2/17/19.
//

#ifndef OPENMS_HOST_MZTABPROTEINROW_H
#define OPENMS_HOST_MZTABPROTEINROW_H

#include <string>
#include <map>

using namespace std;

namespace OpenNS
{
  struct MzTabProteinRow
  {
    const string accession;
    const string description;
    const string taxid;
    const string species;
    const string database;
    const string database_version;
    const string search_engine;
    const multimap<string, double> best_search_engine_score; // TODO Score type dedicated class ?
    const string ambiguity_members; // TODO Type?
    
  };
}
#endif //OPENMS_HOST_MZTABPROTEINROW_H
