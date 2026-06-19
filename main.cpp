#include "stb_image.h"
#include "gl_utils.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h> 
#define GL_LOG_FILE "gl.log"
#include <iostream>
#include <vector>
#include <string>
#include "TileMap.h"
#include "DiamondView.h"
#include "SlideView.h"
#include "ltMath.h"
#include <fstream>
#include "gl_utils.cpp"
#include "maths_funcs.cpp"

using namespace std;

//Configurações Globais
int g_gl_width = 800, g_gl_height = 800;
float xi = -1.0f, xf = 1.0f, yi = -1.0f, yf = 1.0f;
float w = xf - xi, h = yf - yi;
float tw, th, tw2, th2, tileW, tileW2, tileH, tileH2;
int tileSetCols = 9, tileSetRows = 9;

struct Item {
    int x, y;
    bool coletado; //Verdadeiro se o carro pegou a gasolina, falso se ainda nao
};
vector<Item> gasolinas; //Lista para guardar os galoes
GLuint textura_gasolina; //Variavel da imagem do galao de gasolina

//Carro (Jogador) e Meta (Ponto de Objetivo)
int cx = 7, cy = 7; //Ponto de spawn
int direcao_carro = 0; 
int goal_x = -1, goal_y = -1;
bool vitoria = false;
GLuint texturas_carro[8]; //As 8 dirções do carro

TilemapView *tview = new DiamondView();
TileMap *tmap = NULL;
GLFWwindow *g_window = NULL;
vector<int> tiles_bloqueados;

bool eh_bloqueado(int id) {
    for (int i = 0; i < tiles_bloqueados.size(); i++) {
        if (tiles_bloqueados[i] == id) return true;
    }
    return false;
}

//Função da leitura do mapa
TileMap * readMapNovo(const char *filename, string &out_nomeImagem, int &out_qtdTiles, int &out_largura, int &out_altura) {
    ifstream arq(filename);
    if (!arq.is_open()) {
        cout << "Erro: Nao foi possivel abrir o arquivo " << filename << endl;
        return NULL;
    }

    arq >> out_nomeImagem >> out_qtdTiles >> out_largura >> out_altura;

    //Le a quantidade de tiles bloqueados e depois armazena os IDs
    int qtdBloqueados;
    arq >> qtdBloqueados;
    tiles_bloqueados.clear();
    for (int i = 0; i < qtdBloqueados; i++) {
        int idBloqueado;
        arq >> idBloqueado;
        tiles_bloqueados.push_back(idBloqueado);
    }

    //Le o tamanho do mapa e preenche a matriz
    int linhas, colunas;
    arq >> linhas >> colunas;

    TileMap *temp = new TileMap(colunas, linhas, 0);

    for(int r = 0; r < linhas; r++) {
        for(int c = 0; c < colunas; c++) {
            int tid; 
            arq >> tid;
            temp->setTile(c, linhas - r - 1, tid);
        }
    }
   
    arq >> cx >> cy;           //Posição inicial do carro
    arq >> goal_x >> goal_y;   //Posição do objetivo

    //Le quantos galoes existem e guarda a posição de cada um
    int qtdGasolina;
    if (arq >> qtdGasolina) {
        gasolinas.clear();
        for (int i = 0; i < qtdGasolina; i++) {
            Item g;
            arq >> g.x >> g.y;
            g.coletado = false;
            gasolinas.push_back(g);
        }
    }

    arq.close();
    return temp;
}

