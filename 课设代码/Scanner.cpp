#include "Scanner.h"
#include <iostream>
#include <cassert>


void Scanner::openFile( const string & filename) {
    inputFile.open(filename);
    if(inputFile.is_open())
        return;
    else
        cout<<"文件不存在"<<endl;
}

void Scanner::closeFile() {
    inputFile.close();
}

bool Scanner::isKeywords(string word) {
    if (keywords.find(word)!=keywords.end())
        return true;
    else
        return false;
}


Scanner::Scanner() {

}

char Scanner::nextChar() {
    if (bufferPos >= lineBuffer.size()) {  //改换行了
        row++;
        getline(inputFile,lineBuffer);
        lineBuffer += '\n';
        if (!inputFile.fail()) {
            bufferPos = 0;
            return lineBuffer[bufferPos++];
        } else
            return EOF;
    } else
        return lineBuffer[bufferPos++];
}

void Scanner::rollBack() {
    assert(bufferPos > 0);
    bufferPos--;
}

Scanner::Token Scanner::nextToken() {
    Token token;
    State state = START_STATE;

//    char ch = nextChar();
//    string str = "";
//    str += '(';
//    cout<<isSymbols(str);

    while (state != FINISH_STATE) {
        char ch = nextChar();  //获取下一个字符
        if (ch == EOF) {
            token.tokenType = ENDOFFILE;
            break;
        }
        string str_ch ="";
        str_ch += ch;
        switch (state) {
            case START_STATE:  //开始状态
                if (!isFilter(ch)) {  //过滤空格符
                    if (isalpha(ch) || ch == '_') {   //首字符为字母或者下划线  进入标识符状态
                        state = ID_STATE;
                        token.tokenType = IDENTIFIER;
                        token.lexeme += ch;
                        token.row = row;
                    } else if (isdigit(ch)) {
                        state = INT_STATE;  //进入整数状态
                        token.tokenType = INT;
                        token.lexeme += ch;
                        token.row = row;
                    } else if (isSymbols(str_ch)) {  //进入界符状态
                        state = SYMBOL_STATE;
                        token.tokenType = SYMBOL;
                        token.lexeme += ch;
                        token.row = row;
                    } else if (ch == '"') {  // 字符串状态
                        state = STRING_STATE;
                        token.tokenType = STRING;
                        token.lexeme += ch;
                        token.row = row;
                    } else if (ch == '\'') {  //进入单字符状态
                        state = CHAR_STATE;
                        token.tokenType = CHAR;
                        token.lexeme += ch;
                        token.row = row;
                    } else {  //进入错误状态  // 其它非法字符
                        state = ERROR_STATE;
                        token.tokenType = ERROR;
                        token.lexeme += ch;
                        token.row = row;
                    }
                }
                break;
            case INT_STATE :  //整数状态
                if (isdigit(ch)) {
                    token.lexeme +=ch;
                } else if (ch == '.'){
                    token.lexeme += '.';
                    token.tokenType = FLOAT;
                    state = FLOAT_STATE;
                } else {
                    rollBack();
                    state = FINISH_STATE;
                }
                break;
            case FLOAT_STATE:
                if(isdigit(ch)){
                    token.lexeme += ch;
                } else {
                    rollBack();
                    state = FINISH_STATE;
                }
                break;
            case ID_STATE : //标识符状态
                if (isalpha(ch) || isdigit(ch) || ch == '_') {
                    token.lexeme += ch;
                } else {
                    rollBack();
                    state = FINISH_STATE;
                }
                break;
            case STRING_STATE : //字符串状态
                if (ch == '"') {
                    token.lexeme += '"';
                    state = FINISH_STATE;
                } else if (ch == '\\'){
                    state = S_STRING_STATE;
                    token.lexeme +=ch;
                } else {
                    token.lexeme += ch;
                }
                break;
            case S_STRING_STATE:
                state = STRING_STATE;
                token.lexeme.pop_back();  // 去掉一个 /
                token.lexeme += ch;
                break;
            case CHAR_STATE:  //字符状态
                if (ch != '\\'&&ch != '\'') {
                    state = CHAR_STATE_A;
                    token.lexeme += ch;
                } else if (ch == '\\') {
                    state = CHAR_STATE_B;
                    token.lexeme += ch;
                    token.lexeme.pop_back();  //去掉一个 /
                } else if (ch == '\'') {
                    state = ERROR_STATE;
                    token.tokenType = ERROR;
                    token.lexeme += ch;
                }
                break;
            case SYMBOL_STATE:
                if (token.lexeme == "/") {
                    if (ch == '*') {  // 注释开头
                        state = INCOMMENT_STATE;
                        token.lexeme.pop_back();
                    } else if (ch == '/') {  // 单行注释 这行直接不分析
                        state = START_STATE;
                        bufferPos = lineBuffer.length();
                        token.lexeme.pop_back();  // 弹出 /
                    } else {
                        rollBack();
                        state = FINISH_STATE;
                    }
                } else if (token.lexeme == "<") {
                    if (ch == '=') {  // <=
                        token.lexeme += ch;
                        state = FINISH_STATE;
                    } else {
                        rollBack();
                        state = FINISH_STATE;
                    }
                } else if (token.lexeme == "=") {
                    if (ch == '=') {  //  ==
                        token.lexeme += ch;
                        state = FINISH_STATE;
                    } else {
                        rollBack();
                        state = FINISH_STATE;
                    }
                } else if (token.lexeme == "!") {
                    if (ch == '=') {  // !=
                        token.lexeme += ch;
                    } else {
                        rollBack();
                        state = FINISH_STATE;
                    }
                } else if (token.lexeme == "&") {
                    if (ch == '&') {  // &&
                        token.lexeme += ch;
                        state = FINISH_STATE;
                    } else {
                        rollBack();
                        state = FINISH_STATE;
                    }
                } else if (token.lexeme == "|") {
                    if (ch == '|') {  // ||
                        token.lexeme += ch;
                        state = FINISH_STATE;
                    } else {
                        rollBack();
                        state = FINISH_STATE;
                    }
                } else {
                    rollBack();
                    state = FINISH_STATE;
                }
                break;
            case CHAR_STATE_A:
                if (ch == '\'') {
                    state = FINISH_STATE;
                    token.lexeme += "'";
                } else {
                    state = ERROR_STATE;
                    token.tokenType = ERROR;
                    string tmp = "'";
                    tmp.append(token.lexeme);
                    tmp += ch;
                    token.lexeme = tmp;
                }
                break;
            case CHAR_STATE_B :
                if (ch == 'a' || ch == 'b' || ch == 'f' || ch == 'n' || ch == 'r' ||
                    ch == 't' || ch == 'v' || ch == '\\' || ch == '?' || ch == '\'' ||
                    ch == '"') {
                    state =CHAR_STATE_C;
                    token.lexeme += ch;
                } else {
                    state = ERROR_STATE;
                    token.tokenType = ERROR;
                    string tmp = "'\\";
                    tmp += ch;
                    token.lexeme = tmp;
                }
                break;
            case CHAR_STATE_C :
                if (ch == '\'') {
                    state = FINISH_STATE;
                    token.lexeme += ch;
                } else {
                    state = ERROR_STATE;
                    token.tokenType = ERROR;
                    string tmp = "'";
                    tmp.append(token.lexeme);
                    tmp += ch;
                    token.lexeme = tmp;
                }
                break;
            case ERROR_STATE:  //错误状态
                if (ch == ' ' || ch == '\n' || ch == '\t')
                    state = FINISH_STATE;
                else
                    token.lexeme += ch;
                break;
            case INCOMMENT_STATE :  //注释状态
                if (ch == '*')
                    state = P_INCOMMENT_STATE;
                break;
            case P_INCOMMENT_STATE:  //接近退出注释状态
                if (ch == '/') {
                    state = START_STATE;
                } else {
                    state = INCOMMENT_STATE;
                }
                break;
        }
        if (state == FINISH_STATE && token.tokenType ==IDENTIFIER){
            if (!isKeywords(token.lexeme))
                token.tokenType = IDENTIFIER;
            else
                token.tokenType = KEYWORD;
        }
    }
    return token;
}

bool Scanner::isFilter(char ch) {
    if (filter.find(ch) != filter.end())
        return true;
    else
        return false;
}

bool Scanner::isSymbols( const string & ch) {
    if(symbols.find(ch) != symbols.end())
        return true;
    else
        return false;
}


























