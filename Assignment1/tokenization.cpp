// Shahir Habib Roll no. : 002210501006
#include <bits/stdc++.h>
using namespace std;

// Token types (you can expand this based on more categories)
enum class TokenType
{
    Keyword,
    Identifier,
    Operator,
    Integer,
    Float,
    String,
    Delimiter,
    Comment,
    Unknown
};

// Struct to store a token and its type
struct Token
{
    string value;
    TokenType type;
};

TokenType getTokenType(const string &token)
{
    static const regex keyword_regex(R"(\b(int|float|char|return|if|else|while|for|void|struct|long|double)\b)");
    static const regex identifier_regex(R"([a-zA-Z_][a-zA-Z0-9_]*)");
    static const regex operator_regex(R"([+\-*/=<>!&|%^]+)");
    static const regex integer_regex(R"(\b\d+\b)");
    static const regex float_regex(R"(\b\d+\.\d+\b)");
    static const regex string_regex(R"(".*?")");
    static const regex delimiter_regex(R"([;{}(),\[\]])");
    static const regex comment_regex(R"(//.*|/\*.*?\*/)");

    if (regex_match(token, keyword_regex))
    {
        return TokenType::Keyword;
    }
    if (regex_match(token, identifier_regex))
    {
        return TokenType::Identifier;
    }
    if (regex_match(token, operator_regex))
    {
        return TokenType::Operator;
    }
    if (regex_match(token, integer_regex))
    {
        return TokenType::Integer;
    }
    if (regex_match(token, float_regex))
    {
        return TokenType::Float;
    }
    if (regex_match(token, string_regex))
    {
        return TokenType::String;
    }
    if (regex_match(token, delimiter_regex))
    {
        return TokenType::Delimiter;
    }
    if (regex_match(token, comment_regex))
    {
        return TokenType::Comment;
    }

    return TokenType::Unknown;
}

vector<Token> tokenize(const string &program)
{
    vector<Token> tokens;
    regex token_regex("[a-zA-Z_][a-zA-Z0-9_]*|\\d+\\.\\d+|\\d+|\".*?\"|//.*|/\\*.*?\\*/|[+\\-*/=<>!&|%^]+|;|\\{|\\}|\\(|\\)|,|\\[|\\]");
    auto words_begin = sregex_iterator(program.begin(), program.end(), token_regex);
    auto words_end = sregex_iterator();

    for (sregex_iterator i = words_begin; i != words_end; ++i)
    {
        string match = (*i).str();
        TokenType type = getTokenType(match);
        tokens.push_back({match, type});
    }

    return tokens;
}

// Function to display tokens
void displayTokens(const vector<Token> &tokens)
{
    for (const auto &token : tokens)
    {
        string type;
        switch (token.type)
        {
        case TokenType::Keyword:
            type = "Keyword";
            break;
        case TokenType::Identifier:
            type = "Identifier";
            break;
        case TokenType::Operator:
            type = "Operator";
            break;
        case TokenType::Integer:
            type = "Integer";
            break;
        case TokenType::Float:
            type = "Float";
            break;
        case TokenType::String:
            type = "String";
            break;
        case TokenType::Delimiter:
            type = "Delimiter";
            break;
        case TokenType::Comment:
            type = "Comment";
            break;
        case TokenType::Unknown:
            type = "Unknown";
            break;
        }
        cout << type << ":          " << token.value << endl;
        cout << "--------------------------------------" << endl;
    }
}

int main()
{
    FILE *fp = fopen("input.c", "r");
    if (fp == NULL)
    {
        cout << "Error in opening file" << endl;
        return 0;
    }
    string c_program;
    char ch;
    while ((ch = fgetc(fp)) != EOF)
    {
        c_program += ch;
    }
    fclose(fp);
    

    vector<Token> tokens = tokenize(c_program);
    displayTokens(tokens);

    return 0;
}
