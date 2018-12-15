CFILES = $(shell find ./ -name "*.c")

parser : syntax.tab.c main.c lex.yy.c datastruct.c symbol.c intermediate.c
	gcc syntax.tab.c main.c datastruct.c symbol.c intermediate.c -lfl -ly -o parser
	rm syntax.tab.c syntax.tab.h lex.yy.c

syntax.tab.c : syntax.y
	bison -d -v syntax.y

lex.yy.c : lexical.l
	flex lexical.l

# 定义的一些伪目标
.PHONY: clean
clean:
	rm -f parser lex.yy.c syntax.tab.c syntax.tab.h syntax.output test.ir
	rm -f *~