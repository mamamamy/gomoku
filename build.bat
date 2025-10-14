
:: make data folder
set "folder_name=./data"
if not exist "%folder_name%" (
  md "%folder_name%"
)

:: make generator
gcc -Wall -Wextra -O3 generator.c save.c vct.c board.c bitmap256.c wtree.c -o generator -lpthread

:: make test
gcc -Wall -Wextra -O3 test.c vct.c board.c bitmap256.c -o test
