/**************************************************************************
** The Kalimat programming language
**   Copyright (C) 2010 Mohamed Samy Ali - samy2004@gmail.com
**   This project is released under the Apache License version 2.0
**   as described in the included license.txt file
**************************************************************************/

#ifndef KALIMATLEXER_H
#define KALIMATLEXER_H

const int FORALL = 1;
const int FROM = 2;
const int TO = 3;
const int WHILE = 4;
const int CONTINUE = 5;
const int FUNCTION = 6;
const int PROCEDURE = 7;
const int RETURN_WITH = 8;
const int IF = 9;
const int ELSE = 10;
const int END = 11;
const int DONE = 12;
const int STOP = 13;
const int C_TRUE = 14;
const int C_FALSE = 15;
const int AND = 16;
const int OR = 17;
const int NOT = 18;
const int ANDNOT = 19;
const int WHEN = 20;
const int EVENT = 21;
const int DO = 22;
const int NEW = 23;
const int TYPE = 24;
const int HAS = 25;
const int RESPONDS = 26;
const int REPLIES = 27;
const int BUILT = 28;
const int ON = 29;
const int RESPONSEOF = 30;
const int UPON = 31;
const int REPLYOF = 32;
const int CALL_PREV_RESPONSE = 33;
const int READ = 34;
const int PRINT = 35;
const int ZOOM = 36;
const int DRAW_PIXEL = 37;
const int DRAW_LINE = 38;
const int DRAW_RECT = 39;
const int DRAW_CIRCLE = 40;
const int DRAW_SPRITE = 41;
const int IN = 42;
const int CLASS = 43;
const int MOUSE_BUTTON_DOWN = 44;
const int MOUSE_BUTTON_UP = 45;
const int MOUSE_MOVE = 46;
const int KEY_DOWN = 47;
const int KEY_UP = 48;
const int KEY_PRESS = 49;
const int COLLISION = 50;
const int GLOBAL = 51;
const int FIELD = 52;
const int USING = 53;
const int UNIT = 54;
const int USING_WIDTH = 55;
const int NOTHING = 56;
const int LABEL = 57;
const int GO = 58;
const int DELEGATE = 59;
const int IS = 60;
const int LAUNCH = 61;
const int SEND = 62;
const int RECEIVE = 63;
const int SELECT = 64;
const int SIGNAL_ = 65;
const int KEYWORD_CUTOFF = 65; // As long as the ID of the token is <= KEYWORD_CUTOFF, the
                               // syntax hilighter will consider it a keyword.
const int NUM_LITERAL = 71;
const int ADD_OP = 72;
const int SUB_OP = 73;
const int MUL_OP = 74;
const int DIV_OP = 75;
const int EQ = 76;
const int LT = 77;
const int GT = 78;
const int LE = 79;
const int GE = 80;
const int NE = 81;


const int LPAREN = 82;
const int RPAREN = 83;
const int IDENTIFIER = 84;
const int COMMENT = 85;
const int NEWLINE = 86;
const int COMMA = 87;
const int SEMI = 88;
const int STR_LITERAL = 89;
const int COLON = 90;
const int KASRA = 91;
const int DOLLAR = 92;
const int LBRACKET = 93;
const int RBRACKET = 94;
const int HASH = 95;
const int ELLIPSIS = 96;
const int LBRACE = 97;
const int RBRACE = 98;
const int ROCKET = 99;

const int WHITESPACE = TokenNone;

QString TokenNameFromId(int id);

class KalimatLexer : public Lexer
{
    CharPredicate *digit, *arabicDigit, *europeanDigit;
    CharPredicate *arabDecimalSeparator, *europeanDecimalSeparator;
    CharPredicate *letter;
    CharPredicate *idSymbol;

    CharPredicate *plus, *minus;
    CharPredicate *mul, *div;

    CharPredicate *lparen, *rparen, *lbracket, *rbracket, *lbrace, *rbrace;
    CharPredicate *dollarSign;
    Predicate *rocket;

    CharPredicate *spacer, *tab, *space;
    Predicate *lineComment;

    CharPredicate *addOp, *subOp, *mulOp, *divOp, *eq, *lt, *gt;

    Predicate *le,  *ge, *ne;
    Predicate *returnWith;

    CharPredicate *comma, *semi, *colon, *kasra, *hash;
    Predicate *ellipsis;
    CharPredicate *quote;
    Predicate *two_quotes;

    CharPredicate *anyChar, *nl, *noneNl;
    Predicate *sol, *eof;

    Action *retract;

    QMap<QString, int> keywords;
    QMap<QString, QString> arab_chars;

    void InitCharPredicates();
    void InitKeywords();
    void InitArabChars();
    Token accept(TokenType);
public:
    KalimatLexer();
    static QMap<int, QString> tokenNameMap;
};

class ColonUnsupportedInIdentifiersException
{

};

#endif // KALIMATLEXER_H