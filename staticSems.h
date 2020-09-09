#ifndef STATICSEMS_H
#define STATICSEMS_H
#include <vector>
#include "token.h"
#include "node.h"

using namespace std;

//This includes the token of the variable and the line number
//it is on in the file.
typedef struct Stacked{
  Token token;
  int line_number;
}stacked;

extern vector<stacked> stock;

void static_sems(node*);
void declare_var(stacked);
int lookup_var(stacked);

#endif
