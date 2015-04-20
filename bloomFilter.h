#ifndef BLOOMFILTER_H_
#define BLOOMFILTER_H_ 

void initPruning(int nGraphs);
void freePruning();

int hashID(const int elementID);
void addToPruningSet(const int elementID, const int index);
void resetPruningSet(const int index);
void initialAddToPruningSet(const int elementID, const int index);
char containedInPruningSet(const int elementID, const int index);
char isSubset(const int fingerPrint, const int index);
char isEmpty(const int index);

#endif
