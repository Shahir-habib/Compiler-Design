#include <bits/stdc++.h>
#include "symbolTable.cpp"
#include "lr1itemset.cpp"
#include "parsingTable.cpp"
using namespace std;
SymbolTable symbolTable;
struct Row
{
    string type;  // First column
    string value; // Second column
    int line;     // Third column
    int position; // Fourth column
};

// Helper function to trim whitespace from a string
string trim(const string &str)
{
    size_t first = str.find_first_not_of(' ');
    if (first == string::npos)
        return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, last - first + 1);
}

using Item = tuple<int, string, vector<string>, int, string>;

void handleToken(Row &r, vector<Row> &tokens, int tokenIndex)
{
    string type = r.type;
    string value = r.value;
    int line = r.line;
    int pos = r.position;
    if (value == "return")
        return;
    if (type == "LBRACE")
        symbolTable.enterScope();
    else if (type == "RBRACE")
        symbolTable.exitScope();
    else if (type == "INT" || type == "FLOAT")
    {
        if (tokenIndex + 1 < tokens.size() && tokens[tokenIndex + 1].type == "ID")
        {
            string varName = (tokens[tokenIndex + 1].value);
            symbolTable.insert(type, varName, "", line, pos);
        }
    }
    else if (type == "ID")
    {
        if (!symbolTable.exists(value))
        {
            cerr << "Error: Undeclared variable '" << value << "' at line " << line << endl;
        }
        else
        {
            symbolTable.markUsed(value, line, pos);
        }
    }
    else if (type == "EQ")
    {
        if (tokenIndex - 1 >= 0 && (tokens[tokenIndex - 1].type) == "ID")
        {
            string lhsVar = (tokens[tokenIndex - 1].value);
            string expression = "";
            bool containsVariable = false;

            int i = tokenIndex + 1;
            while (i < tokens.size() && (tokens[i].type) != "SEMI")
            {
                string tType = (tokens[i].type);
                string tVal = (tokens[i].value);

                if (tType == "ID")
                {
                    if (!symbolTable.exists(tVal))
                    {
                        cerr << "\n\nError: Undeclared variable '" << tVal << "' at line " << line << "\n\n";
                        return;
                    }
                    containsVariable = true;
                    symbolTable.markUsed(tVal, line, (tokens[i].position));
                }

                expression += tVal;
                i++;
            }

            if (containsVariable)
            {
                symbolTable.updateValue(lhsVar, "UNKNOWN", line, pos);
            }
            else
            {
                symbolTable.updateValue(lhsVar, expression, line, pos);
            }
        }
    }
}
void parseInput(vector<Row> &tokens,
                const map<pair<int, string>, string> &parsingTable,
                const map<int, pair<string, vector<string>>> &productions, ofstream &outFile)
{
    vector<string> tkns;
    for (auto x : tokens)
    {
        tkns.push_back(x.type);
    }
    stack<string> stck;
    stck.push("0");
    int tknindex = 0;
    tkns.push_back("$");
    string lookahead = tkns[tknindex];
    cout << "Parsing Input........" << endl;
    while (true)
    {
        Row r = tokens[tknindex];
        outFile << "Lookahead : " << lookahead << endl;
        // Print the current stack
        stack<string> temp = stck; // Create a copy of the stack for printing
        string stack_str = "";
        while (!temp.empty())
        {
            stack_str = temp.top() + " " + stack_str;
            temp.pop();
        }
        outFile << "\t\t| ";

        string topstck = stck.top();
        int topst;
        try
        {
            topst = stoi(topstck);
        }
        catch (const std::invalid_argument &e)
        {
            cerr << "Error: Invalid state value on stack: 😢😢😢" << topstck << endl;
            return;
        }

        // Handle Shift action
        if (parsingTable.find({topst, lookahead}) != parsingTable.end() && parsingTable.at({topst, lookahead})[0] == 's')
        {
            outFile << "Action : " << parsingTable.at({topst, lookahead}) << endl;
            outFile << "Shift  : " << lookahead << endl;
            stck.push(lookahead);
            stck.push(parsingTable.at({topst, lookahead}).substr(1));
            handleToken(r, tokens, tknindex);
            tknindex++;
            lookahead = tkns[tknindex];
        }
        // Handle Reduce action
        else if (parsingTable.find({topst, lookahead}) != parsingTable.end() && parsingTable.at({topst, lookahead})[0] == 'r')
        {
            string production = parsingTable.at({topst, lookahead});
            int prod_no = stoi(production.substr(1));
            string head = productions.at(prod_no).first;
            vector<string> body = productions.at(prod_no).second;
            outFile << "Action : " << parsingTable.at({topst, lookahead}) << endl;
            outFile << "Reduce by " << head << " -> ";
            for (const auto &symbol : body)
            {
                outFile << symbol << " ";
            }
            outFile << endl;

            for (int i = 0; i < body.size(); i++)
                stck.pop();
            for (int i = 0; i < body.size(); i++)
                stck.pop();
            topst = stoi(stck.top());
            stck.push(head);
            stck.push(parsingTable.at({topst, head}));
        }
        // Handle Accept action
        else if (parsingTable.find({topst, lookahead}) != parsingTable.end() && parsingTable.at({topst, lookahead}) == "Accept")
        {
            outFile << "Accepted 👌👌👌👌" << endl;
            ofstream outFile("symbol_table.txt");
            if (outFile.is_open())
            {
                symbolTable.printTable(outFile);
                cout << "Symbol Table written to file 'symbol_table.txt'" << endl;
                cout << "--------------------------------------------------------------" << endl;
                outFile.close();
            }
            else
            {
                cerr << "Error: Unable to open file.\n";
            }
            break;
        }
        // Completed just epsilon left
        else
        {
            if (lookahead == "$")
            {
                outFile << "Accepted 👌👌👌👌" << endl;
                ofstream outFile("symbol_table.txt");
                if (outFile.is_open())
                {
                    symbolTable.printTable(outFile);
                    cout << "Symbol Table written to file 'symbol_table.txt'" << endl;
                    outFile.close();
                }
                else
                {
                    cerr << "Error: Unable to open file.\n";
                }
                break;
            }
            outFile << "Error: Unexpected token '" << lookahead << "' at state " << topst << endl;
            break;
        }
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " <grammar_file> <string_file>" << endl;
        return 1;
    }
    ifstream file(argv[1]);
    ifstream stringFile(argv[2]);
    if (!stringFile.is_open())
    {
        cerr << "Error opening string file!" << endl;
        return 1;
    }
    if (!file.is_open())
    {
        cerr << "Error opening file!" << endl;
        return 1;
    }
    string line;
    vector<string> nonTerminals, terminals;
    set<string> terminalSet;
    map<string, set<string>> first;
    map<string, set<string>> follow;
    map<int, vector<Item>> states;
    map<int, pair<string, vector<string>>> productions;
    map<pair<int, string>, string> parsingTable;
    vector<Row> inputTokens;
    Lr1itemset lr1itemset(nonTerminals, terminals, terminalSet, first, follow, states, productions);
    ParsingTable parsingTableobj(nonTerminals, terminals, terminalSet, first, follow, states, productions);
    string token;
    int c = 0;
    lr1itemset.readterminals_and_nonterminals(nonTerminals, terminals, terminalSet, file);
    while (getline(stringFile, line))
    {
        stringstream ss(line);
        string cell;
        Row row;
        if (c > 2)
        {
            getline(ss, cell, '|');
            getline(ss, cell, '|');
            row.type = trim(cell);
            getline(ss, cell, '|');
            row.value = trim(cell);
            getline(ss, cell, '|');
            row.line = stoi(cell);
            getline(ss, cell, '|');
            row.position = stoi(cell);
            inputTokens.push_back(row);
        }
        c++;
    }
    ofstream outFile("lr1itemsets.txt");
    ofstream parsedFile("parsingresult.txt");
    if (!parsedFile)
    {
        cerr << "Error opening file for writing!" << endl;
        return 1;
    }
    if (!outFile)
    {
        cerr << "Error opening file for writing!" << endl;
        return 1;
    }
    outFile << "______N O N T E R M I N A L__________\n";
    for (auto &x : nonTerminals)
        outFile << x << "\n";
    outFile << "Start Symbol :    " << nonTerminals[0] << endl;
    outFile << "______T E R M I N A L__________\n";
    for (auto &x : terminals)
        outFile << x << "\n";
    outFile << "______P R O D U C T I O N S __________\n";
    lr1itemset.read_productions(productions, file);
    for (auto x : productions)
    {
        outFile << x.first << " " << x.second.first << " -> ";
        for (const auto &symbol : x.second.second)
        {
            outFile << symbol << " ";
        }
        outFile << "\n";
    }
    lr1itemset.find_first(productions, nonTerminals, terminals, first);
    lr1itemset.find_follow(productions, nonTerminals, terminals, first, follow);

    outFile << "______F I R S T__________\n";
    for (auto &x : first)
    {
        outFile << x.first << " : ";
        for (auto &y : x.second)
        {
            outFile << y << " ";
        }
        outFile << "\n";
    }
    // Print FIRST sets
    outFile << "______F O L L O W__________\n";
    for (auto &x : follow)
    {
        outFile << x.first << " : ";
        for (auto &y : x.second)
        {
            outFile << y << " ";
        }
        outFile << "\n";
    }
    parsingTableobj.lr1ItemSet(productions, nonTerminals, terminals, first, states);
    parsingTableobj.removeDuplicates(states);
    outFile << "\n_______LR(1) Item Sets_______\n";

    lr1itemset.printStates(states, outFile);

    cout << "First and Follow Sets have been written to 'lr1itemsets.txt'." << endl;
    cout << "--------------------------------------------------------------" << endl;
    cout << "LR(1) Item Sets have been written to 'lr1itemsets.txt'." << endl;
    cout << "--------------------------------------------------------------" << endl;

    parsingTableobj.constructlr1parsingTable(states, productions, nonTerminals, terminals, terminalSet, first, follow, parsingTable);
    cout << "LR(1) Parsing Table has been written to 'parsingtable.txt'." << endl;
    cout << "--------------------------------------------------------------" << endl;

    parseInput(inputTokens, parsingTable, productions, parsedFile);
    cout << "Parsing result has been written to 'parsingresult.txt'." << endl;
    cout << "--------------------------------------------------------------" << endl;
    outFile.close();
    stringFile.close();
    parsedFile.close();
    return 0;
}
