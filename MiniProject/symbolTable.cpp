#include <bits/stdc++.h>
using namespace std;

struct Symbol {
    string tokenType;
    string name;
    string value;
    bool isUsed;
    int line;
    int pos;
    int scope;
    uintptr_t memoryAddress;
    vector<tuple<int, int, int>> references;
};

class SymbolTable {
    unordered_map<int, unordered_map<string, Symbol>> table; // scope -> (name -> Symbol)
    unordered_map<int, int> parentScope; // scope -> parent scope
    unordered_map<string, int> functionBaseScope; // function name -> base scope
    unordered_map<string, stack<int>> functionScopeStacks; // function name -> scope stack
    unordered_map<string, int> functionCurrentScope; // function name -> current scope
    uintptr_t currentMemoryAddress;
    int globalScopeCounter;
    string currentFunction;

public:
    SymbolTable() {
        currentMemoryAddress = 0x1000;
        globalScopeCounter = 0;
        functionBaseScope["global"] = globalScopeCounter;
        parentScope[globalScopeCounter] = -1;
        functionScopeStacks["global"].push(globalScopeCounter);
        functionCurrentScope["global"] = globalScopeCounter;
        currentFunction = "global";
    }

    uintptr_t allocateMemory(string tokenType) {
        uintptr_t assignedAddress = currentMemoryAddress;
        if (tokenType == "INT")
            currentMemoryAddress += 4;
        else if (tokenType == "FLOAT")
            currentMemoryAddress += 8;
        return assignedAddress;
    }

    void startFunction(const string& functionName) {
        currentFunction = functionName;
        globalScopeCounter++;
        int baseScope = globalScopeCounter;
        functionBaseScope[functionName] = baseScope;
        parentScope[baseScope] = functionBaseScope["global"];
        functionScopeStacks[functionName].push(baseScope);
        functionCurrentScope[functionName] = baseScope;
    }

    void endFunction() {
        currentFunction = "global";
    }

    void enterScope() {
        globalScopeCounter++;
        int parent = functionCurrentScope[currentFunction];
        parentScope[globalScopeCounter] = parent;
        functionScopeStacks[currentFunction].push(globalScopeCounter);
        functionCurrentScope[currentFunction] = globalScopeCounter;
    }

    void exitScope() {
        if (!functionScopeStacks[currentFunction].empty()) {
            functionScopeStacks[currentFunction].pop();
            if (!functionScopeStacks[currentFunction].empty())
                functionCurrentScope[currentFunction] = functionScopeStacks[currentFunction].top();
        }
    }

    int findScopeOfVariable(const string& name) {
        int scope = functionCurrentScope[currentFunction];
        while (scope >= 0) {
            if (table[scope].find(name) != table[scope].end())
                return scope;
            scope = parentScope.count(scope) ? parentScope[scope] : -1;
        }
        return -1;
    }

    void insert(string tokenType, string name, string value, int line, int pos) {
        int currScope = functionCurrentScope[currentFunction];
        if (table[currScope].find(name) != table[currScope].end()) {
            table[currScope][name].value = value;
            return;
        }

        int valScope = findScopeOfVariable(value);
        if (valScope != -1) value = table[valScope][value].value;

        uintptr_t memAddress = allocateMemory(tokenType);
        table[currScope][name] = {tokenType, name, value, false, line, pos, currScope, memAddress};
    }

    void markUsed(string name, int line, int pos) {
        int varScope = findScopeOfVariable(name);
        if (varScope != -1) {
            table[varScope][name].isUsed = true;
            table[varScope][name].references.emplace_back(line, pos, functionCurrentScope[currentFunction]);
        }
    }

    string getType(const string &varName) {
        int scope = findScopeOfVariable(varName);
        return (scope != -1) ? table[scope][varName].tokenType : "UNKNOWN";
    }

    bool exists(const string& name) {
        return findScopeOfVariable(name) != -1;
    }

    string getValue(const string& name) {
        int scope = findScopeOfVariable(name);
        return (scope != -1) ? table[scope][name].value : "";
    }

    void updateValue(const string& name, const string& value, int line, int pos) {
        int scope = findScopeOfVariable(name);
        if (scope != -1) {
            Symbol& sym = table[scope][name];
            if (sym.value.empty()) {
                sym.value = value;
                sym.line = line;
                sym.pos = pos;
            }
        }
    }

    void printTable(ostream &out)
    {
        out << "+-------------+--------+---------+------+-----+-------+------------------+\n";
        out << "| Token Type  | Name   |  Value  | Line | Pos | Scope |   Memory Addr    |\n";
        out << "+-------------+--------+---------+------+-----+-------+------------------+\n";

        vector<Symbol> allSymbols;
        for (const auto &scopeEntry : table)
        {
            for (const auto &varEntry : scopeEntry.second)
            {
                allSymbols.push_back(varEntry.second);
            }
        }

        // Sort by increasing memory address
        sort(allSymbols.begin(), allSymbols.end(), [](const Symbol &a, const Symbol &b)
             { return a.memoryAddress < b.memoryAddress; });

        for (auto &sym : allSymbols)
        {
            out << "| " << setw(11) << left << sym.tokenType
                << " | " << setw(6) << left << sym.name
                << " | " << setw(7) << left << sym.value
                << " | " << setw(4) << sym.line
                << " | " << setw(3) << sym.pos
                << " | " << setw(5) << sym.scope
                << " | " << "0x" << setw(8) << hex << sym.memoryAddress << dec << setfill(' ') << "       |\n";

            out << "+-------------+--------+---------+------+-----+-------+------------------+\n";

            if (!sym.references.empty())
            {
                out << "| Referenced at:                                                         |\n";
                out << "|                                                                        |\n";
                for (auto &ref : sym.references)
                {
                    int refLine, refPos, refScope;
                    tie(refLine, refPos, refScope) = ref;
                    out << "|    → Line " << setw(3) << refLine
                        << " , Pos " << setw(3) << refPos
                        << " , Scope " << setw(3) << refScope << "                                    |\n";
                }
                out << "+-------------+--------+---------+------+-----+-------+------------------+\n";
            }
        }
    }
};