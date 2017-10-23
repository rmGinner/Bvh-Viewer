// **********************************************************************
//	BVHViewer.c
//  Desenha e anima um esqueleto a partir de um arquivo BVH (BioVision)
//  Marcelo Cohen
//  marcelo.cohen@pucrs.br
// **********************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>    // somente no Windows
#include "gl/glut.h"
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

typedef struct Node Node;

struct Node {
    char name[20];       // nome
    float offset[3];     // offset (deslocamento)
    int channels;        // qtd de canais (3 ou 6)
    float* channelData;  // vetor com os dados dos canais
    int numChildren;     // qtd de filhos
    Node** children;     // vetor de ponteiros para os filhos
    Node* parent;        // ponteiro para o pai
};

// Raiz da hierarquia
Node* root;

// Total de frames
int totalFrames;

// Frame atual
int curFrame = 0;

// Funcoes para liberacao de memoria da hierarquia
void freeTree();
void freeNode(Node* node);

// Variaveis globais para manipulacao da visualizacao 3D
int width,height;
float deltax=0, deltay=0;
GLfloat angle=60, fAspect=1.0;
GLfloat rotX=0, rotY=0, rotX_ini=0, rotY_ini=0;
GLfloat ratio;
GLfloat angY, angX;
int x_ini=0,y_ini=0,bot=0;
float Obs[3] = {0,0,-80};
float Alvo[3];
float ObsIni[3];

// **********************************************************************
//  Cria um nodo novo para a hierarquia, fazendo também a ligacao com
//  o seu pai (se houver)
//  Parametros:
//  - name: string com o nome do nodo
//  - parent: ponteiro para o nodo pai (NULL se for a raiz)
//  - numChannels: quantidade de canais de transformacao (3 ou 6)
//  - ofx, ofy, ofz: offset (deslocamento) lido do arquivo
//  - numChildren: quantidade de filhos que serao inseridos posteriormente
// **********************************************************************
Node* createNode(char name[20], Node* parent, int numChannels, float ofx, float ofy, float ofz, int numChildren)
{
    Node* aux = malloc(sizeof(Node));
    aux->channels = numChannels;
    aux->channelData = calloc(sizeof(float), numChannels);
    strcpy(aux->name, name);
    aux->offset[0] = ofx;
    aux->offset[1] = ofy;
    aux->offset[2] = ofz;
    aux->numChildren = numChildren;
    if(numChildren > 0)
        aux->children = calloc(sizeof(Node*), numChildren);
    else
        aux->children = NULL;
    aux->parent = parent;
    if(parent)
        for(int i=0; i<parent->numChildren; i++)
            if(!parent->children[i]) {
//                printf("Insert at parent: %d\n", i);
                parent->children[i] = aux;
                break;
            }
    printf("Created %s\n", name);
    return aux;
}

//
// DADOS DE EXEMPLO DO PRIMEIRO FRAME
//

float data[] = { -326.552, 98.7701, 317.634, 71.4085, 60.8487, 17.2406, -70.1915, 0, 88.8779, 84.6529, 68.0632,
               -5.27801, 0.719492, 15.2067, 13.3733, -135.039, 24.774, 172.053, -171.896, 64.9682, -165.105,
               3.6548, 1.03593, -36.4128, -55.7886, 37.8019, -120.338, 9.39682, 14.0503, -27.1815, 4.41274,
               -0.125185, -1.52942, 1.33299, -4.20935, 46.1022, -92.5385, -35.676, 63.2656, -5.23096, -15.2195,
               9.30354, 11.1114, -0.982512, -11.0421, -86.4319, -3.01435, 76.3394, 1.71268, 24.9011, -2.42099,
               9.483, 17.5267, -1.42749, -37.0021, -44.3019, -39.1702, -46.2538, -2.58689, 78.4703, 1.9216, 29.8211,
               -1.99744, -3.70506, 1.06523, 0.577189, 0.146783, 3.70013, 2.9702 };

// Pos. da aplicacao dos dados
int dataPos;

void applyData(float data[], Node* n)
{
    //printf("%s:\n", n->name);
    if(n->numChildren == 0)
        return;
    for(int c=0; c<n->channels; c++) {
        //printf("   %d -> %f\n", c, data[dataPos]);
        n->channelData[c] = data[dataPos++];
    }
    for(int i=0; i<n->numChildren; i++)
        if(n->children[i])
            applyData(data, n->children[i]);
}

