# Triângulo de Sierpinski
## Compilando no linux
basta ter instalado o gcc, o make e as bibliotecas glfw e glew e executar o comando

    make

## Compilando no windows
Utilizar o [MSYS 2](https://www.msys2.org/) e instalar as bibliotecas e o compilador

    pacman -S mingw-w64-x86_64-glew mingw-w64-x86_64-glfw mingw-w64-x86_64-gcc mingw-w64-x86_64-make
E compilar

    mingw32-make
