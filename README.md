# Jogo Isométrico Grau B

👥 Integrantes do Grupo:
Arthur Almeida Steinbach

🚀 Como executar:
Este programa foi desenvolvido na linguagem C++ utilizando as bibliotecas gráficas OpenGL e GLFW. Pode ser compilado diretamente em uma IDE como o VS Code.

Passo a passo no VS Code:
Deve ter o compilador C++ (ex: MinGW/GCC) instalado e as bibliotecas essenciais (GLAD e GLFW) configuradas na sua pasta.

Com o projeto aberto, pressione Ctrl + Shift + B para executar a tarefa de compilação.

Após a compilação bem-sucedida, abra o terminal do próprio VS Code e execute o programa gerado, digitando ./main.exe.

💡 Sobre o Programa:
O projeto consiste em um protótipo de jogo com Tilemap isométrico onde o jogador controla uma viatura policial.

Mecânicas Principais:
Controle em 8 direções: O jogador movimenta o veículo pelos tiles caminháveis utilizando as teclas clássicas (W, A, S, D para retas e Q, E, Z, C para diagonais).

Leitura Dinâmica: Todo o layout do labirinto, coordenadas de spawn, localização de obstáculos (água e lava) e itens coletáveis são gerados através da leitura em tempo real do arquivo map.txt.

Interação de Cenário: O veículo deixa um rastro de terra/pneu ao passar pelos blocos de grama, mostrando a funcionalidade de alteração de tiles em tempo de execução.

Coleta e Punição: O objetivo do jogo é coletar todos os galões de gasolina espalhados pelo mapa e alcançar o bloco de saída. O jogador deve evitar os blocos de lava, pois se pisar neles causa a explosão do carro, resetando a posição do jogador, apagando os rastros deixados no chão e recolocando as gasolinas no mapa.

📌 Observações:
O código-fonte principal encontra-se no arquivo main.cpp.

Não é necessário recompilar o código para criar novos cenários ou alterar a quantidade de gasolinas; basta editar as matrizes e valores dentro do arquivo map.txt.

O projeto utiliza a biblioteca stb_image.h para o carregamento transparente das texturas .png.

🙌 Créditos:
Texturas da viatura policial disponibilizada gratuitamente no Kenney.nl.

📚 Referências:
Documentação Oficial do GLFW: glfw.org

Repositório stb_image: nothings/stb

Tutoriais de C++ e OpenGL: LearnOpenGL (referência para o setup de Shaders e VAOs/VBOs).

Além do material do modulo 5 :D.
