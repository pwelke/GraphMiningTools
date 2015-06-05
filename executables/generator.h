#ifndef GENERATOR_H_
#define GENERATOR_H_

struct Graph* erdosRenyiWithLabels(int n, double p, int nVertexLabels, int nEdgeLabels, struct GraphPool* gp);
struct Graph* erdosRenyi(int n, double p, struct GraphPool* gp);
int main(int argc, char** argv);

#endif