int loadTexture(unsigned int &texture, const char *filename) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        cout << "Aviso: Falha ao carregar a textura " << filename << endl;
    }
    stbi_image_free(data);
    return 1; 
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        int n_cx = cx, n_cy = cy, n_dir = direcao_carro;

        //Direções retas (WASD/Setinha)
        if (key == GLFW_KEY_UP || key == GLFW_KEY_W) { n_cy++; n_dir = 3; } //Norte
        else if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) { n_cy--; n_dir = 1; } //Sul
        else if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A) { n_cx--; n_dir = 2; } //Oeste
        else if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) { n_cx++; n_dir = 0; } //Leste
        
        //Diagonais (Q, E, Z, C)
        else if (key == GLFW_KEY_Q) { n_cx--; n_cy++; n_dir = 5; } //(NW)
        else if (key == GLFW_KEY_E) { n_cx++; n_cy++; n_dir = 4; } //(NE)
        else if (key == GLFW_KEY_Z) { n_cx--; n_cy--; n_dir = 6; } //(SW)
        else if (key == GLFW_KEY_C) { n_cx++; n_cy--; n_dir = 7; } //(SE)

        //Logica de movimentação
        if (tmap != NULL && n_cx >= 0 && n_cx < tmap->getWidth() && n_cy >= 0 && n_cy < tmap->getHeight()) {
            if (!eh_bloqueado((int)tmap->getTile(n_cx, n_cy))) {
                //Pinta o chao em terra (ID 4) antes do carro sair dele
                tmap->setTile(cx, cy, 4);
                cx = n_cx; cy = n_cy;
                //Passa por todos os galoes para ver se pisamos em algum
                for (int i = 0; i < gasolinas.size(); i++) {
                    if (!gasolinas[i].coletado && gasolinas[i].x == cx && gasolinas[i].y == cy) {
                        gasolinas[i].coletado = true;
                        cout << "Pegou um galao de gasolina!" << endl;
                    }
                }
                //Se chegou no objetivo é aplicada a vitoria
                if (cx == goal_x && cy == goal_y && !vitoria) {
                    bool pegouTodas = true;
                    //Verifica se deixou de pegar algum galao
                    for (int i = 0; i < gasolinas.size(); i++) {
                        if (!gasolinas[i].coletado) pegouTodas = false;
                    }

                    if (pegouTodas) {
                        vitoria = true;
                        cout << "Missao cumprida! FIM DE JOGO." << endl;
                    } else {
                        cout << "Voce precisa coletar todas as gasolinas primeiro!" << endl;
                    }
                }
                //Mecanismo da lava e (RESET TOTAL)
                if ((int)tmap->getTile(cx, cy) == 1) { //Le o ID da lava (1)
                    cout << "\n=====================================" << endl;
                    cout << "Voce pisou na lava e explodiu!" << endl;
                    cout << "Resetando o jogo..." << endl;
                    cout << "=====================================\n" << endl;
                    
                    //Volta o carro para o spawn inicial
                    cx = 2; 
                    cy = 2;
                    
                    //Reseta o status de vitoria
                    vitoria = false;
                    
                    //Renasce todos os galões de gasolina no mapa
                    for (int i = 0; i < gasolinas.size(); i++) {
                        gasolinas[i].coletado = false;
                    }

                    //Limpa as marcas de pneu (Muda os IDs 4 de volta para 0)
                    for (int r = 0; r < tmap->getHeight(); r++) {
                        for (int c = 0; c < tmap->getWidth(); c++) {
                            if ((int)tmap->getTile(c, r) == 4) { 
                                tmap->setTile(c, r, 0); 
                            }
                        }
                    }
                }
            }
        }
        direcao_carro = n_dir;
    }
}

