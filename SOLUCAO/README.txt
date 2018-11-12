#ESTRUTURA DAS DIRETORIAS#
SOLUCAO
| doTest.sh //script que obtem o tempo de execucao do programa
| Makefile //Makefile geral (inclui o CircuitRouter-ParSolver e o CircuitRouter-SeqSolver)
| README.txt
|
|---CircuitRouter-ParSolver //codigo do programa
|   | CircuitRouter-ParSolver.c
|   | coordinate.c
|   | coordinate.h
|   | grid.c
|   | grid.h
|	  | Makefile //Makefile do CircuitRouter-ParSolver
|   | maze.c
|   | maze.h
|   | router.c
|   | router.h
|---CircuitRouter-SeqSolver //codigo do Ex1 (usado para calcular o speed up)
|   | CircuitRouter-SeqSolver.c
|   | coordinate.c
|   | coordinate.h
|   | grid.c
|   | grid.h
|	  | Makefile //Makefile do CircuitRouter-SeqSolver
|   | maze.c
|   | maze.h
|   | router.c
|   | router.h
|---lib //biblioteca auxiliar ao programa
|   | commandlinereader.c
|   | commandlinereader.
|   | list.c
|   | list.h
|   | pair.c
|   | pair.o
|   | queue.c
|   | queue.h
|   | timer.h
|   | types.h
|   | utility.h
|   | vector.c
|   | vector.h
----results //resultados da execucao de doTest.sh para cada ficheiro
    | random-x32-y32-z3-n64.txt.speedups.csv
    | random-x32-y32-z3-n96.txt.speedups.csv
    | random-x48-y48-z3-n48.txt.speedups.csv
    | random-x48-y48-z3-n64.txt.speedups.csv
    | random-x64-y64-z3-n48.txt.speedups.csv
    | random-x64-y64-z3-n64.txt.speedups.csv
    | random-x128-y128-z3-n64.txt.speedups.csv
    | random-x128-y128-z3-n128.txt.speedups.csv
    | random-x128-y128-z5-n128.txt.speedups.csv
    | random-x256-y256-z3-n256.txt.speedups.csv
    | random-x256-y256-z5-n256.txt.speedups.csv

#COMPILACAO E EXECUCAO#
Para compilar o codigo todo usar:
make
Para compilar apenas o CircuitRouter-ParSolver ou o CircuitRouter-SeqSolver usar respetivamente:
make -C CircuitRouter-ParSolver
make -C CircuitRouter-SeqSolver

Para eliminar os ficheiros criados pela compilacao usar:
make clean

Para executar o programa usar:
./CircuitRouter-ParSolver/CircuitRouter-ParSolver -t <numero de threads a usar> <ficheiro de entrada>

Para executar o programa e medir o speedup usar:
./doTest.sh <numero de threads a usar> <ficheiro de entrada>

#CARACTERISTICAS DO PROCESSADOR USADO PARA OS TESTES#
Foi usado um computador com hyper-threading
Numero de cores: 4
Numero de threads: 8
Clock Rate: 4 GHz
Modelo: AMD FX(tm)-8350 Eight-Core Processor

#CARACTERISTICAS DO SISTEMA OPERATIVO USADO PARA OS TESTES#
Linux DESKTOP-GBVNN15 4.4.0-17134-Microsoft #345-Microsoft Wed Sep 19 17:47:00 PST 2018 x86_64 x86_64 x86_64 GNU/Linux
