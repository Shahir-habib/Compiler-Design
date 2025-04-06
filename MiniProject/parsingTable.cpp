#include <bits/stdc++.h>
using namespace std;
using Item = tuple<int, string, vector<string>, int, string>;
class ParsingTable
{
public:
    string line;
    vector<string> nonTerminals, terminals;
    set<string> terminalSet;
    map<string, set<string>> first;
    map<string, set<string>> follow;
    map<int, vector<Item>> states;
    map<int, pair<string, vector<string>>> productions;

    ParsingTable(vector<string> &nonTerminals, vector<string> &terminals, set<string> &terminalSet,
                 map<string, set<string>> &first, map<string, set<string>> &follow,
                 map<int, vector<Item>> &states, map<int, pair<string, vector<string>>> &productions)
        : nonTerminals(nonTerminals), terminals(terminals), terminalSet(terminalSet),
          first(first), follow(follow), states(states), productions(productions)
    {
        // Initialize the ParsingTable object
    }

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
        ofstream outFile("parsingTable.txt");
        if (!outFile)
        {
            cout << "Error Opening the file!!!" << endl;
            return;
        }

        // Ensure $ is in terminals
        if (find(terminals.begin(), terminals.end(), "$") == terminals.end())
            terminals.push_back("$");

        // Filter visible symbols
        vector<string> visibleTerminals;
        for (const string &t : terminals)
            if (t != "epsilon")
                visibleTerminals.push_back(t);

        vector<string> visibleNonTerminals;
        for (const string &nt : nonTerminals)
            if (nt != productions[0].first)
                visibleNonTerminals.push_back(nt);

        // Column widths
        int colWidthTerminals = 12;
        int colWidthNonTerminals = 18;
        int stateWidth = 7;

        int actionWidth = (colWidthTerminals + 3) * visibleTerminals.size();
        int gotoWidth = (colWidthNonTerminals + 3) * visibleNonTerminals.size();
        int totalWidth = gotoWidth + actionWidth - 27;

        // Title
        string title = "LR(1) Parsing Table";
        int titlePos = (totalWidth - title.length()) / 2;
        outFile << string(titlePos, '-') << " " << title << " " << string(totalWidth - titlePos - title.length() - 2, '-') << "\n\n";

        // Top header line
        outFile << string(stateWidth + 1, '-') << "+";
        outFile << string(actionWidth - 25, '-') << "+";
        outFile << string(gotoWidth - 13, '-') << "+\n";

        // First header row: State | ACTION | GOTO
        outFile << setw(stateWidth) << "State" << " |";
        outFile << setw(actionWidth / 2 + 30) << "ACTION (terminals)";
        outFile << setw(gotoWidth / 2) << "|" << setw(gotoWidth / 2) << "GOTO (non-terminals)" << setw(gotoWidth / 2 - 11) << "|\n";

        outFile << string(stateWidth, '-') << "-+";
        for (size_t i = 0; i < visibleTerminals.size(); ++i)
            outFile << string(colWidthTerminals, '-') << "-+";
        for (size_t i = 0; i < visibleNonTerminals.size(); ++i)
            outFile << string(colWidthNonTerminals, '-') << "-+";
        outFile << "\n";
        // Second header row: terminals and non-terminals
        outFile << setw(stateWidth) << " " << " |";
        for (const string &term : visibleTerminals)
            outFile << setw(colWidthTerminals) << term << " |";
        for (const string &nt : visibleNonTerminals)
            outFile << setw(colWidthNonTerminals) << nt << " |";
        outFile << "\n";

        // Separator line
        outFile << string(stateWidth, '-') << "-+";
        for (size_t i = 0; i < visibleTerminals.size(); ++i)
            outFile << string(colWidthTerminals, '-') << "-+";
        for (size_t i = 0; i < visibleNonTerminals.size(); ++i)
            outFile << string(colWidthNonTerminals, '-') << "-+";
        outFile << "\n";

        // Table rows
        for (const auto &[stateId, items] : states)
        {
            outFile << setw(stateWidth) << stateId << " |";

            for (const string &term : visibleTerminals)
            {
                string action = parsingTable[{stateId, term}];
                outFile << setw(colWidthTerminals) << (action.empty() ? " " : action) << " |";
            }

            for (const string &nt : visibleNonTerminals)
            {
                string go = parsingTable[{stateId, nt}];
                outFile << setw(colWidthNonTerminals) << (go.empty() ? " " : go) << " |";
            }

            outFile << "\n";

            // Row separator
            outFile << string(stateWidth, '-') << "-+";
            for (size_t i = 0; i < visibleTerminals.size(); ++i)
                outFile << string(colWidthTerminals, '-') << "-+";
            for (size_t i = 0; i < visibleNonTerminals.size(); ++i)
                outFile << string(colWidthNonTerminals, '-') << "-+";
            outFile << "\n";
        }
        outFile.close();
    }
};