int main() {
    srand(time(NULL)); 
    restart_gl_log(); 
    start_gl();
    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LESS);

    //Le o arquivo txt
    string nomeImagem;
    int qtdTiles, larguraTileTxt, alturaTileTxt;

    tmap = readMapNovo("map.txt", nomeImagem, qtdTiles, larguraTileTxt, alturaTileTxt);

    if (tmap == NULL) {
        cout << "Falha ao carregar o mapa. Encerrando." << endl;
        glfwTerminate();
        return -1;
    }

    tw = w / (float)tmap->getWidth(); 
    th = tw / 2.0f;
    tw2 = th; 
    th2 = th / 2.0f;

    tileSetCols = 9; 
    tileSetRows = 9; 

    tileW = 1.0f / (float)tileSetCols; 
    tileW2 = tileW / 2.0f;
    tileH = 1.0f / (float)tileSetRows; 
    tileH2 = tileH / 2.0f;

    glfwSetKeyCallback(g_window, key_callback);
    
    //Carregamento da textura lida
    GLuint tid; 
    loadTexture(tid, nomeImagem.c_str()); 
    tmap->setTid(tid);

    //Vertices do mapa isometrico
    float vertices[] = {
        xi    , yi+th2, 0.0f, tileH2,
        xi+tw2, yi    , tileW2, 0.0f,
        xi+tw , yi+th2, tileW, tileH2,
        xi+tw2, yi+th , tileW2, tileH,
    };
    unsigned int indices[] = { 0, 1, 3, 3, 1, 2 };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO); glGenBuffers(1, &EBO);
    glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Carrega as 8 direções do carro (0=E, 1=S, 2=W, 3=N, 4=NE, 5=NW, 6=SW, 7=SE)
        loadTexture(texturas_carro[0], "police_E.png");
        loadTexture(texturas_carro[1], "police_S.png");
        loadTexture(texturas_carro[2], "police_W.png");
        loadTexture(texturas_carro[3], "police_N.png");
        loadTexture(texturas_carro[4], "police_NE.png");
        loadTexture(texturas_carro[5], "police_NW.png");
        loadTexture(texturas_carro[6], "police_SW.png");
        loadTexture(texturas_carro[7], "police_SE.png");
        loadTexture(textura_gasolina, "gasolina.png"); //Carrega a textura da gasolina

    float car_v[] = { xi, yi, 0, 1, xi+tw, yi, 1, 1, xi+tw, yi+tw, 1, 0, xi, yi+tw, 0, 0 };
    unsigned int cVAO, cVBO, cEBO;
    glGenVertexArrays(1, &cVAO); glGenBuffers(1, &cVBO); glGenBuffers(1, &cEBO);
    glBindVertexArray(cVAO); glBindBuffer(GL_ARRAY_BUFFER, cVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(car_v), car_v, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    char vs_s[1024*256], fs_s[1024*256];
    parse_file_into_str("_geral_vs.glsl", vs_s, 1024*256); parse_file_into_str("_geral_fs.glsl", fs_s, 1024*256);
    GLuint vs = glCreateShader(GL_VERTEX_SHADER); const GLchar* p_v = vs_s; glShaderSource(vs, 1, &p_v, NULL); glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER); const GLchar* p_f = fs_s; glShaderSource(fs, 1, &p_f, NULL); glCompileShader(fs);
    GLuint shader = glCreateProgram(); glAttachShader(shader, vs); glAttachShader(shader, fs); glLinkProgram(shader);

    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    while (!glfwWindowShouldClose(g_window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader);

        //Renderiza o chão
        glBindVertexArray(VAO);
        for(int r = 0; r < tmap->getHeight(); r++) {
            for(int c = 0; c < tmap->getWidth(); c++) {
                int t_id = (int)tmap->getTile(c, r);
                glUniform1f(glGetUniformLocation(shader, "offsetx"), (t_id % tileSetCols) * tileW);
                glUniform1f(glGetUniformLocation(shader, "offsety"), (t_id / tileSetCols) * tileH);
                float x, y; tview->computeDrawPosition(c, r, tw, th, x, y);
                glUniform1f(glGetUniformLocation(shader, "tx"), x);
                glUniform1f(glGetUniformLocation(shader, "ty"), y + 1.0f);
                glUniform1f(glGetUniformLocation(shader, "layer_z"), tmap->getZ());
                
                float weight = 0.0f;
                float hr=0, hg=0, hb=1; 

                if (c == cx && r == cy) {
                    weight = 0.5f; hr=0; hg=0; hb=1; 
                } else if (c == goal_x && r == goal_y) {
                    weight = 0.6f; hr=0; hg=1; hb=0; 
                }
                
                glUniform1f(glGetUniformLocation(shader, "weight"), weight);
                glUniform4f(glGetUniformLocation(shader, "highlight_color"), hr, hg, hb, 1.0f);
                
                glBindTexture(GL_TEXTURE_2D, tmap->getTileSet());
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        //Renderiza as Gasolinas com "efeito flutuante"
        glBindVertexArray(cVAO);
        glBindTexture(GL_TEXTURE_2D, textura_gasolina);
        
        //Configura a "onda" e a velocidade da gasolina
        float tempo = glfwGetTime();
        float altura_flutuacao = sin(tempo * 1.5f) * 0.01f; 

        for (int i = 0; i < gasolinas.size(); i++) {
            if (!gasolinas[i].coletado) {
                float gx, gy; 
                tview->computeDrawPosition(gasolinas[i].x, gasolinas[i].y, tw, th, gx, gy);
                glUniform1f(glGetUniformLocation(shader, "offsetx"), 0); 
                glUniform1f(glGetUniformLocation(shader, "offsety"), 0);
                glUniform1f(glGetUniformLocation(shader, "tx"), gx); 
                
                glUniform1f(glGetUniformLocation(shader, "ty"), gy + 1.0f + altura_flutuacao);
                glUniform1f(glGetUniformLocation(shader, "layer_z"), tmap->getZ() - 0.05f);
                glUniform1f(glGetUniformLocation(shader, "weight"), 0.0f); 
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        //Renderiza o carro
        glBindVertexArray(cVAO);
        glBindTexture(GL_TEXTURE_2D, texturas_carro[direcao_carro]); 
        float car_x, car_y; tview->computeDrawPosition(cx, cy, tw, th, car_x, car_y);
        glUniform1f(glGetUniformLocation(shader, "offsetx"), 0); glUniform1f(glGetUniformLocation(shader, "offsety"), 0);
        glUniform1f(glGetUniformLocation(shader, "tx"), car_x); glUniform1f(glGetUniformLocation(shader, "ty"), car_y + 1.0f);
        glUniform1f(glGetUniformLocation(shader, "layer_z"), tmap->getZ() - 0.1f);
        glUniform1f(glGetUniformLocation(shader, "weight"), 0.0f); 
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwPollEvents(); glfwSwapBuffers(g_window);
    }
    glfwTerminate(); return 0;
}