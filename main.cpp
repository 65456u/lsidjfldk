#include <iostream>
#include <set>
#include <utility>
#include <vector>
#include <sstream>
#include <cstring>
#include <map>
#include <queue>
#include <algorithm>
using namespace std;
typedef enum {
    startState, endState, other
} stateType;
struct NFAFunctionLine {
    int from;
    set<int> toZero;
    set<int> toOne;
    NFAFunctionLine(int from_state, set<int> to0, set<int> to1) {
        from=from_state;
        toZero=std::move(to0);
        toOne=std::move(to1);
    }
};
class NFA {
    set<int> startStates;
    set<int> endStates;
    set<int> states;
    vector<NFAFunctionLine> functionTable;
public:
    void addStartState(int state) {
        startStates.insert(state);
    }
    void addEndState(int state) {
        endStates.insert(state);
    }
    void addState(int state) {
        states.insert(state);
    }
    void addFunctionLine(const NFAFunctionLine &theLine) {
        functionTable.push_back(theLine);
    }
    set<int> getStartState() {
        return startStates;
    }
    set<int> getEndStates() {
        return endStates;
    }
    set<int> getStates() {
        return states;
    }
    vector<NFAFunctionLine> getFunctionTable() {
        return functionTable;
    }
};
void NFAInput(istream &istream, NFA &nfa);
stateType fromInput(string fromString, int &from) {
    string buf;
    stateType type=other;
    if (fromString[0]=='(') {
        switch (fromString[1]) {
            case 's':
                type=startState;
                break;
            case 'e':
                type=endState;
                break;
        }
        buf=fromString.substr(5);
        from=stoi(buf);
    } else {
        buf=fromString.substr(2);
        from=stoi(buf);
    }
    return type;
}
vector<string> split(const string &str, const string &delim) {
    vector<string> res;
    if (str.empty()) return res;
    //convert string to char *
    char *strs=new char[str.length()+1];
    strcpy(strs, str.c_str());

    char *d=new char[delim.length()+1];
    strcpy(d, delim.c_str());

    char *p=strtok(strs, d);
    while (p) {
        string s=p; //convert the splited string to string
        res.push_back(s); //store into the targeted vector
        p=strtok(nullptr, d);
    }

    return res;
}
set<int> listInput(const string &bufString) {
    set<int> theSet;
    auto buffers=split(bufString, string(","));
    for (auto q: buffers) {
        if (q[0]=='[') {
            auto r=q.substr(2);
            auto p=stoi(r);
            theSet.insert(p);
        } else if (q[0]=='N') {
            return theSet;
        } else {
            auto r=q.substr(1);
            auto p=stoi(r);
            theSet.insert(p);
        }
    }
    return theSet;
}
void NFAInput(istream &istream, NFA &nfa) {
    string buf;
    getline(istream, buf); // drop the first line
    while (getline(istream, buf)) {
        stringstream ss(buf);
        string from, zero, one;
        ss>>from>>zero>>one;
        int fromState;
        auto theType=fromInput(from, fromState);
        nfa.addState(fromState);
        auto zeroList=listInput(zero);
        auto oneList=listInput(one);
        switch (theType) {
            case startState:
                nfa.addStartState(fromState);
                break;
            case endState:
                nfa.addEndState(fromState);
                break;
            case other:
                break;
        }
        NFAFunctionLine theLine(fromState, zeroList, oneList);
        nfa.addFunctionLine(theLine);
    }
}

int stateFind(const vector<set<int>> &states, const set<int> &toFind) {
    for (int i=0; i<states.size(); i++) {
        if (states[i]==toFind) {
            return i;
        }
    }
    return -1;
}
struct DFAFunctionLine {
    int from;
    int to0;
    int to1;
    bool reachable=true;
};
class DFA {
    int start;
    set<int> end;
    vector<set<int>> states;
    vector<DFAFunctionLine> functionTable;
    int insertState(set<int> stateToInsert) {
        auto index=stateFind(states, stateToInsert);
        if (index==-1) {
            index=states.size();
            states.push_back(stateToInsert);
        }
        return index;
    }
    void convertFromNFA(NFA input) {
        queue<set<int>> statesBuffer;
        for (auto q: input.getStates()) {
            states.push_back(set<int>{q});
        }
        start=stateFind(states, input.getStartState());
        statesBuffer.push(states[0]);
        auto newFunctionTable=input.getFunctionTable();
        set<int> reachedState;
        while (!statesBuffer.empty()) {
            auto fromState=statesBuffer.front();
            statesBuffer.pop();
            auto fromStateIndex=insertState(fromState);
            if (reachedState.find(fromStateIndex)==reachedState.end()) {
                reachedState.insert(fromStateIndex);
                auto NFAFunctionTable=input.getFunctionTable();
                set<int> toOnes, toZeros;
                for (auto q: fromState) {
                    auto relatedFunctionLine=NFAFunctionTable[q];
                    auto NFAToOne=relatedFunctionLine.toOne;
                    auto NFAToZero=relatedFunctionLine.toZero;
                    toOnes.insert(NFAToOne.begin(), NFAToOne.end());
                    toZeros.insert(NFAToZero.begin(), NFAToZero.end());
                }
                int toOneIndex=-1;
                int toZeroIndex=-1;
                if (!toZeros.empty()) {
                    toZeroIndex=insertState(toZeros);
                }
                if (!toOnes.empty()) {
                    toOneIndex=insertState(toOnes);
                }
                DFAFunctionLine theLine{fromStateIndex, toZeroIndex, toOneIndex};
                functionTable.push_back(theLine);
                statesBuffer.push(toZeros);
                statesBuffer.push(toOnes);
            }
        }
        auto NFAEndStates=input.getEndStates();
        end.insert(NFAEndStates.begin(),NFAEndStates.end());
    }
public:
    DFA(const NFA &input) {
        convertFromNFA(input);
    }
    void printTable() {
        cout<<"\t\t0\t1"<<endl;
        for (auto q: functionTable) {
            if (!(q.to1==-1&&q.to0==-1)) {
                if (q.reachable) {
                    if (q.from==start) {
                        cout<<"(s)";
                    } else {
                        bool isEnd=false;
                        auto fromStates=states[q.from];
                        for (auto p: fromStates) {
                            if (end.find(p)!=end.end()) {
                                isEnd=true;
                                break;
                            }
                        }
                        if (isEnd) {
                            cout<<"(e)";
                        }
                    }
                    cout<<'q'<<q.from;
                    cout<<'\t';
                    if (q.to0==-1) {
                        cout<<'N';
                    } else {
                        cout<<'q'<<q.to0;
                    }
                    cout<<'\t';
                    if (q.to1==-1) {
                        cout<<'N';
                    } else {
                        cout<<'q'<<q.to1;
                    }
                    cout<<endl;
                }
            }
        }
    }
    void sort(){
        std::sort(functionTable.begin(), functionTable.end(), [](DFAFunctionLine a,DFAFunctionLine b) {return a.from<b.from;});
    }
};
int main() {
    NFA input;
    NFAInput(cin, input);
    auto output=DFA(input);
    output.sort();
    output.printTable();
}