void apply()
{
    dataPos = 0;
    applyData(data, root);
}

void initMaleSkel()
{
    root = createNode("Hips", NULL, 6, 0, 0, 0, 3);

    Node* toSpine = createNode("ToSpine", root, 3, -2.69724, 7.43032, -0.144315, 1);
    Node* spine = createNode("Spine", toSpine, 3, -0.0310711, 10.7595, 1.96963, 1);
    Node* spine1 = createNode("Spine1", spine, 3, 19.9056, 3.91189, 0.764692, 3);

    Node* neck = createNode("Neck", spine1, 3, 25.9749, 7.03908, -0.130764, 1);
    Node* head = createNode("Head", neck, 3, 9.52751, 0.295786, -0.907742, 1);
    Node* top = createNode("Top", head, 3, 16.4037, 0.713936, 2.7358, 0);

    /**/
    Node* leftShoulder = createNode("LeftShoulder", spine1, 3, 17.7449, 4.33886, 11.7777, 1);
    Node* leftArm = createNode("LeftArm", leftShoulder, 3, 0.911315, 1.27913, 9.80584, 1);
    Node* leftForeArm = createNode("LeftForeArm", leftArm, 3, 28.61265, 1.18197, -3.53199, 1);
    Node* leftHand = createNode("LeftHand", leftForeArm, 3, 27.5088, 0.0218783, 0.327423, 1);
    Node* endLeftHand = createNode("EndLHand", leftHand, 3, 18.6038, -0.000155887, 0.382096, 0);

    /**/
    Node* rShoulder = createNode("RShoulder", spine1, 3, 17.1009, 2.89543, -12.2328, 1);
    Node* rArm = createNode("RArm", rShoulder, 3, 1.4228, 0.178766, -10.211, 1);
    Node* rForeArm = createNode("RForeArm", rArm, 3, 28.733, 1.87905, 2.64907, 1);
    Node* rHand = createNode("RHand", rForeArm, 3, 27.4588, 0.290562, -0.101845, 1);
    Node* endRHand = createNode("RLHand", rHand, 3, 17.8396, -0.255518, -0.000602873, 0);

    Node* lUpLeg = createNode("LUpLeg", root, 3, -5.61296, -2.22332, -10.2353, 1);
    Node* lLeg = createNode("LLeg", lUpLeg, 3, 2.56703, -44.7417, -7.93097, 1);
    Node* lFoot = createNode("LFoot", lLeg, 3, 3.16933, -46.5642, -3.96578, 1);
    Node* lToe = createNode("LToe", lFoot, 3, 0.346054, -6.02161, 12.8035, 1);
    Node* lToe2 = createNode("LToe2", lToe, 3, 0.134235, -1.35082, 5.13018, 0);

    Node* rUpLeg = createNode("RUpLeg", root, 3, -5.7928, -1.72406, 10.6446, 1);
    Node* rLeg = createNode("RLeg", rUpLeg, 3, -2.57161, -44.7178, -7.85259, 1);
    Node* rFoot = createNode("RFoot", rLeg, 3, -3.10148, -46.5936, -4.03391, 1);
    Node* rToe = createNode("RToe", rFoot, 3, -0.0828122, -6.13587, 12.8035, 1);
    Node* rToe2 = createNode("RToe2", rToe, 3, -0.131328, -1.35082, 5.13018, 0);

    apply();
}

// Sums two vectors, result in c
void vsum (float a[3], float b[3], float c[3])
{
    c[0] = a[0] + b[0];
    c[1] = a[1] + b[1];
    c[2] = a[2] + b[2];
}

//  Draw line segment from point 'aaa' to point 'bbb', colored 'col'.
void drawLine (float col[3], float aaa[3], float bbb[3])
{
    glColor3fv(col);
    glBegin(GL_LINES);
       glVertex3fv(aaa);
       glVertex3fv(bbb);
    glEnd();
}

float orange[] = { 1, 0.5, 0 };
float yellow[] = { 1, 1, 0 };
float red[] = { 1, 0, 0 };
float white[] = { 1, 1, 1 };

