#include <cstdio>
#include <cctype>
#include <string>


//-------------------------------- LEXER ----------------------------------------
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

int main(void){
    return 0;
}