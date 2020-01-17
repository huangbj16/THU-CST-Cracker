all: parser.y parser.l
	yacc parser.y -d -o y.tab.cpp
	flex parser.l
	mv lex.yy.c lex.yy.cpp

clean:
	rm *.o lex.yy.cpp y.tab.cpp y.tab.hpp
