#include <bits/stdc++.h>
using namespace std;

// Set of valid symbols
set<char> validSymbols = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',

    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',

    '+', '-', '*', '/', '%',
    '<', '>', '=',
    ';', ',', '(', ')', '{', '}',
    ' ', '\t', '\n',
    '_', '.'};

// Token types
enum class TokenType {
    ID,     // Identifiers (variable names)
    NUM,    // Numbers (integers)
    DEC,    // Decimal numbers (floats)
    INT,    // "int" keyword
    VOID,   // "void" keyword
    FLOAT,  // "float" keyword
    EQ,     // "=" operator
    LPAREN, // "(" symbol
    RPAREN, // ")" symbol
    LBRACE, // "{" symbol
    RBRACE, // "}" symbol
    SEMI,   // ";" symbol
    COMMA,  // "," symbol
    IF,     // "if" keyword
    ELSE,   // "else" keyword
    READ,   // "read" keyword
    PRINT,  // "print" keyword
    EQEQ,   // "==" operator
    LT,     // "<" operator
    GT,     // ">" operator
    PLUS,   // "+" operator
    MINUS,  // "-" operator
    MULT,   // "*" operator
    DIV,    // "/" operator
    MOD,    // "%" operator
    INC,    // "++" operator
};

// Map TokenType to its name
unordered_map<TokenType, string> tokenTypeNames = {
    {TokenType::ID, "ID"},
    {TokenType::NUM, "NUM"},
    {TokenType::DEC, "DEC"},
    {TokenType::INT, "INT"},
    {TokenType::VOID, "VOID"},
    {TokenType::FLOAT, "FLOAT"},
    {TokenType::EQ, "EQ"},
    {TokenType::LPAREN, "LPAREN"},
    {TokenType::RPAREN, "RPAREN"},
    {TokenType::LBRACE, "LBRACE"},
    {TokenType::RBRACE, "RBRACE"},
    {TokenType::SEMI, "SEMI"},
    {TokenType::COMMA, "COMMA"},
    {TokenType::IF, "IF"},
    {TokenType::ELSE, "ELSE"},
    {TokenType::READ, "READ"},
    {TokenType::PRINT, "PRINT"},
    {TokenType::EQEQ, "EQEQ"},
    {TokenType::LT, "LT"},
    {TokenType::GT, "GT"},
    {TokenType::PLUS, "PLUS"},
    {TokenType::MINUS, "MINUS"},
    {TokenType::MULT, "MULT"},
    {TokenType::DIV, "DIV"},
    {TokenType::MOD, "MOD"},
    {TokenType::INC, "INC"},
};

// List of tokens: <TokenType, value, line number, scope level>
vector<tuple<TokenType, string, int, int>> tokens;

// Tokenize the input code using regex
void tokenize(const string &code) {
    // Define regex patterns for each token type
    vector<pair<TokenType, regex>> tokenPatterns = {
        {TokenType::INT, regex("int")},
        {TokenType::VOID, regex("void")},
        {TokenType::FLOAT, regex("float")},
        {TokenType::IF, regex("if")},
        {TokenType::ELSE, regex("else")},
        {TokenType::READ, regex("read")},
        {TokenType::PRINT, regex("print")},
        {TokenType::EQEQ, regex("==")},
        {TokenType::LT, regex("<")},
        {TokenType::GT, regex(">")},
        {TokenType::PLUS, regex("\\+")},
        {TokenType::MINUS, regex("-")},
        {TokenType::MULT, regex("\\*")},
        {TokenType::DIV, regex("/")},
        {TokenType::MOD, regex("%")},
        {TokenType::INC, regex("\\+\\+")},
        {TokenType::EQ, regex("=")},
        {TokenType::LPAREN, regex("\\(")},
        {TokenType::RPAREN, regex("\\)")},
        {TokenType::LBRACE, regex("\\{")},
        {TokenType::RBRACE, regex("\\}")},
        {TokenType::SEMI, regex(";")},
        {TokenType::COMMA, regex(",")},
        {TokenType::ID, regex("[a-zA-Z_][a-zA-Z0-9_]*")},
        {TokenType::DEC, regex("\\d+\\.\\d*")},
        {TokenType::NUM, regex("\\d+")}        
    };

    int lineNo = 1;       // Track line number
    int scopeNo = 0;      // Track scope level
    string remainingCode = code;

    // Iterate through the code and match tokens
    while (!remainingCode.empty()) {
        bool matched = false;

        // Try to match each token pattern
        for (const auto &pattern : tokenPatterns) {
            smatch match;
            if (regex_search(remainingCode, match, pattern.second, regex_constants::match_continuous)) {
                string tokenValue = match.str();
                TokenType tokenType = pattern.first;

                // Update scope level based on '{' and '}'
                if (tokenType == TokenType::LBRACE) {
                    scopeNo++;
                } else if (tokenType == TokenType::RBRACE) {
                    scopeNo--;
                }

                // Add the token to the list
                tokens.push_back({tokenType, tokenValue, lineNo, scopeNo});

                // Remove the matched token from the remaining code
                remainingCode = match.suffix().str();

                // Update line number for newlines
                size_t newlineCount = count(tokenValue.begin(), tokenValue.end(), '\n');
                lineNo += newlineCount;

                matched = true;
                break;
            }
        }

        // If no token matched, check for invalid characters
        if (!matched) {
            char currentChar = remainingCode[0];
            if (!isspace(currentChar)) {
                cout << "Lexical Error: Invalid character '" << currentChar << "' at line " << lineNo << endl;
                return;
            }

            // Skip whitespace and update line number
            if (currentChar == '\n') {
                lineNo++;
            }
            remainingCode.erase(0, 1);
        }
    }
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }
    ifstream file(argv[1]);
    if (!file.is_open()) {
        cout << "Cannot open file " << argv[1] << endl;
        return 1;
    }

    string code((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    tokenize(code);
    cout << "+-----------+----------------+-----------+----------------+" << endl;
    cout << "| Token     | Value          | Line      | Scope          |" << endl;
    cout << "+-----------+----------------+-----------+----------------+" << endl;
    for (const auto &token : tokens) {
        cout << "| " << left << setw(9) << tokenTypeNames[get<0>(token)] << " | "
                  << setw(14) << get<1>(token) << " | "
                  << setw(9) << to_string(get<2>(token)) << " | "
                  << setw(14) << get<3>(token) << " |" << endl;
    }
    cout << "+-----------+----------------+-----------+----------------+" << endl;
    return 0;
}