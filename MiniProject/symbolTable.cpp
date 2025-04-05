#include <bits/stdc++.h>
using namespace std;

struct Symbol
{
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

class SymbolTable
{
    unordered_map<int, unordered_map<string, Symbol>> table;
    stack<int> scopeStack;
    int currScope;
    uintptr_t currentMemoryAddress;

public:
    SymbolTable()
    {
        currentMemoryAddress = 0x1000;
        currScope = 0;
        scopeStack.push(currScope);
    }

    uintptr_t allocateMemory(string tokenType)
    {
        uintptr_t assignedAddress = currentMemoryAddress;

        if (tokenType == "INT")
        {
            currentMemoryAddress += 4;
        }
        else if (tokenType == "FLOAT")
        {
            currentMemoryAddress += 8;
        }

        return assignedAddress;
    }

    int getCurrentScope()
    {
        return currScope;
    }

    void enterScope()
    {
        currScope++;
        scopeStack.push(currScope);
    }

    void exitScope()
    {
        if (!scopeStack.empty())
        {
            scopeStack.pop();
            if (!scopeStack.empty())
                currScope = scopeStack.top();
            else
                currScope = 0;
        }
    }

    int getVariableScope(const string &name)
    {
        stack<int> tempStack = scopeStack;
        while (!tempStack.empty())
        {
            int scopeLevel = tempStack.top();
            tempStack.pop();

            if (table[scopeLevel].find(name) != table[scopeLevel].end())
            {
                return scopeLevel;
            }
        }
        return -1;
    }

    void insert(string tokenType, string name, string value, int line, int pos)
    {
        // Check if the variable exists in the current scope
        if (table[currScope].find(name) != table[currScope].end())
        {
            table[currScope][name].value = value;
            return;
        }

        // Check if the value corresponds to an existing variable in any scope
        if (exists(value))
        {
            string existingValue = getValue(value);
            if (!existingValue.empty())
            {
                value = existingValue;
            }
        }

        // Allocate memory and insert the variable in the current scope
        uintptr_t memAddress = allocateMemory(tokenType);
        table[currScope][name] = {tokenType, name, value, false, line, pos, currScope, memAddress};
    }

    void markUsed(string name, int line, int pos)
    {
        stack<int> tempStack = scopeStack;
        while (!tempStack.empty())
        {
            int scopeLevel = tempStack.top();
            tempStack.pop();
            if (table[scopeLevel].find(name) != table[scopeLevel].end())
            {
                table[scopeLevel][name].isUsed = true;
                table[scopeLevel][name].references.push_back({line, pos, currScope});
                return;
            }
        }
    }

    string getType(const string &varName)
    {
        stack<int> tempStack = scopeStack;

        while (!tempStack.empty())
        {
            int scopeLevel = tempStack.top();
            tempStack.pop();

            if (table[scopeLevel].find(varName) != table[scopeLevel].end())
            {
                return table[scopeLevel][varName].tokenType;
            }
        }
        return "UNKNOWN";
    }

    bool exists(string name)
    {
        stack<int> tempStack = scopeStack;
        while (!tempStack.empty())
        {
            int scopeLevel = tempStack.top();
            tempStack.pop();

            if (table[scopeLevel].find(name) != table[scopeLevel].end())
                return true;
        }
        return false;
    }

    string getValue(string name)
    {
        stack<int> tempStack = scopeStack;
        while (!tempStack.empty())
        {
            int scopeLevel = tempStack.top();
            tempStack.pop();

            if (table[scopeLevel].find(name) != table[scopeLevel].end())
            {
                return table[scopeLevel][name].value;
            }
        }
        return "";
    }

    void updateValue(string name, string value, int line, int pos)
    {
        int varScope = getVariableScope(name);
        if (varScope != -1)
        {
            Symbol &sym = table[varScope][name];
            if (sym.value.empty())
            {
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