#include <cstdio>
#include <cctype>

#include <string>
#include <memory>
#include <vector>
#include <map>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-------------------------------- LEXER ---------------------------------------
//The lexer returns tokens [0 - 255](ASCII value) if it is an unknown character,
//otherwise one of these for known things
enum Token{
    //end of file
    tok_eof = -1,

    //commands
    tok_def = -2,
    tok_extern = -3,

    //primary
    tok_identifier = -4,
    tok_number = -5,
};

static std::string identiferStr;        //Filled in if tok_indertifier
static double numVal;                   //Filled in if tok_number

//Return the next token from standard input
static int getTok(){
    static int lastChar = ' ';

    //Skip any whitespace
    while(isspace(lastChar))
        lastChar = getchar();

    //Recognize indentifiers and specific keywords
    if(isalpha(lastChar)){     //identifer: [a-z A-Z][a-z A-Z 0-9]
        identiferStr = lastChar;

        while(isalnum(lastChar = getchar()))
            identiferStr += lastChar;

        if(identiferStr == "def")
            return tok_def;
        if(identiferStr == "extern")
            return tok_extern;
        return tok_identifier;
    }

    //Recognize numeric values
    //---------------------------------------------------------------
    //Needs error check for invalid numeric input like 1.23.45.67
    if(isdigit(lastChar) || lastChar == '.'){      //Number: [0-9.]+ 
        std::string numStr;
        do{
            numStr += lastChar;
            lastChar = getchar();
        }while(isdigit(lastChar) || lastChar == '.');

        numVal = strtod(numStr.c_str(), 0);
        return tok_number;
    }

    //Recognize comments
    if(lastChar == '#'){
        //comment until end of line
        do
            lastChar = getchar();
        while(lastChar != EOF && lastChar != '\n' && lastChar != 'r');
        
        if(lastChar != EOF)
            return getTok();
    }

    //Check for end of file
    if(lastChar == EOF)
        return tok_eof;
    
    int thisChar = lastChar;
    lastChar = getchar();
    return thisChar;
};
//------------------------------END OF LEXER -----------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------





//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-------------------- THE ABSTRACT SYNTAX TREE (AST) ---------------------------
namespace{
//ExprAST - Base class for all expression nodes
class ExprAST{
public:
    virtual ~ExprAST() {}
};

//NumeberExprATS - Expression class for numeric literals like "1.0".
class NumberExprATS : public ExprAST{
public:
    NumberExprATS(double val) : val(val) {}

private:
    double val;
};

//VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST{
public:
    VariableExprAST(const std::string &name) : name(name) {}

private:
    std::string name;
};

//BinaryExprAST - Expression class for a binary operator, like "+".
class BinaryExprASR : public ExprAST{
public:
    BinaryExprASR(char op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
        : op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

private:
    char op;
    std::unique_ptr<ExprAST> LHS, RHS;
};

//CallExprATS - Expression class for function calls
class CallExprATS : public ExprAST{
public:
    CallExprATS(const std::string &callee, std::vector<std::unique_ptr<ExprAST>> args)
        : callee(callee), args(std::move(args)) {}
private:
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;
};

//PrototypeAST - This class reperesents the "protptype" for a function,
//which captures its name, and its argument names (thus implicity the number of arguments the function takes)
class PrototypeAST{
public:
    PrototypeAST(const std::string &name, std::vector<std::string> args)
        : name(name), args(std::move(args)) {}
    
    const std::string getName() const { return name; }

private:
    std::string name;
    std::vector<std::string> args;
};

//FunctionAST - This class represents a function definition itself
class FunctionAST{
public:
    FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
        : proto(std::move(proto)), body(std::move(body)) {}

private:
    std::unique_ptr<PrototypeAST> proto;
    std::unique_ptr<ExprAST> body;
};
}

//----------------- END OF THE ABSTRACT SYNTAX TREE (AST) ----------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------




//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//---------------------------------- PARSER ------------------------------------

//---------------------------------PARSER BASICS--------------------------------
//curTok/getNextToken - Provide a simple token buffer.
//curTok is the current token the parser is looking at.
//getNextToken reads another token from the lexer and updates curTok with its results.
static int curTok;
static int getNextToken(){
    return curTok = getTok();
}

//logError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> logError(const char *str){
    fprintf(stderr, "LogError: %s\n", str);

    return nullptr;
}
std::unique_ptr<PrototypeAST> logErrorP(const char *str){
    logError(str);

    return nullptr;
}

//-------------------------- BASIC EXPRESSION ----------------------------

//numberexpr ::= number
static std::unique_ptr<ExprAST> parseNumberExpr(){
    auto result = std::make_unique<NumberExprATS>(numVal);
    getNextToken();                                            //comsume the number
    return std::move(result);
}

//parenexpr ::= '(' expresion ')'
static std::unique_ptr<ExprAST> parseParenExpr(){
    getNextToken();

    auto v = parseExpression();
    if(!v)
        return nullptr;
    
    if(curTok != ')')
        return logError("expected ')'");
    getNextToken();

    return v;
}

//identifierexpr
//  ::= identifier
//  ::= identifier '(' expression* ')'
static std::unique_ptr<ExprAST> ParseIdentifierExpr(){
    std::string idName = identiferStr;

    getNextToken();

    if(curTok != '(')                                       //simple variable ref.
        return std::make_unique<VariableExprAST> idName;
    
    //call
    getNextToken();
    std::vector<std::unique_ptr<ExprAST>> args;
    if(curTok != ')'){
        while(1){
            if(auto arg = paraseExpression)
                args.push_back(arg);
            else
                return nullptr;
            
            if(curTok == ')')
                break;
            
            if(curTok != ',')
                return logError("expected ')' or ',' in argument list");
            
            getNextToken;
        }
    }

    getNextToken();

    return std::make_unique<CallExprATS>(idName, std::move(args));
}

//primary
//  ::= identifierexpr
//  ::= numberexpr
//  ::= parenexpr
static std::unique_ptr<ExprAST> parsePrimary(){
    switch(curTok){
    default:
        return logError("unknow token when expecting an expression");
    case tok_identifier:
        return ParseIdentifierExpr();
    case tok_number:
        return parseNumberExpr();
    case '(':
        return parseParenExpr();
    }
}

//------------------------ BINARY EXPRESSION ---------------------------

//binoPrecedence - This holds the precedence for each binary operator that os defined
static std::map<char, int> binoPrecedence;

//getTokPrecedence - Get the precedence of the pending binary operator token
static int getTokPrecedence(){
    if(!isascii(curTok))
        return -1;

    //make sure it is a declared binop
    int tokPrec = binoPrecedence[curTok];
    if (tokPrec <= 0)
    {
        return -1;
    }
    
    return tokPrec;
}

//a sequence of [binop, primaryexpr] pairs
//expression
//  ::= primary binoprhs
static std::unique_ptr<ExprAST> parseExpression(){
    auto LHS = parsePrimary();
    if(!LHS)
        return nullptr;
    
    return parseBinOpRHS(0, std::move(LHS));
}

int main(void){
    //Install standard binary operator
    //1 is the lowest precedence
    //only support 4 binary operators right now
    binoPrecedence['<'] = 10;
    binoPrecedence['+'] = 20;
    binoPrecedence['-'] = 30;
    binoPrecedence['*'] = 40;           //highest


    return 0;
}