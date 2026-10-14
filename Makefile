SRCS=src/assembler.cpp src/lexer.cpp src/generator.cpp src/parser.cpp
OBJS=build/assembler.o build/lexer.o build/generator.o build/parser.o

main.out: $(OBJS)
	g++ \
	$(OBJS) \
	-o main.out

build/%.o: src/%.cpp
	g++ -I./include -c $< -o $@
