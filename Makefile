# Gabriel Pinto de Camargo - 9293456
# Gabriel Simmel Nascimento - 9050232
# Marcos Cesar Ribeiro de Camargo - 9278045
# Victor Luiz Roquete Forbes - 9293394

all:
	@gcc -o main src/*.c -I./includes

debug:
	@gcc -o main src/*.c -I./includes -g -Wall

clean:
	@rm bin/*.bin bin/*.idx bin/*.header main -f

run: all
	@./main

test1: clean all
	@./main < teste1.in

test2: clean all
	@./main < teste2.in

fullrun: debug
	@valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all ./main

fulltest1: clean debug
	@valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all ./main < teste1.in

fulltest2: clean debug
	@valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all ./main < teste2.in

zip: clean
	@zip -r Grupo_6.zip *