// Desenha um nodo da hierarquia (chamada recursiva)
void drawNode(float pox, float poy, float poz, Node* node)
{
    float parentPos[3] = { pox, poy, poz };
    float nodePos[3];
    vsum (parentPos, node->offset, nodePos);  // right knee offset

    int c = 0;
    glPushMatrix ();

       glTranslatef ( parentPos[0], parentPos[1], parentPos[2]);
       if(node->channels == 6) {
           glTranslatef(node->channelData[0], node->channelData[1], node->channelData[2]);
           c = 3;
       }
       glRotatef (node->channelData[c++], 0,0,1);
       glRotatef (node->channelData[c++], 1,0,0);
       glRotatef (node->channelData[c++], 0,1,0);
       glTranslatef ( -parentPos[0], -parentPos[1], -parentPos[2]);
       drawLine (yellow, parentPos, nodePos);

       for(int i=0; i<node->numChildren; i++) {
          if(node->children[i])
             drawNode(nodePos[0], nodePos[1], nodePos[2], node->children[i]);
       }

    glPopMatrix();
}

void drawSkeleton()
{
    drawNode(0, 0, 0, root);
}

void freeTree()
{
    freeNode(root);
}

void freeNode(Node* node)
{
    //printf("Freeing %s %p\n", node->name,node->children);
    if(node == NULL) return;
    //printf("Freeing children...\n");
    for(int i=0; i<node->numChildren; i++) {
        //printf(">>> child %d\n", i);
        freeNode(node->children[i]);
    }
    //printf("Freeing channel data...\n");
    free(node->channelData);
    if(node->numChildren>0) {
        //printf("Freeing children array...\n");
        free(node->children);
    }
}

// **********************************************************************
//  Desenha um quadriculado para representar um piso
// **********************************************************************
void drawFloor()
{
    const float LARG = 1000.0;
    int qtd = 40;
    float delta = (2*LARG)/(qtd-1);
    float z = -LARG;

    int i;
    for (i=0; i<qtd; i++)
    {
        glBegin(GL_LINES);
        glVertex3f(-LARG,0,z);
        glVertex3f(LARG,0,z);
        glEnd();
        glBegin(GL_LINES);
        glVertex3f(z,0,-LARG);
        glVertex3f(z,0,LARG);
        glEnd();

        z += delta;
    }

}
// **********************************************************************
//  Desenha os eixos coordenados
// **********************************************************************
void drawAxes()
{
    glBegin(GL_LINES);
    glColor3f(1,0,0); // vermelho

    glVertex3f(0,0,0); // Eixo X
    glVertex3f(50,0,0);

    glVertex3f(0,0,0); // Eixo Y
    glVertex3f(0,50,0);

    glVertex3f(0,0,0); // Eixo X
    glVertex3f(0,0,50);
    glEnd();

}

// Função callback para eventos de botões do mouse
void mouse(int button, int state, int x, int y)
{
	if(state==GLUT_DOWN)
	{
		// Salva os parâmetros atuais
		x_ini = x;
		y_ini = y;
        ObsIni[0] = Obs[0];
        ObsIni[1] = Obs[1];
        ObsIni[2] = Obs[2];
		rotX_ini = rotX;
		rotY_ini = rotY;
		bot = button;
	}
	else bot = -1;
}

// Função callback para eventos de movimento do mouse
#define SENS_ROT	5.0
#define SENS_OBS	5.0
void move(int x, int y)
{
	// Botão esquerdo ?
	if(bot==GLUT_LEFT_BUTTON)
	{
		// Calcula diferenças
		int deltax = x_ini - x;
		int deltay = y_ini - y;
		// E modifica ângulos
		rotY = rotY_ini - deltax/SENS_ROT;
		rotX = rotX_ini - deltay/SENS_ROT;
	}
	// Botão direito ?
	else if(bot==GLUT_RIGHT_BUTTON)
	{
		// Calcula diferença
		int deltaz = y_ini - y;
		// E modifica distância do observador
		//Obs.x = x;
		//Obs.y = y;
        Obs[2] = ObsIni[2] - deltaz/SENS_OBS;
	}
	//PosicionaObservador();
	glutPostRedisplay();
}

// **********************************************************************
//  Posiciona observador
// **********************************************************************
void posUser()
{
    // Set the clipping volume
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60,ratio,0.01,2000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Especifica posição do observador e do alvo
    glTranslatef(Obs[0], Obs[1], Obs[2]);
	glRotatef(rotX,1,0,0);
	glRotatef(rotY,0,1,0);

}

