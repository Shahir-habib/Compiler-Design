#include <bits/stdc++.h>
using namespace std;
using Item = tuple<int, string, vector<string>, int, string>;
class Lr1itemset
{
public:
    string line;
    vector<string> nonTerminals, terminals;
    set<string> terminalSet;
    map<string, set<string>> first;
    map<string, set<string>> follow;
    map<int, vector<Item>> states;
    map<int, pair<string, vector<string>>> productions;
    Lr1itemset(vector<string> &nonTerminals, vector<string> &terminals, set<string> &terminalSet,
               map<string, set<string>> &first, map<string, set<string>> &follow,
               map<int, vector<Item>> &states, map<int, pair<string, vector<string>>> &productions)
        : nonTerminals(nonTerminals), terminals(terminals), terminalSet(terminalSet),
          first(first), follow(follow), states(states), productions(productions)
    {
        // Initialize the Lr1itemset object
    }
    // Helper function to split a string into symbols based on spaces
    vector<string> splitSymbols(const string &body)
    {
        vector<string> symbols;
        stringstream ss(body);
        string symbol;
        while (ss >> symbol)
        {
            symbols.push_back(symbol);
        }
        return symbols;
    }
    string trim(const string &str)
    {
        size_t first = str.find_first_not_of(' ');
        if (first == string::npos)
            return "";
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, last - first + 1);
    }
    void readterminals_and_nonterminals(vector<string> &nonTerminals, vector<string> &terminals, set<string> &terminalSet, ifstream &file)
    {
        string line;
        getline(file, line);
        stringstream ss(line);
        string symbol;

        if (line.find("%tokens") != string::npos)
        {
            while (ss >> symbol)
            {
                if (symbol != "%tokens")
                {
                    terminals.push_back(symbol);
                }
            }
        }

        // Skip lines until %%
        while (getline(file, line))
        {
            if (line == "%%")
            {
                break;
            }
        }

        // Read productions and extract non-terminals
        while (getline(file, line))
        {
            if (line.empty())
                continue;

            stringstream prodStream(line);
            prodStream >> symbol; // Production head
            if (symbol.back() == ':')
            {
                symbol.pop_back(); // Remove the colon
            }
            if (symbol == "%%")
                break;
            if (symbol == ";" || symbol == "|")
                continue;
            if (find(nonTerminals.begin(), nonTerminals.end(), symbol) == nonTerminals.end())
            {
                nonTerminals.push_back(symbol);
            }

            string arrow;
            prodStream >> arrow; // Skip ':'

            while (prodStream >> symbol)
            {
                if (symbol == "\t;" || symbol == "|" || symbol.empty())
                    continue;
                if (find(nonTerminals.begin(), nonTerminals.end(), symbol) == nonTerminals.end())
                {
                    terminalSet.insert(symbol);
                }
            }
        }
    }

    void read_productions(map<int, pair<string, vector<string>>> &mp, ifstream &file)
    {
        file.seekg(0, ios::beg);
        vector<string> productions;
        bool insideGrammar = false;
        string line;

        // Read the file and extract productions
        while (getline(file, line))
        {
            line = trim(line);
            if (line == "%%")
            {
                if (insideGrammar)
                    break;
                insideGrammar = true;
                continue;
            }
            if (insideGrammar && !line.empty() && line != ";")
            {
                productions.push_back(line);
            }
        }
        file.close();

        // Add augmented start production if not present
        bool hasAugmentedStart = false;
        for (const string &prod : productions)
        {
            if (prod.find("S'") == 0)
            {
                hasAugmentedStart = true;
                break;
            }
        }
        if (!hasAugmentedStart)
        {
            productions.insert(productions.begin(), "S' : S");
        }

        // Format and output productions
        int count = 0; // Start counting from 0
        string head;
        for (const string &production : productions)
        {
            size_t pos = production.find(":");

            if (pos != string::npos)
            {
                head = trim(production.substr(0, pos));
                string body = trim(production.substr(pos + 1));

                // Split the body into symbols
                vector<string> bodySymbols = splitSymbols(body);

                // Insert into the map
                mp.insert({count, {head, bodySymbols}});
                count++;
            }
            if (production.find("|") != string::npos)
            {
                // Split multiple body parts using '|'
                pos = production.find("|");
                string body = trim(production.substr(pos + 1));
                if (body.empty())
                {
                    body = "epsilon";
                }

                // Split the body into symbols
                vector<string> bodySymbols = splitSymbols(body);

                // Insert into the map
                mp.insert({count, {head, bodySymbols}});
                count++;
            }
        }
    }

    void find_first(map<int, pair<string, vector<string>>> &productions,
                    vector<string> &nonTerminals,
                    vector<string> &terminals,
                    map<string, set<string>> &first)
    {

        // Initialize FIRST sets for terminals
        for (auto &x : terminals)
        {
            if (x != "epsilon")
            {
                first[x].insert(x); // FIRST(terminal) = {terminal}
            }
        }

        // Initialize FIRST sets for non-terminals
        for (auto &x : nonTerminals)
        {
            first[x] = {}; // Initialize as empty set
        }

        bool changed = true;
        while (changed)
        {
            changed = false;
            for (auto &x : productions)
            {
                string head = x.second.first;          // Production head
                vector<string> body = x.second.second; // Production body (vector of symbols)

                // If the body is "epsilon", add epsilon to FIRST(head)
                if (body.size() == 1 && body[0] == "epsilon")
                {
                    if (first[head].insert("epsilon").second)
                    {
                        changed = true;
                    }
                    continue;
                }

                // Iterate through the symbols in the body
                bool epsilon_in_all = true;
                for (const string &symbol : body)
                {
                    // If the symbol is a terminal or non-terminal
                    if (first.find(symbol) != first.end())
                    {
                        // Add FIRST(symbol) to FIRST(head), excluding epsilon
                        for (const string &y : first[symbol])
                        {
                            if (y != "epsilon")
                            {
                                if (first[head].insert(y).second)
                                {
                                    changed = true;
                                }
                            }
                        }

                        // If FIRST(symbol) does not contain epsilon, stop propagation
                        if (first[symbol].find("epsilon") == first[symbol].end())
                        {
                            epsilon_in_all = false;
                            break;
                        }
                    }
                    else
                    {
                        // Handle the case when symbol is not in the FIRST map
                        cerr << "Warning: Symbol '" << symbol << "' not found in FIRST sets.\n";
                        epsilon_in_all = false;
                        break;
                    }
                }

                // If all symbols in the body can derive epsilon, add epsilon to FIRST(head)
                if (epsilon_in_all)
                {
                    if (first[head].insert("epsilon").second)
                    {
                        changed = true;
                    }
                }
            }
        }
    }

    // Function to find FIRST of a string of symbols
    set<string> getFirstOfSymbols(const vector<string> &symbols, map<string, set<string>> &first)
    {
        set<string> result;
        bool canDeriveEpsilon = true;
        for (const auto &sym : symbols)
        {
            if (first.find(sym) == first.end())
            { // Terminal
                result.insert(sym);
                canDeriveEpsilon = false;
                break;
            }
            const auto &firstSet = first[sym];
            result.insert(firstSet.begin(), firstSet.end());
            result.erase("epsilon");
            if (firstSet.find("epsilon") == firstSet.end())
            {
                canDeriveEpsilon = false;
                break;
            }
        }
        if (canDeriveEpsilon)
        {
            result.insert("epsilon");
        }
        return result;
    }
    void find_follow(map<int, pair<string, vector<string>>> &productions,
                     vector<string> &nonTerminals,
                     vector<string> &terminals,
                     map<string, set<string>> &first,
                     map<string, set<string>> &follow)
    {

        // Initialize FOLLOW sets for all non-terminals
        for (const string &nt : nonTerminals)
        {
            follow[nt] = {};
        }

        // Start symbol gets `$` in FOLLOW set
        follow[nonTerminals[0]].insert("$");

        bool changed = true;
        while (changed)
        {
            changed = false;

            for (const auto &prod : productions)
            {
                string head = prod.second.first;          // Production head
                vector<string> body = prod.second.second; // Production body (vector of symbols)

                for (size_t i = 0; i < body.size(); i++)
                {
                    string B = body[i];

                    // If B is a non-terminal
                    if (find(nonTerminals.begin(), nonTerminals.end(), B) != nonTerminals.end())
                    {
                        // Case 1: If B is followed by another symbol, add FIRST(next) - epsilon to FOLLOW(B)
                        if (i + 1 < body.size())
                        {
                            vector<string> nextSymbols(body.begin() + i + 1, body.end());
                            set<string> firstOfNext = getFirstOfSymbols(nextSymbols, first);

                            for (const string &f : firstOfNext)
                            {
                                if (f != "epsilon" && follow[B].find(f) == follow[B].end())
                                {
                                    follow[B].insert(f);
                                    changed = true;
                                }
                            }

                            // If FIRST(next) contains epsilon, add FOLLOW(head) to FOLLOW(B)
                            if (firstOfNext.find("epsilon") != firstOfNext.end())
                            {
                                for (const string &f : follow[head])
                                {
                                    if (follow[B].find(f) == follow[B].end())
                                    {
                                        follow[B].insert(f);
                                        changed = true;
                                    }
                                }
                            }
                        }
                        // Case 2: If B is at the end of the production, add FOLLOW(head) to FOLLOW(B)
                        else
                        {
                            for (const string &f : follow[head])
                            {
                                if (follow[B].find(f) == follow[B].end())
                                {
                                    follow[B].insert(f);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // Function to print a single item

    void printItem(const Item &item, ofstream &outFile)
    {
        int prod_no = get<0>(item);
        string head = get<1>(item);
        vector<string> body = get<2>(item);
        int dot_pos = get<3>(item);
        string lookahead = get<4>(item);

        // Print production number and head
        outFile << prod_no << ". " << head << " -> ";

        // Print body with dot
        for (size_t i = 0; i < body.size(); i++)
        {
            if (i == dot_pos)
            {
                outFile << ".";
            }
            outFile << body[i] << " ";
        }

        // If dot is at the end
        if (dot_pos == body.size())
        {
            outFile << ".";
        }

        // Print lookahead
        outFile << ", " << lookahead << endl;
    }

    // Function to print all states
    void printStates(const map<int, vector<Item>> &states, ofstream &outFile)
    {

        for (const auto &[stateId, items] : states)
        {
            outFile << "State " << stateId << ":" << endl;
            for (const auto &item : items)
            {
                printItem(item, outFile);
            }
            outFile << "-----------------------------" << endl;
        }
        outFile.close();
    }
};