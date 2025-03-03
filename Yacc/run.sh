win_flex --header-file=lex.yy.h q1.l
win_bison -d q1.y
gcc lex.yy.c q1.tab.c 