// **********************************************************************
//  Callback para redimensionamento da janela OpenGL
// **********************************************************************
void reshape( int w, int h )
{
    // Prevent a divide by zero, when window is too short
    // (you cant make a window of zero width).
    if(h == 0)
        h = 1;

    ratio = 1.0f * w / h;
    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    posUser();
}

// **********************************************************************
//  Callback para desenho da tela
// **********************************************************************
void display()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    posUser();

    glMatrixMode(GL_MODELVIEW);

    glRotatef(30,0,1,0); // Gira o cenario todo
    drawAxes();
    glColor3f(0.0f,0.0,1.0f); // azul
    drawFloor();

    glPushMatrix();
    //glRotatef(angX,1,0,0);
    glRotatef(angY,0,1,0);
    glColor3f(1.0f,1.0,0.0f); // amarelo
    drawSkeleton();
    glPopMatrix();

    glutSwapBuffers();
}

// **********************************************************************
//  Callback para eventos de teclado
// **********************************************************************
void keyboard ( unsigned char key, int x, int y )
{
    switch ( key )
    {
    case 27:        // Termina o programa qdo
        freeTree();
        exit ( 0 );   // a tecla ESC for pressionada
        break;

    default:
        break;
    }
}

// **********************************************************************
//  Callback para eventos de teclas especiais
// **********************************************************************
void arrow_keys ( int a_keys, int x, int y )
{
    float passo = 3.0;
    switch ( a_keys )
    {
    case GLUT_KEY_RIGHT:
        if(++curFrame >= totalFrames)
            curFrame = 0;
        apply();
        glutPostRedisplay();
        break;
    case GLUT_KEY_LEFT:
        if(--curFrame < 0)
            curFrame = totalFrames-1;
        apply();
        glutPostRedisplay();
        break;
    case GLUT_KEY_UP:
        //
        glutPostRedisplay();
        break;
    case GLUT_KEY_DOWN:
        //
        glutPostRedisplay();
        break;
    default:
        break;
    }
}

// **********************************************************************
//	Inicializa os parâmetros globais de OpenGL
// **********************************************************************
void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Fundo de tela preto

    angX = 0.0;
    angY = 0.0;
}

// **********************************************************************
//  Programa principal
// **********************************************************************
int main (int argc, char** argv)
{
    glutInit            ( &argc, argv );
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowPosition (0,0);

    // Define o tamanho inicial da janela grafica do programa
    glutInitWindowSize  ( 650, 500);

    // Cria a janela na tela, definindo o nome da
    // que aparecera na barra de título da janela.
    glutCreateWindow    ("BVH Viewer" );

    // executa algumas inicializações
    init ();

    // Exemplo: monta manualmente um esqueleto
    // (no trabalho, deve-se ler do arquivo)
    initMaleSkel();

    // Define que o tratador de evento para
    // o redesenho da tela. A funcao "display"
    // será chamada automaticamente quando
    // for necessário redesenhar a janela
    glutDisplayFunc ( display );

    // Define que a função que irá rodar a
    // continuamente. Esta função é usada para fazer animações
    // A funcao "display" será chamada automaticamente
    // sempre que for possível
    //glutIdleFunc ( display );

    // Define que o tratador de evento para
    // o redimensionamento da janela. A funcao "reshape"
    // será chamada automaticamente quando
    // o usuário alterar o tamanho da janela
    glutReshapeFunc ( reshape );

    // Define que o tratador de evento para
    // as teclas. A funcao "keyboard"
    // será chamada automaticamente sempre
    // o usuário pressionar uma tecla comum
    glutKeyboardFunc ( keyboard );

    // Define que o tratador de evento para
    // as teclas especiais(F1, F2,... ALT-A,
    // ALT-B, Teclas de Seta, ...).
    // A funcao "arrow_keys" será chamada
    // automaticamente sempre o usuário
    // pressionar uma tecla especial
    glutSpecialFunc ( arrow_keys );

    // Registra a função callback para eventos de botões do mouse
	glutMouseFunc(mouse);

	// Registra a função callback para eventos de movimento do mouse
	glutMotionFunc(move);

    // inicia o tratamento dos eventos
    glutMainLoop ( );
}
