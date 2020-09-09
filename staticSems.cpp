#include <iostream>
#include <stdlib.h>
#include <vector>
#include "staticSems.h"
#include "token.h"
#include "node.h"

using namespace std;

//This variable is for if a single variable has been defined
static stacked temp;

//The stack, which will hold all the variables defined.
vector<stacked> stack;

//Current location on the stack
int stack_location = 0;

//Variables currently on the stack
int stack_vars = 0;

void static_sems(node *root){

    if(root->literal == "<program>"){

        cout << endl << "root is <program>" << endl;
        static_sems(root->child_1); //<vars>
        static_sems(root->child_2); //<block>

        cout << "+=+=+ SEMANTICS are OK +=+=+" << endl;
        return;
    }

    //<vars> => empty | declare Identifier := Integer ; <vars>
    if(root->literal == "<vars>"){
        //Check if the token is an empty production or not
        temp.token = root->all_toks.front();

        if(temp.token.tok_string != "EMPTY"){

            //Declare variable if it has not already been declared
            declare_var(temp);

            //Increment the number of variables on the stack
            stack_vars++;

            //process <vars>
            static_sems(root->child_1);
        }
    }

    //<block> => { <vars> <stats> }
    if(root->literal == "<block>"){

        static_sems(root->child_1); //<vars>
        static_sems(root->child_2); //<stats>

        return;
    }

    //<expr> => <N> - <expr> | <N>
    if(root->literal == "<expr>"){
        //If empty, then production was <N> - <expr>
        if(root->all_toks.empty()){
            static_sems(root->child_1); //process <N>
        }else{
            static_sems(root->child_1); //process <N>
            static_sems(root->child_2); //process <expr>
        }
    }

    //<N> => <A> / <N> | <A> * <N> | <A>
    if (root->literal == "<N>"){
        //If empty, then the production was <N> => <A>
        if (root->all_toks.empty()){
            static_sems(root->child_1); //process <A>
        }else{
            static_sems(root->child_1); //process <A>
            static_sems(root->child_2); //process <N>
        }
    }

    //A => <M> + <A> | <M>
    if(root->literal == "<A>"){
        //If empty, then production was <M> + <A>
        if(root->all_toks.empty()){
            static_sems(root->child_1); //process <M>
        }else{
            static_sems(root->child_1); //process <M>
            static_sems(root->child_2); //process <A>
        }
    }

    //<M> => * <M> | <R>
    if(root->literal == "<M>"){

        static_sems(root->child_1); //process <M> or <R>
    }

    //<R> => ( <expr> ) | Identifier | Integer
    if (root->literal == "<R>"){
        //If empty, then production was <R> => ( <expr> )
        if (root->all_toks.empty()){
            static_sems(root->child_1); //process <expr>
        }else{
            //Otherwise, if an identifier token was used, verify that it
            //was already defined and can be accessed in this scope
            temp.token = root->all_toks.front();

            if (temp.token.ID == ID_tk){
                //Check to determine if this variable may be accessed
                lookup_var(temp);
            }

            return;
        }
    }

    //<stats> => <stat> <mStat>
    if(root->literal == "<stats>"){
        static_sems(root->child_1); //process <stat>
        static_sems(root->child_2); //process <mStat>
    }

    //<mStat> => empty | <stat> <mStat>
    if(root->literal == "<mStat>"){
        //The <mStat> production will normally not have any tokens,
        //but tokens.empty() will return FALSE when the empty production is present.
        if (root->all_toks.empty()){
            static_sems(root->child_1); //process <stat>
            static_sems(root->child_2); //process <mStat>
        }
    }

    //<stat> => <in>; | <out>; | <block> | <if>; |
        //<loop>; | <assign>; | <goto>; | <label>;
    if(root->literal == "<stat>"){
        static_sems(root->child_1); //process the RHS production
    }

    //<in> =>  in Identifier
    if(root->literal == "<in>"){
        //Verify that the identifier exists, and was declared
        temp.token = root->all_toks.front();
        lookup_var(temp);

        return;
    }

    //<out> => out <expr>
    if(root->literal == "<out>"){
        static_sems(root->child_1); //process <expr>
    }

    //<if> => iffy [ <expr> <RO> <expr>] then <stat>
    if(root->literal == "<if>"){

        static_sems(root->child_1); //process <expr>
        static_sems(root->child_2); //process <RO>
        static_sems(root->child_3); //process <expr>
        static_sems(root->child_4); //process <stat>
    }

    //<loop> => loop [ <expr> <RO> <expr> ] <stat>
    if(root->literal == "<loop>"){

        static_sems(root->child_1); //process <expr>
        static_sems(root->child_2); //process <RO>
        static_sems(root->child_3); //process <expr>
        static_sems(root->child_4); //process <stat>
    }

    //<assign> => Identifier := <expr>
    if(root->literal == "<assign>"){
        //cout << "<assign>" << endl;
        //Check if the identifier has been defined
        temp.token = root->all_toks.front();

        int define = lookup_var(temp);

        //if variable isn't found, then it wasn't declared else push on stack
        if(define == -1){
            cout << "Semantics Error: The -*" << temp.token.tok_string
            << "*- has NOT been declared in this scope." << endl;
            exit(EXIT_FAILURE);
        }else{
            //Push the variable onto the stack and increment the stack location.
            stack.push_back(temp);
            stack_location++;
            //print_stack();
        }

        static_sems(root->child_1); //process <expr>

        return;
    }

    //<label> => label Identifier

    //<goto => goto Identifier

    //<RO> => < | < < (two tokens >) | > | > > (two tokens) | == (one token==)

}//end static sems

void declare_var(stacked vars){

    cout << "Declare " << vars.token.tok_string << endl;

    int define = lookup_var(vars);

    if(define != -1){
        cout << "Semantics Error: The -*" << vars.token.tok_string
      << "*- has already been defined in this scope." << endl;
        exit(EXIT_FAILURE);
    }else{
        //Push the variable onto the stack and increment the stack location.
        stack.push_back(vars);
        stack_location++;
        //print_stack();

    }
}

int lookup_var(stacked vars){

    //cout << "Looking up " << vars.token.tok_string << endl;
    for(int iter = 0; iter < stack.size(); iter++){
        //If the given identifier matches the identifier at this location
        //in the stack, return the location of this variable in the stack
        if((stack.at(iter).token.tok_string == vars.token.tok_string)){
            return iter;
        }
    }

    return -1;
}//end find_var


