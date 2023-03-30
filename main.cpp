#include <iostream>
#include <set>
#include <utility>
#include <vector>
#include <sstream>
#include <cstring>
#include <map>
#include <queue>
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
    void print() {
        cout<<"from:"<<from<<endl;
        cout<<"0:"<<endl;
        for (auto q: toZero) {
            cout<<q<<endl;
        }
        cout<<"1:"<<endl;
        for (auto q: toOne) {
            cout<<q<<endl;
        }
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
        //cout<<"from:"<<from<<" zero:"<<zero<<" one:"<<one<<endl;
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
    int end;
    vector<set<int>> states;
    vector<DFAFunctionLine> functionTable;
    void addTo(const set<int> &theSet, int &to) {
        if (theSet.size()==1) {
            for (auto q: theSet) {
                to=q;
            }
        } else if (theSet.empty()) {
            to=-1;
        } else {
            int index=stateFind(states, theSet);
            if (index==-1) {
                states.push_back(theSet);
                to=states.size()-1;
            } else {
                to=index;
            }
        }
    }
    int insertState(set<int> stateToInsert) {
        auto index=stateFind(states, stateToInsert);
        if (index==-1) {
            index=states.size();
            states.push_back(stateToInsert);
        }
        return index;
    }
    void deleteUnreachableStates() {
        vector<bool> reachable(states.size(), false);
        queue<int> q;
        q.push(start);
        reachable[start]=true;
        while (!q.empty()) {
            int cur=q.front();
            q.pop();
            if (functionTable[cur].to0!=-1 && !reachable[functionTable[cur].to0]) {
                reachable[functionTable[cur].to0]=true;
                q.push(functionTable[cur].to0);
            }
            if (functionTable[cur].to1!=-1 && !reachable[functionTable[cur].to1]) {
                reachable[functionTable[cur].to1]=true;
                q.push(functionTable[cur].to1);
            }
        }
        for (auto &p: functionTable) {
            int index=p.from;
            if (!reachable[index]) {
                p.reachable=false;
            }
        }
    }
    void convertFromNFA(NFA input) {
        auto NFATable=input.getFunctionTable();
        set<int> startState=input.getStartState();
        auto originalStatesCount=input.getStates().size();
        for (int i=0; i<originalStatesCount; i++) {
            states.push_back(set<int>{i});
        }
        int startIndex=stateFind(states, input.getStartState());
        if (startIndex==-1) {
            startIndex=states.size();
            states.push_back(input.getStartState());
        }
        start=startIndex;
        int endIndex=stateFind(states, input.getEndStates());
        if (endIndex==-1) {
            endIndex=states.size();
            states.push_back(input.getEndStates());
        }
        end=endIndex;
        for (const auto &line: input.getFunctionTable()) {
            DFAFunctionLine theLine{};
            theLine.from=line.from;
            set<int> zeroSet=line.toZero;
            set<int> oneSet=line.toOne;
            addTo(zeroSet, theLine.to0);
            addTo(oneSet, theLine.to1);
            functionTable.push_back(theLine);
        }
        for (int i=originalStatesCount; i<states.size(); i++) {
            set<int> toZero;
            set<int> toOne;
            auto theState=states[i];
            for (auto q: theState) {
                auto index=functionTable[q].to0;
                if (index!=-1) {
                    auto toZeroSet=states[index];
                    for (auto p: toZeroSet) {
                        toZero.insert(p);
                    }
                }
                index=functionTable[q].to1;
                if (index!=-1) {
                    auto toOneSet=states[index];
                    for (auto p: toOneSet) {
                        toOne.insert(p);
                    }
                }
            }
            auto zeroIndex=insertState(toZero);
            auto oneIndex=insertState(toOne);
            DFAFunctionLine line{i, zeroIndex, oneIndex};
            functionTable.push_back(line);
        }
    }
public:
    DFA(const NFA &input) {
        convertFromNFA(input);
        deleteUnreachableStates();
    }
    void printTable() {
        cout<<"\t\t0\t1"<<endl;
        for (auto q: functionTable) {
            if (q.reachable) {
                if (q.from==start) {
                    cout<<"(s)";
                } else {
                    bool isEnd=false;
                    auto fromStates=states[q.from];
                    for (auto p: fromStates) {
                        if (p==end) {
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
};
int main() {
    NFA input;
    NFAInput(cin, input);
    auto output=DFA(input);
    output.printTable();
}