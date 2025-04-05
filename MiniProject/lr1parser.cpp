#include <bits/stdc++.h>
using namespace std;

template <typename K, typename V>
void removeDuplicates(map<K, V> &inputMap)
{
    set<V> seenValues;
    for (auto it = inputMap.begin(); it != inputMap.end();)
    {
        if (seenValues.find(it->second) != seenValues.end())
        {
            it = inputMap.erase(it);
        }
        else
        {
            // Add value to the set of seen values
            seenValues.insert(it->second);
            ++it;
        }
    }
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
// Helper function to trim whitespace from a string
string trim(const string &str)
{
    size_t first = str.find_first_not_of(' ');
    if (first == string::npos)
        return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, last - first + 1);
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

using Item = tuple<int, string, vector<string>, int, string>;

// Helper function to compute FIRST of a sequence of symbols
set<string> getFirstOfSequence(const vector<string> &sequence, const map<string, set<string>> &first)
{
    set<string> result;
    bool canDeriveEpsilon = true;

    for (const string &symbol : sequence)
    {
        if (first.find(symbol) == first.end())
        { // Terminal
            result.insert(symbol);
            canDeriveEpsilon = false;
            break;
        }
        const set<string> &firstSet = first.at(symbol);
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

vector<Item> closure(vector<Item> items, map<int, pair<string, vector<string>>> &productions,
                     map<string, set<string>> &first, vector<string> &nonTerminals)
{
    bool changed;
    do
    {
        changed = false;
        vector<Item> newItems = items;
        for (const auto &item : items)
        {
            int prod_no = get<0>(item);
            string head = get<1>(item);
            vector<string> body = get<2>(item);
            int dot_pos = get<3>(item);
            string lookahead = get<4>(item);

            if (dot_pos >= body.size())
                continue;

            string next_symbol = body[dot_pos];
            if (find(nonTerminals.begin(), nonTerminals.end(), next_symbol) != nonTerminals.end())
            {
                vector<string> remaining(body.begin() + dot_pos + 1, body.end());
                set<string> firstSet = getFirstOfSequence(remaining, first);
                if (firstSet.erase("epsilon"))
                {
                    firstSet.insert(lookahead);
                }
                if (firstSet.empty())
                {
                    firstSet.insert(lookahead);
                }

                for (const auto &prod : productions)
                {
                    if (prod.second.first == next_symbol)
                    {
                        for (const string &la : firstSet)
                        {
                            Item newItem = make_tuple(prod.first,
                                                      prod.second.first,
                                                      prod.second.second,
                                                      0,
                                                      la);
                            if (find(newItems.begin(), newItems.end(), newItem) == newItems.end())
                            {
                                newItems.push_back(newItem);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
        items = newItems;
    } while (changed);
    return items;
}

vector<Item> goTo(const vector<Item> &items, const string &symbol,
                  map<int, pair<string, vector<string>>> &productions,
                  map<string, set<string>> &first, vector<string> &nonTerminals)
{
    vector<Item> nextItems;
    for (const auto &item : items)
    {
        int prod_no = get<0>(item);
        string head = get<1>(item);
        vector<string> body = get<2>(item);
        int dot_pos = get<3>(item);
        string lookahead = get<4>(item);

        if (dot_pos < body.size() && body[dot_pos] == symbol)
        {
            Item newItem = make_tuple(prod_no, head, body, dot_pos + 1, lookahead);
            nextItems.push_back(newItem);
        }
    }
    return closure(nextItems, productions, first, nonTerminals);
}

void lr1ItemSet(map<int, pair<string, vector<string>>> &productions,
                vector<string> &nonTerminals, vector<string> &terminals,
                map<string, set<string>> &first,
                map<int, vector<Item>> &states)
{

    // Ensure augmented start production exists
    if (productions.find(0) == productions.end())
    {
        cerr << "Error: Augmented start production (S' -> S) not found!\n";

        return;
    }

    // Initialize with augmented start item
    vector<Item> items;
    items.emplace_back(0, productions[0].first, productions[0].second, 0, "$"); // [S' -> .S, $]

    // Compute initial closure
    items = closure(items, productions, first, nonTerminals);

    states[0] = items;
    int stateCount = 1;
    bool changed = true;

    while (changed)
    {
        changed = false;
        map<int, vector<Item>> newStates = states;

        for (const auto &[stateId, stateItems] : states)
        {
            // Collect all possible transition symbols
            set<string> symbols;
            for (const auto &item : stateItems)
            {
                const vector<string> &body = get<2>(item);
                int dot_pos = get<3>(item);
                if (dot_pos < body.size())
                {
                    symbols.insert(body[dot_pos]);
                }
            }

            // Process each symbol
            for (const string &sym : symbols)
            {
                vector<Item> newState = goTo(stateItems, sym, productions, first, nonTerminals);

                if (!newState.empty())
                {
                    // Check if state already exists
                    bool stateExists = false;
                    for (const auto &[id, existingState] : newStates)
                    {
                        if (existingState == newState)
                        {
                            stateExists = true;
                            break;
                        }
                    }

                    if (!stateExists)
                    {
                        newStates[stateCount] = newState;
                        stateCount++;
                        changed = true;
                    }
                }
            }
        }
        states = newStates;
    }
}

void constructlr1parsingTable(const map<int, vector<Item>> &states,
                              map<int, pair<string, vector<string>>> &productions,
                              vector<string> &nonTerminals, vector<string> &terminals,
                              set<string> &terminalSet,
                              map<string, set<string>> &first,
                              map<string, set<string>> &follow,
                              map<pair<int, string>, string> &parsingTable)
{
    // Fill parsing table
    for (const auto &[stateId, items] : states)
    {
        for (const auto &item : items)
        {
            int prod_no = get<0>(item);
            string head = get<1>(item);
            vector<string> body = get<2>(item);
            int dot_pos = get<3>(item);
            string lookahead = get<4>(item);

            // If dot is at the end, add reduce actions
            if (dot_pos == body.size())
            {
                if (head == productions[0].first && lookahead == "$")
                {
                    parsingTable[{stateId, "$"}] = "Accept";
                }
                else
                {
                    // Add reduce action for all lookaheads in FOLLOW[head] for lr(0)
                    // for (const string &la : follow[head])
                    // {
                    //     parsingTable[{stateId, la}] = "r" + to_string(prod_no);
                    // }//for lr1
                    parsingTable[{stateId, lookahead}] = "r" + to_string(prod_no);
                }
            }
            else
            {
                string nextSymbol = body[dot_pos];
                // Find the next state via GOTO
                vector<Item> gotoItems = goTo(items, nextSymbol, productions, first, nonTerminals);
                if (!gotoItems.empty())
                {
                    // Find the state ID for the GOTO result
                    for (const auto &[nextStateId, nextState] : states)
                    {
                        if (nextState == gotoItems)
                        {
                            if (find(nonTerminals.begin(), nonTerminals.end(), nextSymbol) != nonTerminals.end())
                            {
                                // GOTO action for non-terminals
                                parsingTable[{stateId, nextSymbol}] = to_string(nextStateId);
                            }
                            else
                            {
                                // SHIFT action for terminals
                                parsingTable[{stateId, nextSymbol}] = "s" + to_string(nextStateId);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    ofstream outFile("parsingtable.txt");

    if (!outFile)
    {
        cerr << "Error opening file for writing!" << endl;
        return;
    }
    int totalsize = terminals.size() + nonTerminals.size() - 1; // Adjusted for uniform column width
    outFile << "\n_______LR(1) Parsing Table_______\n";         // Print header row
    outFile << setw(5) << "State" << " |";
    outFile << setw(15 * terminals.size()) << "Action" << " |";
    outFile << setw(15 * nonTerminals.size()) << "Goto" << " |\n"; // Adjusted for consistency

    outFile << string(25 * (totalsize), '-') << endl; // Adjusted separator length

    vector<string> allTerms = terminals;
    // Ensure "$" is included in terminals
    if (find(allTerms.begin(), allTerms.end(), "$") == allTerms.end())
    {
        allTerms.push_back("$");
    }

    // Print terminal headers (skip "epsilon" if exists)
    outFile << "       "; // Adjusted for uniform column width
    for (const string &term : allTerms)
    {
        if (term != "epsilon")
        {
            outFile << setw(10) << term << " |"; // Adjusted for uniform column width
        }
    }

    // Print non-terminal headers (excluding augmented start symbol)
    for (const string &nt : nonTerminals)
    {
        if (nt != productions[0].first)
        {
            outFile << setw(10) << nt << " |"; // Adjusted for uniform column width
        }
    }

    outFile << "\n"
            << string(25 * (totalsize), '-') << endl; // Adjusted separator length to 40

    // Populate table rows
    for (const auto &[stateId, items] : states)
    {
        outFile << setw(5) << stateId << " |"; // Adjusted for uniform column width

        // Print actions for terminals (including "$")
        for (const string &term : allTerms)
        {
            if (term == "epsilon")
                continue;
            string action = parsingTable[{stateId, term}];
            if (!action.empty())
            {
                outFile << setw(10) << action << " |"; // Adjusted for uniform column width
            }
            else
            {
                outFile << setw(10) << " " << " |"; // Adjusted for uniform column width
            }
        }

        // Print GOTO actions for non-terminals
        for (const string &nt : nonTerminals)
        {
            if (nt != productions[0].first)
            {
                string gotoAction = parsingTable[{stateId, nt}];
                outFile << setw(10) << gotoAction << " |"; // Adjusted for uniform column width
            }
        }

        outFile << "\n"
                << string(25 * (totalsize), '-') << endl; // Adjusted separator length to 40
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

void parseInput(vector<string> &tkns,
                const map<pair<int, string>, string> &parsingTable,
                const map<int, pair<string, vector<string>>> &productions)
{
    stack<string> stck;
    stck.push("0");
    int tknindex = 0;
    tkns.push_back("$");
    string lookahead = tkns[tknindex];

    cout << "Parsing Input........" << endl;
    // Print the header for the table
    // cout << "| Stack\t\t| Input\t\t\t| Action\t|" << endl;
    // cout << "|---------------|----------------------|---------------|" << endl;
    while (true)
    {
        
        cout << "Lookahead : " << lookahead << endl;
        // Print the current stack
        stack<string> temp = stck; // Create a copy of the stack for printing
        string stack_str = "";
        while (!temp.empty())
        {
            stack_str = temp.top() + " " + stack_str;
            temp.pop();
        }

        // Print the current input
        // cout << "| " << stack_str << "\t\t| ";
        // for (int i = tknindex; i < tkns.size(); i++)
        // {
        //     cout << tkns[i] << " ";
        // }

        // Prepare to print the action
        cout << "\t\t| ";

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
            cout << "Action : " << parsingTable.at({topst, lookahead}) << endl;
            cout << "Shift  : " << lookahead << endl;
            stck.push(lookahead);
            stck.push(parsingTable.at({topst, lookahead}).substr(1));
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
            cout << "Action : " << parsingTable.at({topst, lookahead}) << endl;
            cout << "Reduce by " << head << " -> ";
            for (const auto &symbol : body)
            {
                cout << symbol << " ";
            }
            cout << endl;

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
            cout << "Accepted 👌👌👌👌" << endl;
            break;
        }
        // Completed just epsilon left
        else
        {
            if (lookahead == "$" )
            {
                cout << "Accepted 👌👌👌👌" << endl;
                break;
            }
            cout << "Error: Unexpected token '" << lookahead << "' at state " << topst << endl;
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
    readterminals_and_nonterminals(nonTerminals, terminals, terminalSet, file);
    map<int, pair<string, vector<string>>> productions;
    map<pair<int, string>, string> parsingTable;
    vector<string> inputTokens;
    string token;
    // while (stringFile >> token)
    // {
    //     // if (token == "epsilon")
    //     //     continue;
    //     inputTokens.push_back(token);
    // }
    int c=0;
    while (getline(stringFile, line))
    {
        stringstream ss(line);
        string tokenType, value;
        int lineNumber, scopeLevel;

        ss >> tokenType >> value >> lineNumber >> scopeLevel;
        if (c > 2)
            inputTokens.push_back(value);
        c++;
    }
    inputTokens.pop_back();
    ofstream outFile("lr1itemsets.txt");

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
    read_productions(productions, file);
    for (auto x : productions)
    {
        outFile << x.first << " " << x.second.first << " -> ";
        for (const auto &symbol : x.second.second)
        {
            outFile << symbol << " ";
        }
        outFile << "\n";
    }
    find_first(productions, nonTerminals, terminals, first);
    find_follow(productions, nonTerminals, terminals, first, follow);

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
    lr1ItemSet(productions, nonTerminals, terminals, first, states);
    removeDuplicates(states);
    outFile << "\n_______LR(1) Item Sets_______\n";

    printStates(states, outFile);

    cout << "First and Follow Sets have been written to 'lr1itemsets.txt'." << endl;
    cout << "LR(1) Item Sets have been written to 'lr1itemsets.txt'." << endl;
    constructlr1parsingTable(states, productions, nonTerminals, terminals, terminalSet, first, follow, parsingTable);
    cout << "LR(1) Parsing Table has been written to 'parsingtable.txt'." << endl;
    parseInput(inputTokens, parsingTable, productions);
    cout << endl;
    outFile.close();
    stringFile.close();
    return 0;
}
