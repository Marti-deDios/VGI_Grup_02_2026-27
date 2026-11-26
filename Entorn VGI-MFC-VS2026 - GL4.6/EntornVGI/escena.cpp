//******** PRACTICA VISUALITZACI? GR?FICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn b?sic VS2026 MULTIFINESTRA amb OpenGL 4.6, interf?cie MFC i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Juli?, D?bora Gil, Enric Mart? G?dia (Setembre 2026)
// escena.cpp : Aqui es on ha d'anar el codi de les funcions que 
//              dibuixin les escenes.
//
//    Versi? 2.0:	- Objectes Cub, Esfera, Tetera (primitives libreria GLUT)
//
//	  Versi? 2.2:	- Objectes Cub, Esfera, Tetera definides en fitxer font glut_geometry amb altres primitives GLUT
//
//	  Versi? 2.5:	- Objectes cubRGB i Tie (nau Star Wars fet per alumnes)
//

#include "pch.h"
#include "material.h"
#include "visualitzacio.h"
#include "escena.h"
#include <cmath>

// Prototip previ de dibuixa perqu? pugui rebre l'array de textures
void dibuixa(GLuint sh_programID, char obj, glm::mat4 MatriuTG,
	GLuint texturesBoles[16], float posicioBolaBlancaX, float posicioBolaBlancaY,
	glm::mat4 rotacioBolaBlanca, bool bolaBlancaActiva,
	const glm::vec2 posicioBolesColor[15], const glm::mat4 rotacioBolesColor[15],
	const bool bolesColorActives[15], float angleTac, float desplacamentTac, bool mostrarTac, const float enfonsamentTronera[16], bool mostrarLiniaBlanca, COBJModel* taulaOBJ, bool modeEscacs, const FutbolBillar* futbol);

// Dibuixa Eixos Coordenades M?n i Reixes, activant un shader propi.
void dibuixa_Eixos(GLuint ax_programID, bool eix, GLuint axis_Id, CMask3D reixa, CPunt3D hreixa,
	glm::mat4 MatriuProjeccio, glm::mat4 MatriuVista)
{
	// Visualitzaci? Eixos Coordenades M?n
	glUseProgram(ax_programID);

	// Pas Matrius Projecci? i Vista Vista a shader
	glUniformMatrix4fv(glGetUniformLocation(ax_programID, "projectionMatrix"), 1, GL_FALSE, &MatriuProjeccio[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(ax_programID, "viewMatrix"), 1, GL_FALSE, &MatriuVista[0][0]);

	// Attribute Locations must be setup before calling glLinkProgram()
	glBindAttribLocation(ax_programID, 0, "in_Vertex"); // V?rtexs
	glBindAttribLocation(ax_programID, 1, "in_Color");	// Color

	//  Dibuix dels eixos
	if (eix) draw_Eixos(axis_Id);
}

// Dibuixa Skybox en forma de Cub, activant un shader propi.
void dibuixa_Skybox(GLuint sk_programID, GLuint cmTexture, char eix_Polar, glm::mat4 MatriuProjeccio, glm::mat4 MatriuVista)
{
	glm::mat4 ModelMatrix(1.0);

	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content

	// Activaci? shader per a cub skybox
	glUseProgram(sk_programID);

	// Pas Matrius Projecci? i Vista a shader
	glUniformMatrix4fv(glGetUniformLocation(sk_programID, "projectionMatrix"), 1, GL_FALSE, &MatriuProjeccio[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(sk_programID, "viewMatrix"), 1, GL_FALSE, &MatriuVista[0][0]);

	// Rotar skyBox per a orientar sobre eix superior Z o X en Vista Esf?rica (POLARX, POLARY, POLARZ)
	if (eix_Polar == POLARZ) ModelMatrix = glm::rotate(ModelMatrix, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
	else if (eix_Polar == POLARX) ModelMatrix = glm::rotate(ModelMatrix, radians(-90.0f), vec3(0.0f, 0.0f, 1.0f));

	// Escalar Cub Skybox a 5000 per encabir objectes escena a l'interior
	ModelMatrix = glm::scale(ModelMatrix, vec3(5000.0f, 5000.0f, 5000.0f));		//glScaled(5000.0, 5000.0, 5000.0);
	glUniformMatrix4fv(glGetUniformLocation(sk_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);

	// Activar textura cubemaps del Skybox per encabir objectes escena
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cmTexture);

	// Attribute Locations must be setup before calling glLinkProgram()
	glBindAttribLocation(sk_programID, 0, "in_Vertex"); // V?rtexs

	//  Dibuix del Skybox
	drawCubeSkybox();

	glDepthFunc(GL_LESS); // set depth function back to default
}


// dibuixa_EscenaGL: Dibuix de l'escena amb comandes GL
// VGI_AULA_UAB_V2: laboratori immersiu de la UAB. 4 parets i sostre fora de la camera superior.

// Només afecta la geometria del decorat: la taula Blender, la fisica i el HUD son independents.

static GLuint UAB_CarregaTextura(const char* fitxer)

{

    return static_cast<GLuint>(loadIMA_SOIL(fitxer));

}



static void UAB_Cub(GLuint sh, const glm::mat4& base,

    const glm::vec3& pos, const glm::vec3& mida,

    float r, float g, float b, bool llum = false)

{

    CColor color = { r, g, b, 1.0f };

    bool material[5] = { llum, true, !llum, false, true };

    SeleccionaColorMaterial(sh, color, material);

    glUniform1i(glGetUniformLocation(sh, "textur"), GL_FALSE);

    glUniform1i(glGetUniformLocation(sh, "modulate"), GL_TRUE);

    glm::mat4 model = glm::scale(glm::translate(base, pos), mida);

    glUniformMatrix4fv(glGetUniformLocation(sh, "modelMatrix"), 1, GL_FALSE, &model[0][0]);

    draw_TriEBO_Object(GLUT_CUBE);

}



// Plano 3D amb UV explicites. Invertim nomes V: l'OBJ/OpenGL rebia la imatge

// amb l'origen vertical oposat; aixo feia que 'VGI' es llegis cap per avall.

static void UAB_Panell(GLuint sh, const glm::mat4& base, GLuint textura,

    const glm::vec3& origen, const glm::vec3& amplada, const glm::vec3& alcada)

{

    if (textura == 0) return;

    const glm::vec3 punts[6] = {

        origen, origen + amplada, origen + amplada + alcada,

        origen, origen + amplada + alcada, origen + alcada

    };

    const float uv[6][2] = {

        {0.0f,0.0f}, {1.0f,0.0f}, {1.0f,1.0f},

        {0.0f,0.0f}, {1.0f,1.0f}, {0.0f,1.0f}

    };

    const glm::vec3 normal = glm::normalize(glm::cross(amplada, alcada));

    GLfloat vertices[6 * 12] = {};

    for (int i = 0; i < 6; ++i)

    {

        const int k = i * 12;

        vertices[k] = punts[i].x;

        vertices[k+1] = punts[i].y;

        vertices[k+2] = punts[i].z;

        vertices[k+3] = 1.0f;

        vertices[k+4] = 1.0f;

        vertices[k+5] = 1.0f;

        vertices[k+6] = 1.0f;

        vertices[k+7] = normal.x;

        vertices[k+8] = normal.y;

        vertices[k+9] = normal.z;

        vertices[k+10] = uv[i][0];

        vertices[k+11] = 1.0f - uv[i][1];

    }

    static GLuint vao = 0, vbo = 0;

    if (vao == 0)

    {

        glGenVertexArrays(1, &vao);

        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        const GLsizei stride = static_cast<GLsizei>(12 * sizeof(GLfloat));

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(GLfloat)));

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(7 * sizeof(GLfloat)));

        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(10 * sizeof(GLfloat)));

        glEnableVertexAttribArray(0);

        glEnableVertexAttribArray(1);

        glEnableVertexAttribArray(2);

        glEnableVertexAttribArray(3);

    }

    else

    {

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);

    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CColor blanc = { 1.0f, 1.0f, 1.0f, 1.0f };

    bool material[5] = { false, true, true, false, true };

    SeleccionaColorMaterial(sh, blanc, material);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, textura);

    glUniform1i(glGetUniformLocation(sh, "texture0"), 0);

    glUniform1i(glGetUniformLocation(sh, "textur"), GL_TRUE);

    glUniform1i(glGetUniformLocation(sh, "modulate"), GL_FALSE);

    glUniform1i(glGetUniformLocation(sh, "flag_invert_y"), GL_FALSE);

    glUniformMatrix4fv(glGetUniformLocation(sh, "modelMatrix"), 1, GL_FALSE, &base[0][0]);

    const GLboolean culling = glIsEnabled(GL_CULL_FACE);

    glDisable(GL_CULL_FACE);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    if (culling) glEnable(GL_CULL_FACE);

    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);

    glUniform1i(glGetUniformLocation(sh, "textur"), GL_FALSE);

    glUniform1i(glGetUniformLocation(sh, "modulate"), GL_TRUE);

}



// Estacio amb monitor realista: marc, pantalla amb editor de codi 3D, suport,

// teclat tecla a tecla, portatil, sobre de noguera i cadira amb base de cinc radis.

static void UAB_Estacio(GLuint sh, const glm::mat4& base,

    GLuint monitor, GLuint fusta, float x, float y, float costat)

{

    const float s = costat;

    const float aluR = 0.30f, aluG = 0.37f, aluB = 0.40f;

    UAB_Cub(sh, base, glm::vec3(x,y,-1.32f), glm::vec3(1.85f,0.99f,0.12f), 0.35f,0.25f,0.19f);

    UAB_Panell(sh, base, fusta, glm::vec3(x-0.91f,y-0.475f,-1.255f),

        glm::vec3(1.82f,0.0f,0.0f),glm::vec3(0.0f,0.95f,0.0f));

    for (int ix=-1; ix<=1; ix+=2)

        for (int iy=-1; iy<=1; iy+=2)

            UAB_Cub(sh,base,glm::vec3(x+ix*0.77f,y+iy*0.35f,-1.86f),

                glm::vec3(0.085f,0.085f,1.13f),aluR,aluG,aluB);

    // Computer: panell prim amb base i bisell real; pantalla cap a la taula.

    UAB_Cub(sh,base,glm::vec3(x,y+s*0.15f,-0.79f),

        glm::vec3(1.20f,0.075f,0.77f),0.08f,0.11f,0.14f);

    UAB_Cub(sh,base,glm::vec3(x,y+s*0.10f,-0.79f),

        glm::vec3(1.13f,0.012f,0.71f),0.19f,0.23f,0.27f);

    if (s > 0.0f)

        UAB_Panell(sh,base,monitor,glm::vec3(x-0.55f,y+s*0.088f,-1.125f),

            glm::vec3(1.10f,0,0),glm::vec3(0,0,0.665f));

    else

        UAB_Panell(sh,base,monitor,glm::vec3(x+0.55f,y+s*0.088f,-1.125f),

            glm::vec3(-1.10f,0,0),glm::vec3(0,0,0.665f));

    UAB_Cub(sh,base,glm::vec3(x,y+s*0.19f,-1.20f),

        glm::vec3(0.075f,0.12f,0.20f),aluR,aluG,aluB);

    UAB_Cub(sh,base,glm::vec3(x,y+s*0.19f,-1.24f),

        glm::vec3(0.36f,0.26f,0.03f),aluR,aluG,aluB);

    UAB_Cub(sh,base,glm::vec3(x,y-s*0.27f,-1.224f),

        glm::vec3(0.97f,0.35f,0.035f),0.10f,0.13f,0.15f);

    for (int row=0;row<3;++row)

        for(int key=0;key<12;++key)

            UAB_Cub(sh,base,

                glm::vec3(x-0.42f+key*0.076f,y-s*(0.39f-row*0.100f),-1.199f),

                glm::vec3(0.056f,0.061f,0.013f),0.32f,0.36f,0.38f);

    UAB_Cub(sh,base,glm::vec3(x+0.72f,y-s*0.23f,-1.206f),

        glm::vec3(0.12f,0.19f,0.027f),0.14f,0.17f,0.19f);

    // Cadira d'oficina amb seient, respatller i estructura fina.

    const float cy = y-s*1.20f;

    UAB_Cub(sh,base,glm::vec3(x,cy,-1.79f),

        glm::vec3(0.66f,0.68f,0.135f),0.18f,0.25f,0.31f);

    UAB_Cub(sh,base,glm::vec3(x,cy-s*0.34f,-1.35f),

        glm::vec3(0.69f,0.12f,0.84f),0.21f,0.29f,0.34f);

    UAB_Cub(sh,base,glm::vec3(x,cy,-2.08f),

        glm::vec3(0.075f,0.075f,0.57f),aluR,aluG,aluB);

    for (int j=-2;j<=2;++j)

    {

        const bool vertical = (j%2==0);

        UAB_Cub(sh,base,glm::vec3(x+j*0.14f,cy+(j%2)*0.17f,-2.39f),

            glm::vec3(vertical?0.33f:0.20f,vertical?0.055f:0.24f,0.045f),aluR,aluG,aluB);

    }

    for (int ix=-1;ix<=1;ix+=2)

        UAB_Cub(sh,base,glm::vec3(x+ix*0.36f,cy,-1.60f),

            glm::vec3(0.060f,0.55f,0.045f),aluR,aluG,aluB);

}



static void UAB_Planta(GLuint sh, const glm::mat4& base, float x, float y)

{

    UAB_Cub(sh,base,glm::vec3(x,y,-2.21f),glm::vec3(0.51f,0.51f,0.60f),0.34f,0.31f,0.25f);

    UAB_Cub(sh,base,glm::vec3(x,y,-1.43f),glm::vec3(0.08f,0.08f,1.12f),0.27f,0.39f,0.24f);

    for (int j=-2;j<=2;++j)

    {

        UAB_Cub(sh,base,glm::vec3(x+j*0.25f,y-0.20f,-0.79f+j*0.06f),

            glm::vec3(0.44f,0.14f,0.08f),0.19f,0.47f,0.31f);

        UAB_Cub(sh,base,glm::vec3(x+j*0.19f,y+0.18f,-0.69f+j*0.08f),

            glm::vec3(0.31f,0.15f,0.06f),0.23f,0.53f,0.34f);

    }

}



static void EscacsForma(GLuint sh,const glm::mat4& base,glm::vec3 pos,glm::vec3 escala,CColor c,int tipus) {
    bool mat[5]={false,true,true,true,true};
    SeleccionaColorMaterial(sh,c,mat);
    glUniform1i(glGetUniformLocation(sh,"textur"),GL_FALSE);
    glm::mat4 m=glm::scale(glm::translate(base,pos),escala);
    glUniformMatrix4fv(glGetUniformLocation(sh,"modelMatrix"),1,GL_FALSE,&m[0][0]);
    if(tipus==0) draw_TriEBO_Object(GLUT_CUBE); else draw_TriEBO_Object(GLU_SPHERE);
}
static void EscacsPeca(GLuint sh,const glm::mat4& base,int tipus,bool blanca) {
    CColor c=blanca?CColor{0.91f,0.83f,0.65f,1}:CColor{0.12f,0.16f,0.23f,1};
    CColor orC={0.78f,0.55f,0.18f,1};
    EscacsForma(sh,base,glm::vec3(0,0,0.025f),glm::vec3(0.52f,0.52f,0.13f),c,1);
    EscacsForma(sh,base,glm::vec3(0,0,0.052f),glm::vec3(0.43f,0.43f,0.045f),orC,1);
    EscacsForma(sh,base,glm::vec3(0,0,0.13f),glm::vec3(0.30f,0.30f,0.45f),c,1);
    float h=tipus==0?0.20f:0.24f;
    EscacsForma(sh,base,glm::vec3(0,0,h),glm::vec3(0.36f,0.36f,0.08f),c,1);
    if(tipus==2) {
        EscacsForma(sh,base,glm::vec3(0,0,h+0.045f),glm::vec3(0.14f,0.14f,0.075f),c,0);
        for(int i=-1;i<=1;i+=2)for(int j=-1;j<=1;j+=2) EscacsForma(sh,base,glm::vec3(i*0.052f,j*0.052f,h+0.10f),glm::vec3(0.045f,0.045f,0.05f),c,0);
    } else if(tipus==3) {
        EscacsForma(sh,base,glm::vec3(0.025f,0,h+0.065f),glm::vec3(0.37f,0.27f,0.37f),c,1);
        EscacsForma(sh,base,glm::vec3(0.065f,0,h+0.035f),glm::vec3(0.24f,0.30f,0.16f),c,1);
    } else {
        EscacsForma(sh,base,glm::vec3(0,0,h+0.065f),glm::vec3(0.30f,0.30f,tipus==1?0.30f:0.40f),c,1);
        if(tipus==5) { EscacsForma(sh,base,glm::vec3(0,0,h+0.17f),glm::vec3(0.035f,0.035f,0.11f),orC,0); EscacsForma(sh,base,glm::vec3(0,0,h+0.18f),glm::vec3(0.12f,0.035f,0.035f),orC,0); }
        if(tipus==4) for(int k=0;k<5;++k) {float a=k*1.256637f; EscacsForma(sh,base,glm::vec3(0.065f*std::cos(a),0.065f*std::sin(a),h+0.13f),glm::vec3(0.14f),orC,1);}
    }
}
static void EscacsSala(GLuint sh,const glm::mat4& b) {
    static GLuint pedra=0, llibres=0;
    if(!pedra) pedra=UAB_CarregaTextura("textures/escacs_marbre.jpg");
    if(!llibres) llibres=UAB_CarregaTextura("textures/escacs_biblioteca.jpg");
    auto cub=[&](float x,float y,float z,float a,float d,float h,float r,float g,float bl,bool l=false){UAB_Cub(sh,b,glm::vec3(x,y,z),glm::vec3(a,d,h),r,g,bl,l);};
    cub(0,0,-2.65f,26,18,0.3f,0.15f,0.16f,0.20f);
    UAB_Panell(sh,b,pedra,glm::vec3(-13,-9,-2.49f),glm::vec3(26,0,0),glm::vec3(0,18,0));
    for(int s=-1;s<=1;s+=2){
        cub(0,s*9,8,26,0.6f,21,0.09f,0.11f,0.16f);
        cub(s*13,0,8,0.6f,18,21,0.09f,0.11f,0.16f);
        for(int k=-2;k<=2;++k){float x=k*4.7f;
            cub(x,s*8.35f,3.0f,0.65f,0.8f,11,0.24f,0.26f,0.31f);
            cub(x,s*8.35f,-2.15f,1.2f,1.05f,0.65f,0.62f,0.43f,0.17f);
            cub(x,s*8.35f,8.4f,1.15f,1.0f,0.6f,0.62f,0.43f,0.17f);
            if(k<2){float a=x+0.7f; cub(a+1.65f,s*8.65f,0.8f,3.3f,0.40f,6.2f,0.22f,0.12f,0.08f);
            UAB_Panell(sh,b,llibres,glm::vec3(s>0?a:a+3.3f,s*8.43f,-2.2f),glm::vec3(s>0?3.3f:-3.3f,0,0),glm::vec3(0,0,6.0f));}
        }
        for(int k=-1;k<=1;++k){float y=k*5.2f;
            cub(s*12.5f,y,3,0.7f,0.7f,11,0.27f,0.28f,0.33f);
            cub(s*12.1f,y,3,0.08f,0.15f,5.2f,0.98f,0.72f,0.30f,true);
        }
        cub(0,s*8.6f,10.0f,25,0.20f,0.12f,0.95f,0.69f,0.29f,true);
        cub(s*12.6f,0,10.0f,0.20f,17,0.12f,0.95f,0.69f,0.29f,true);
        cub(s*9.7f,0,-2.0f,2,2,1,0.12f,0.14f,0.2f);
        glm::mat4 p=glm::scale(glm::translate(b,glm::vec3(s*9.7f,0,-1.5f)),glm::vec3(8));
        EscacsPeca(sh,p,s>0?5:4,s>0);
    }
    cub(0,0,18.0f,26,18,0.4f,0.11f,0.13f,0.18f);
    for(int k=-3;k<=3;++k) cub(k*3.4f,0,17.7f,0.12f,15,0.15f,0.88f,0.66f,0.35f,true);
}
static void EscacsTapet(GLuint sh,const glm::mat4& b,bool blender) {
    float hx=blender?4.02f:4.45f,hy=blender?1.90f:2.05f;
    for(int i=0;i<8;++i)for(int j=0;j<8;++j){
        bool clar=(i+j)%2==0;
        UAB_Cub(sh,b,glm::vec3(-hx+(i+0.5f)*hx/4,-hy+(j+0.5f)*hy/4,0.152f),glm::vec3(hx/4,hy/4,0.006f),clar?0.80f:0.12f,clar?0.73f:0.15f,clar?0.58f:0.22f);
    }
}
static void DibuixaClubClassicV2(GLuint sh, const glm::mat4& base)
{
    // VGI_ORBITAL_ARENA_V1: decorat independent de la taula i de la fisica.
    static GLuint cartell=0, cosmos=0;
    if(!cartell) cartell=UAB_CarregaTextura("textures/vgi_orbital_sign.png");
    if(!cosmos) cosmos=UAB_CarregaTextura("textures/vgi_orbital_panorama.jpg");
    auto cub = [&](float x,float y,float z,float a,float b,float c,float r,float g,float bl,bool llum=false) {
        UAB_Cub(sh,base,glm::vec3(x,y,z),glm::vec3(a,b,c),r,g,bl,llum);
    };
    // Plataforma de competicio i paviment modular amb geometria real.
    cub(0,0,-2.80f,30,23,0.50f,0.16f,0.20f,0.29f);
    cub(0,0,-2.49f,14.6f,8.4f,0.10f,0.24f,0.30f,0.39f);
    for(int s=-1;s<=1;s+=2) {
        cub(0,s*4.25f,-2.42f,14.8f,0.07f,0.045f,0.18f,0.82f,1.0f,true);
        cub(s*7.4f,0,-2.42f,0.07f,8.5f,0.045f,0.52f,0.40f,1.0f,true);
    }
    for(int i=-7;i<=7;++i) cub(i*1.95f,0,-2.54f,0.025f,22,0.025f,0.27f,0.34f,0.44f);
    for(int i=-5;i<=5;++i) cub(0,i*2.0f,-2.53f,29,0.025f,0.025f,0.27f,0.34f,0.44f);
    // Observatori orbital: finestres panoramiques, consoles i pilars.
    for(int k=-1;k<=1;k+=2) {
        const float s=static_cast<float>(k);
        cub(0,s*11.0f,4.6f,29.8f,0.5f,14.0f,0.16f,0.20f,0.30f);
        cub(s*14.8f,0,4.6f,0.5f,22,14.0f,0.16f,0.20f,0.30f);
        // Cada cara es defineix des de l'esquerra de l'observador interior.
        UAB_Panell(sh,base,cosmos,glm::vec3(-s*13.6f,s*10.70f,-1.75f),glm::vec3(s*27.2f,0,0),glm::vec3(0,0,8.6f));
        UAB_Panell(sh,base,cosmos,glm::vec3(s*14.50f,s*10.2f,-1.75f),glm::vec3(0,-s*20.4f,0),glm::vec3(0,0,8.6f));
        for(int i=-3;i<=3;++i) {
            cub(i*4.2f,s*10.45f,2.7f,0.32f,0.65f,10.4f,0.30f,0.37f,0.48f);
            cub(i*4.2f,s*10.08f,2.7f,0.065f,0.06f,9.6f,0.38f,0.66f,1.0f,true);
        }
        for(int i=-2;i<=2;++i) {
            cub(s*14.27f,i*4.1f,2.7f,0.65f,0.32f,10.4f,0.30f,0.37f,0.48f);
            cub(s*13.90f,i*4.1f,2.7f,0.06f,0.065f,9.6f,0.52f,0.40f,1.0f,true);
        }
        cub(0,s*10.30f,-1.95f,28.0f,0.9f,0.46f,0.31f,0.38f,0.49f);
        cub(s*14.1f,0,-1.95f,0.9f,21.3f,0.46f,0.31f,0.38f,0.49f);
        cub(0,s*10.0f,-1.68f,28.0f,0.06f,0.065f,0.18f,0.82f,1.0f,true);
        cub(s*13.8f,0,-1.68f,0.06f,21.3f,0.065f,0.52f,0.40f,1.0f,true);
        // Retol principal als dos extrems: mida 9 x 3.375, lluny del HUD.
        cub(s*12.8f,0,0.70f,0.32f,9.4f,3.75f,0.045f,0.08f,0.14f);
        UAB_Panell(sh,base,cartell,glm::vec3(s*12.60f,s*4.5f,-0.9875f),glm::vec3(0,-s*9.0f,0),glm::vec3(0,0,3.375f));
        cub(s*12.57f,0,-1.12f,0.055f,9.25f,0.07f,0.30f,0.84f,1.0f,true);
        cub(s*12.57f,0,2.52f,0.055f,9.25f,0.07f,0.52f,0.40f,1.0f,true);
        // Retol lateral, orientat correctament a cadascuna de les parets.
        cub(0,s*9.6f,0.60f,8.7f,0.28f,3.5f,0.045f,0.08f,0.14f);
        UAB_Panell(sh,base,cartell,glm::vec3(-s*4.2f,s*9.42f,-0.975f),glm::vec3(s*8.4f,0,0),glm::vec3(0,0,3.15f));
        // Grades i bancs integrats als laterals, sense envair la taula.
        for(int i=-1;i<=1;i+=2) {
            cub(i*8.4f,s*7.7f,-2.20f,4.4f,1.3f,0.55f,0.27f,0.33f,0.43f);
            cub(i*8.4f,s*8.2f,-1.67f,4.4f,0.35f,0.85f,0.19f,0.24f,0.36f);
            cub(i*8.4f,s*7.0f,-2.02f,4.2f,0.06f,0.07f,0.28f,0.77f,1.0f,true);
        }
        // Estructura superior oberta per no ocultar la camera zenital.
        cub(0,s*9.8f,7.6f,29.0f,0.9f,0.40f,0.27f,0.34f,0.47f);
        cub(0,s*9.4f,7.35f,28.0f,0.10f,0.07f,0.34f,0.79f,1.0f,true);
        cub(s*13.8f,0,7.6f,0.9f,20.0f,0.40f,0.27f,0.34f,0.47f);
    }
}

static void DibuixaAulaUAB(GLuint sh, const glm::mat4& base)

{

    static GLuint paviment=0, vidre=0, pissarra=0, placa=0, fusta=0, monitor=0, sostre=0;

    if(!paviment) paviment=UAB_CarregaTextura("textures/uab_v2_terra.jpg");

    if(!vidre) vidre=UAB_CarregaTextura("textures/uab_v2_vidriera.jpg");

    if(!pissarra) pissarra=UAB_CarregaTextura("textures/uab_aula_pissarra.png");

    if(!placa) placa=UAB_CarregaTextura("textures/uab_aula_placa.png");

    if(!fusta) fusta=UAB_CarregaTextura("textures/uab_v2_noguera.jpg");

    if(!monitor) monitor=UAB_CarregaTextura("textures/uab_v2_monitor.jpg");

    if(!sostre) sostre=UAB_CarregaTextura("textures/uab_v2_sostre.jpg");



    // Paviment realista fins a cada paret. Es veu sencer a la vista superior.

    UAB_Cub(sh,base,glm::vec3(0.0f,0.0f,-2.82f),

        glm::vec3(49.0f,38.0f,0.60f),0.28f,0.33f,0.35f);

    UAB_Panell(sh,base,paviment,glm::vec3(-24.0f,-18.5f,-2.515f),

        glm::vec3(48.0f,0.0f,0.0f),glm::vec3(0.0f,37.0f,0.0f));



    // Quatre murs 3D complets. El sostre (z=23) queda molt per sobre

    // de la camera superior del projecte (z=14), sense amagar les boles.

    for (int k=-1;k<=1;k+=2)

    {

        const float s=static_cast<float>(k);

        UAB_Cub(sh,base,glm::vec3(0.0f,s*18.20f,10.0f),

            glm::vec3(48.0f,0.35f,25.0f),0.35f,0.40f,0.42f);

        const glm::vec3 orig=s>0 ? glm::vec3(-21.0f,17.99f,-0.90f):glm::vec3(21.0f,-17.99f,-0.90f);

        UAB_Panell(sh,base,vidre,orig,

            glm::vec3(s*42.0f,0.0f,0.0f),glm::vec3(0.0f,0.0f,8.10f));

        // Estructura de vidre: muntants negres i travessers metal.lics.

        for (int j=-6;j<=6;++j)

            UAB_Cub(sh,base,glm::vec3(j*3.27f,s*17.91f,2.95f),

                glm::vec3(0.10f,0.12f,8.45f),0.13f,0.20f,0.23f);

        for(int j=0;j<3;++j)

            UAB_Cub(sh,base,glm::vec3(0.0f,s*17.90f,0.05f+j*3.10f),

                glm::vec3(42.0f,0.12f,0.12f),0.13f,0.20f,0.23f);

        UAB_Cub(sh,base,glm::vec3(0.0f,s*17.85f,8.10f),

            glm::vec3(42.0f,0.07f,0.075f),0.29f,0.84f,0.83f,true);

        // Franges d'absorcio acustica a la part alta, amb llum indirecta.

        for (int j=-3;j<=3;++j)

            UAB_Cub(sh,base,glm::vec3(j*6.0f,s*17.85f,14.0f),

                glm::vec3(4.75f,0.075f,2.45f),0.20f,0.27f,0.30f);

    }



    for (int k=-1;k<=1;k+=2)

    {

        const float s=static_cast<float>(k);

        const float x=s>0?24.0f:-24.0f;

        UAB_Cub(sh,base,glm::vec3(x,0.0f,10.0f),

            glm::vec3(0.40f,36.5f,25.0f),0.35f,0.40f,0.42f);

        const glm::vec3 orig=s>0 ? glm::vec3(23.76f,15.5f,-0.95f)

                                   :glm::vec3(-23.76f,-15.5f,-0.95f);

        UAB_Panell(sh,base,vidre,orig,

            glm::vec3(0.0f,s*-31.0f,0.0f),glm::vec3(0.0f,0.0f,8.25f));

        for (int j=-4;j<=4;++j)

            UAB_Cub(sh,base,glm::vec3(x-s*0.25f,j*3.55f,3.03f),

                glm::vec3(0.12f,0.11f,8.50f),0.13f,0.20f,0.23f);

        UAB_Cub(sh,base,glm::vec3(x-s*0.28f,0.0f,8.20f),

            glm::vec3(0.055f,31.0f,0.055f),0.75f,0.63f,0.37f,true);

    }



    // Sostre alt: dona continuïtat al fons de qualsevol costat,

    // sense ocultar les boles a la vista superior (14 unitats d'altura).

    UAB_Cub(sh,base,glm::vec3(0.0f,0.0f,22.80f),

        glm::vec3(48.0f,36.3f,0.24f),0.24f,0.28f,0.28f);

    UAB_Panell(sh,base,sostre,glm::vec3(-23.9f,-18.0f,22.67f),

        glm::vec3(47.8f,0.0f,0.0f),glm::vec3(0.0f,36.0f,0.0f));

    for(int i=-3;i<=3;++i)

    {

        UAB_Cub(sh,base,glm::vec3(i*6.0f,0.0f,22.53f),

            glm::vec3(0.13f,33.0f,0.10f),0.21f,0.27f,0.30f);

        UAB_Cub(sh,base,glm::vec3(i*6.0f,0.0f,22.45f),

            glm::vec3(0.045f,31.0f,0.045f),0.95f,0.77f,0.49f,true);

    }



    // Pissarra digital UAB-VGI en un marc real, davant del mur del fons.

    UAB_Cub(sh,base,glm::vec3(14.15f,0.0f,1.73f),

        glm::vec3(0.21f,12.0f,6.10f),0.13f,0.19f,0.23f);

    UAB_Cub(sh,base,glm::vec3(14.01f,0.0f,1.73f),

        glm::vec3(0.10f,11.64f,5.75f),0.02f,0.07f,0.09f);

    // El panell te l'orientacio correcta cap a la camera de tir (-X).

    // L'UV V es capgira a UAB_Panell per corregir les lletres invertides.

    UAB_Panell(sh,base,pissarra,glm::vec3(13.951f,5.65f,-1.10f),

        glm::vec3(0.0f,-11.30f,0.0f),glm::vec3(0.0f,0.0f,5.66f));

    UAB_Cub(sh,base,glm::vec3(13.89f,5.87f,1.73f),

        glm::vec3(0.08f,0.065f,6.25f),0.30f,0.85f,0.77f,true);

    UAB_Cub(sh,base,glm::vec3(13.89f,-5.87f,1.73f),

        glm::vec3(0.08f,0.065f,6.25f),0.30f,0.85f,0.77f,true);



    // Quatre estacions laterals detallades; no envaeixen les bandes.

    for (int costat=-1;costat<=1;costat+=2)

    {

        const float s=static_cast<float>(costat);

        UAB_Estacio(sh,base,monitor,fusta,-5.5f,s*9.20f,s);

        UAB_Estacio(sh,base,monitor,fusta,5.5f,s*9.20f,s);

        UAB_Planta(sh,base,11.6f,s*13.50f);

        UAB_Cub(sh,base,glm::vec3(0.0f,s*6.75f,-2.44f),

            glm::vec3(23.0f,0.055f,0.04f),0.92f,0.71f,0.40f,true);

    }

    // Placa UAB sobre el terra, visible quan la camera es superior.

    UAB_Panell(sh,base,placa,glm::vec3(-12.0f,-7.4f,-2.495f),

        glm::vec3(5.70f,0.0f,0.0f),glm::vec3(0.0f,2.10f,0.0f));



    // L'estat OpenGL del decorat no pot canviar els colors Blender.

    glBindTexture(GL_TEXTURE_2D,0);

    glUniform1i(glGetUniformLocation(sh,"textur"),GL_FALSE);

    glUniform1i(glGetUniformLocation(sh,"modulate"),GL_TRUE);

    glUniform1i(glGetUniformLocation(sh,"flag_invert_y"),GL_FALSE);

    CColor blancModel={1.0f,1.0f,1.0f,1.0f};

    bool materialModel[5]={false,true,true,true,true};

    SeleccionaColorMaterial(sh,blancModel,materialModel);

}

void dibuixa_EscenaGL(GLuint sh_programID, bool eix, GLuint axis_Id, CMask3D reixa, CPunt3D hreixa, char objecte,
	CColor col_object, bool sw_mat[5],
	bool textur, GLuint texturID[NUM_MAX_TEXTURES], bool textur_map, bool flagInvertY,
	int nptsU, CPunt3D PC_u[MAX_PATCH_CORBA], GLfloat pasCS, bool sw_PC, bool dib_TFrenet,
	COBJModel* objecteOBJ,
	glm::mat4 MatriuTG, GLuint texturesBoles[16],
	float posicioBolaBlancaX, float posicioBolaBlancaY, glm::mat4 rotacioBolaBlanca,
	bool bolaBlancaActiva, const glm::vec2 posicioBolesColor[15],
	const glm::mat4 rotacioBolesColor[15], const bool bolesColorActives[15], float angleTac, float desplacamentTac, bool mostrarTac, const float enfonsamentTronera[16], bool mostrarLiniaBlanca, COBJModel* taulaOBJ, bool modeEscacs, const FutbolBillar* futbol)
{
	float altfar = 0;
	GLint npunts = 0, nvertexs = 0;
	int i, j, k;
	GLdouble tras[3]; //Sierpinski Sponge
	CColor color_vermell, color_Mar;
	bool sw_material[5];

	// Matrius de Transformaci?
	glm::mat4 ModelMatrix(1.0), TransMatrix(1.0); // ScaleMatrix(1.0), RotMatrix(1.0);
	//glm::mat4 NormalMatrix(1.0);

// VAO
	CVAO objectVAO;
	objectVAO.vaoId = 0;	objectVAO.vboId = 0;	objectVAO.nVertexs = 0;

	tras[0] = 0.0;	tras[1] = 0.0; tras[2] = 0.0;
	color_vermell.r = 1.0;	color_vermell.g = 0.0; color_vermell.b = 0.0; color_vermell.a = 1.0;
	sw_material[0] = false;	sw_material[1] = true; sw_material[2] = true; sw_material[3] = false;	sw_material[4] = true;

	// Shader Visualitzaci? Objectes
	glUseProgram(sh_programID);

	// Parametritzaci? i activaci?/desactivaci? de textures
	if (texturID[0] != 0) SetTextureParameters(0, texturID[0], true, true, textur_map, false);
	if (textur) {
		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_TRUE); //glEnable(GL_TEXTURE_2D);
		glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_TRUE); //glEnable(GL_MODULATE);
	}
	else {
		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE); //glDisable(GL_TEXTURE_2D);
		glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_FALSE); //glDisable(GL_MODULATE);
	}
	glUniform1i(glGetUniformLocation(sh_programID, "flag_invert_y"), flagInvertY);

	// Attribute Locations must be setup before calling glLinkProgram()
	glBindAttribLocation(sh_programID, 0, "in_Vertex");		// V?rtexs
	glBindAttribLocation(sh_programID, 1, "in_Color");		// Color
	glBindAttribLocation(sh_programID, 2, "in_Normal");		// Normals
	glBindAttribLocation(sh_programID, 3, "in_TexCoord");	// Textura

	// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material.
	SeleccionaColorMaterial(sh_programID, col_object, sw_mat);

	switch (objecte)
	{

		// Arc
	case ARC:
		// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material pel color de l'objecte.
		SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
		arc(sh_programID, MatriuTG, sw_mat);

		// Activar transpar?ncia
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Dibuix geometria Mar
		color_Mar.r = 0.5;	color_Mar.g = 0.4; color_Mar.b = 0.9; color_Mar.a = 0.5;
		// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material pel color de l'objecte.
		SeleccionaColorMaterial(sh_programID, color_Mar, sw_mat);
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);

		draw_TriVAO_Object(MAR_FRACTAL_VAO);

		// Desactivar transpar?ncia
		glDisable(GL_BLEND);
		break;

		// Dibuix de l'objecte TIE (Nau enemiga Star Wars)
	case TIE:
		tie(sh_programID, MatriuTG, sw_mat);
		break;

		// Dibuix de l'objecte OBJ
	case OBJOBJ:
		ModelMatrix = MatriuTG;
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);

		// Definir caracter?stiques material de cada punt
		SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
		// Objecte OBJ: Dibuix de l'objecte OBJ amb textures amb varis VAO's, un per a cada material.
		objecteOBJ->draw_TriVAO_OBJ(sh_programID);	// Dibuixar VAO a pantalla
		break;

		// Corba Bezier
	case C_BEZIER:
		// Dibuixar Punts de Control
		if (sw_PC)
		{	// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material pel color vermell.
			SeleccionaColorMaterial(sh_programID, color_vermell, sw_material);

			for (int i = 0; i < nptsU; i++)
			{
				ModelMatrix = glm::translate(MatriuTG, vec3(PC_u[i].x, PC_u[i].y, PC_u[i].z));
				// Pas ModelViewMatrix a shader
				glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
				draw_TriEBO_Object(GLU_SPHERE);	//draw_TriVAO_Object(GLU_SPHERE); //gluSphere(5.0, 20, 20);
			}

		}
		// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material pel color de l'objecte.
		SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
		ModelMatrix = MatriuTG;
		// Pas ModelViewMatrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		draw_LinEBO_Object(CRV_BEZIER); //draw_LinVAO_Object(CRV_BEZIER); //draw_Bezier_Curve(nptsU, PC_u, pasCS, false);
		if (dib_TFrenet) draw_TFBezier_Curve(sh_programID, PC_u, nptsU, pasCS, false); // Dibuixar Triedre de Frenet
		break;

		// Corba Lemniscata
	case C_LEMNISCATA:
		// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material pel color de l'objecte.
		SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
		ModelMatrix = MatriuTG;
		// Pas ModelViewMatrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		draw_LinEBO_Object(CRV_LEMNISCATA3D);  //draw_LinVAO_Object(CRV_LEMNISCATA3D); //draw_Lemniscata3D(800, pasCS*20.0);
		if (dib_TFrenet) draw_TFLemniscata3D(sh_programID, 800, pasCS * 20.0);
		break;

		// Corba BSpline
	case C_BSPLINE:
		// Dibuixar Punts de Control
		if (sw_PC)
		{	// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material pel color vermell.
			SeleccionaColorMaterial(sh_programID, color_vermell, sw_material);
			for (int i = 0; i < nptsU; i++)
			{
				ModelMatrix = glm::translate(MatriuTG, vec3(PC_u[i].x, PC_u[i].y, PC_u[i].z));
				// Pas ModelViewMatrix a shader
				glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
				draw_TriEBO_Object(GLU_SPHERE); //draw_TriVAO_Object(GLU_SPHERE); //gluSphere(5.0, 20, 20);
			}
		}
		SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
		ModelMatrix = MatriuTG;
		// Pas ModelViewMatrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		draw_LinEBO_Object(CRV_BSPLINE); //draw_LinVAO_Object(CRV_BSPLINE); //draw_BSpline_Curve(nptsU, PC_u, pasCS);
		if (dib_TFrenet) draw_TFBSpline_Curve(sh_programID, PC_u, nptsU, pasCS); // Dibuixar Triedre de Frenet
		break;

		// Corba Hermitte
	case C_HERMITTE:
		// Dibuixar Punts de Control
		if (sw_PC)
		{	// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material pel color vermell.
			SeleccionaColorMaterial(sh_programID, color_vermell, sw_material);
			for (int i = 0; i < nptsU; i++)
			{
				ModelMatrix = glm::translate(MatriuTG, vec3(PC_u[i].x, PC_u[i].y, PC_u[i].z));
				// Pas ModelViewMatrix a shader
				glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
				draw_TriEBO_Object(GLU_SPHERE); //draw_TriVAO_Object(GLU_SPHERE); //gluSphere(5.0, 20, 20);
			}
		}
		SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
		ModelMatrix = MatriuTG;
		// Pas ModelViewMatrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		draw_LinEBO_Object(CRV_HERMITTE);	//draw_LinVAO_Object(CRV_HERMITTE);
		if (dib_TFrenet) draw_TFHermitte_Curve(sh_programID, PC_u, nptsU, pasCS); // Dibuixar Triedre de Frenet
		break;

		// Corba Catmull-Rom
	case C_CATMULL_ROM:
		// Dibuixar Punts de Control
		if (sw_PC)
		{	// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material pel color vermell.
			SeleccionaColorMaterial(sh_programID, color_vermell, sw_material);
			for (int i = 0; i < nptsU; i++)
			{
				ModelMatrix = glm::translate(MatriuTG, vec3(PC_u[i].x, PC_u[i].y, PC_u[i].z));
				// Pas ModelViewMatrix a shader
				glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
				// Pas NormalMatrix a shader
				//NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
				//glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
				draw_TriEBO_Object(GLU_SPHERE); //gluSphere(5.0, 20, 20);
			}
		}
		SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
		ModelMatrix = MatriuTG;
		// Pas ModelViewMatrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		// Pas NormalMatrix a shader
		//NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
		//glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
		//draw_LinVAO_Object(CRV_CATMULL_ROM);
		draw_LinEBO_Object(CRV_CATMULL_ROM);
		if (dib_TFrenet) draw_TFCatmullRom_Curve(sh_programID, PC_u, nptsU, pasCS); // Dibuixar Triedre de Frenet
		break;

		// Matriu de Primitives SENSE pre-compilaci? pr?via en VBO (precompilaci?, draw i alliberaci? VBO en una funci?)
	case MATRIUP:
		SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
		for (i = 0; i < 10; i++)
			for (j = 0; j < 10; j++)
				for (k = 0; k < 10; k++)
				{
					TransMatrix = glm::translate(MatriuTG, vec3(i * 15.0f, j * 15.0f, k * 15.0));
					ModelMatrix = glm::scale(TransMatrix, vec3(5.0f, 5.0f, 5.0f));
					// Pas ModelView Matrix a shader
					glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
					// Pas NormalMatrix a shader
					//NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
					//glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
					glutSolidCube(1.0);
					//glutSolidTorus(1.0,2.0,20,20);
					//glutSolidOctahedron();
					//glutSolidTetrahedron();
					//glutSolidIcosahedron();
					//glutSolidRhombicDodecahedron();
					//glutSolidSierpinskiSponge(3,tras,1.0);
					//glutSolidTeapot(1.0);
					//gluCylinder(0.5,0.5,1.0,20,20);
					//gluSphere(1.0, 20, 20);
				}
		for (i = 0; i < 10; i++)
			for (j = 0; j < 10; j++)
			{
				ModelMatrix = glm::translate(MatriuTG, vec3(i * 15.0f, j * 15.0f, 160.0f));
				// Pas ModelView Matrix a shader
				glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
				// Pas NormalMatrix a shader
				//NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
				//glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
				  //glScaled(5.0, 5.0, 5.0);
				  //glutSolidCube(1.0);
				glutSolidTorus(2.0, 3.0, 20, 20);
				//glutSolidOctahedron();
				//glutSolidTetrahedron();
				//glutSolidIcosahedron();
				//glutSolidRhombicDodecahedron();
				//glutSolidSierpinskiSponge(3,tras,1.0);
				//glutSolidTeapot(1.0);
				//gluCylinder(0.5,0.5,1.0,20,20);
				//gluSphere(1.0, 20, 20);
			}
		// Dibuix una esfera
		TransMatrix = glm::translate(MatriuTG, vec3(200.0f, 200.0f, 200.0f));
		ModelMatrix = glm::scale(TransMatrix, vec3(5.0f, 5.0f, 5.0f));
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		// Pas NormalMatrix a shader
		//NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
		//glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
		gluSphere(1.0, 20, 20);
		break;

		// Matriu de Primitives AMB pre-compilaci? pr?via en VBO 
		//	(separaci? en 3 funcions: *_VBO() per precompilaci?, draw*() per dibuix i deleteVBO() per alliberar VBO)
	case MATRIUP_VAO:
		SeleccionaColorMaterial(sh_programID, col_object, sw_mat);

		for (i = 0; i < 10; i++)
			for (j = 0; j < 10; j++)
				for (k = 0; k < 10; k++)
				{
					TransMatrix = glm::translate(MatriuTG, vec3(i * 15.0f, j * 15.0f, k * 15.0f));
					ModelMatrix = glm::scale(TransMatrix, vec3(5.0f, 5.0f, 5.0f));
					// Pas ModelView Matrix a shader
					glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
					// Pas NormalMatrix a shader
					//NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
					//glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
					draw_TriEBO_Object(GLUT_CUBE); //drawSolidCube();	// Dibuix del cub dins VBO.

				}

		for (i = 0; i < 10; i++)
			for (j = 0; j < 10; j++)
			{
				ModelMatrix = glm::translate(MatriuTG, vec3(i * 15.0f, j * 15.0f, 160.0f));
				// Pas ModelView Matrix a shader
				glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
				// Pas NormalMatrix a shader
				//NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
				//glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
				draw_TriEBO_Object(GLUT_TORUS); //drawSolidTorus(nvertexs);
			}

		// Dibuix una esfera
		TransMatrix = glm::translate(MatriuTG, vec3(200.0f, 200.0f, 200.0f));
		ModelMatrix = glm::scale(TransMatrix, vec3(5.0f, 5.0f, 5.0f));
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		// Pas NormalMatrix a shader
		//NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
		//glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);
		draw_TriEBO_Object(GLU_SPHERE); //gluSphere(1.0, 20, 20);
		break;

		// Dibuix de la resta d'objectes
	default:
		SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
		dibuixa(sh_programID, objecte, MatriuTG, texturesBoles,
			posicioBolaBlancaX, posicioBolaBlancaY, rotacioBolaBlanca,
			bolaBlancaActiva, posicioBolesColor, rotacioBolesColor, bolesColorActives,
			angleTac, desplacamentTac, mostrarTac, enfonsamentTronera, mostrarLiniaBlanca, taulaOBJ, modeEscacs, futbol);
		break;
	}

	// Enviar les comandes gr?fiques a pantalla
	//	glFlush();
}


// dibuixa: Funci? que dibuixa objectes simples de la llibreria GLUT segons obj
static void DibuixaEstadiFutbol(GLuint sh, const glm::mat4& base, const FutbolBillar& f)
{
    auto cub=[&](float x,float y,float z,float a,float b,float c,float r,float g,float bl,bool llum=false){
        UAB_Cub(sh,base,glm::vec3(x,y,z),glm::vec3(a,b,c),r,g,bl,llum);
    };
    auto linia=[&](float x,float y,float a,float b){cub(x,y,0.153f,a,b,0.007f,0.88f,0.95f,0.91f);};
    // Stadium: open roof, stepped stands, floodlights and illuminated entry arches.
    cub(0,0,-2.80f,29,22,0.55f,0.08f,0.11f,0.19f);
    cub(0,0,-2.47f,13,7.8f,0.12f,0.19f,0.25f,0.34f);
    static GLuint cartell=0;
    if(!cartell)cartell=UAB_CarregaTextura("textures/vgi_orbital_sign.png");
    for(int s=-1;s<=1;s+=2){
        for(int k=0;k<5;++k){
            float y=s*(5.6f+k*0.9f),z=-2.1f+k*0.62f;
            cub(0,y,z,21,0.91f,0.55f,0.13f,0.18f,0.27f);
            for(int x=-9;x<=9;++x){
                float r=s<0?0.08f:0.48f, b=s<0?0.55f:0.12f;
                cub(x*0.99f,y,z+0.34f,0.77f,0.57f,0.16f,r,0.19f,b);
                cub(x*0.99f,y+s*0.26f,z+0.55f,0.77f,0.10f,0.44f,r,0.19f,b);
            }
            cub(0,y-s*0.46f,z+0.28f,21,0.045f,0.045f,0.10f,0.62f,1,true);
        }
        cub(0,s*10.5f,2.4f,26,0.4f,10,0.08f,0.12f,0.20f);
        cub(s*13.5f,0,2.4f,0.4f,21,10,0.09f,0.13f,0.22f);
        cub(s*12.9f,0,2.2f,0.20f,8.8f,3.6f,0.025f,0.05f,0.10f);
        UAB_Panell(sh,base,cartell,glm::vec3(s*12.77f,s*4.2f,0.625f),glm::vec3(0,-s*8.4f,0),glm::vec3(0,0,3.15f));
        for(int k=-2;k<=2;++k){
            cub(k*4.4f,s*9.9f,3.5f,0.20f,0.25f,6.2f,0.27f,0.37f,0.48f);
            cub(k*4.4f,s*9.65f,6.2f,2.1f,0.42f,0.34f,0.73f,0.88f,1,true);
        }
    }
    // New football-only table with two goal openings. Classic Blender is untouched.
    cub(0,0,-0.43f,11.1f,5.7f,0.86f,0.075f,0.11f,0.18f);
    for(int s=-1;s<=1;s+=2)for(int t=-1;t<=1;t+=2)
        cub(s*3.95f,t*1.85f,-1.55f,0.7f,0.7f,1.8f,0.12f,0.17f,0.24f);
    cub(0,0,0.04f,9.68f,4.68f,0.20f,0.035f,0.32f,0.14f);
    for(int i=0;i<10;++i) cub(-4.84f+(i+0.5f)*0.968f,0,0.145f,0.968f,4.68f,0.009f,0.045f,i%2?0.37f:0.30f,0.15f);
    linia(0,0,0.035f,4.55f);linia(0,2.25f,9.5f,0.028f);linia(0,-2.25f,9.5f,0.028f);
    for(int s=-1;s<=1;s+=2){
        linia(s*4.72f,0,0.03f,4.5f);linia(s*3.45f,0,0.026f,2.60f);
        linia(s*4.085f,1.30f,1.27f,0.026f);linia(s*4.085f,-1.30f,1.27f,0.026f);
        cub(0,s*2.47f,0.31f,10.5f,0.26f,0.38f,0.10f,0.15f,0.24f);
        cub(0,s*2.60f,0.49f,10.5f,0.045f,0.045f,0.14f,0.75f,1,true);
        for(int t=-1;t<=1;t+=2){
            cub(s*4.97f,t*1.595f,0.31f,0.26f,1.49f,0.38f,0.10f,0.15f,0.24f);
            cub(s*4.84f,t*0.85f,0.60f,0.08f,0.08f,0.90f,0.94f,0.96f,0.99f);
            cub(s*5.50f,t*0.85f,0.60f,0.055f,0.055f,0.90f,0.72f,0.80f,0.88f);
            cub(s*5.17f,t*0.85f,1.04f,0.70f,0.05f,0.05f,0.82f,0.90f,0.98f);
            for(int k=0;k<5;++k)cub(s*(4.88f+k*0.15f),t*0.85f,0.60f,0.012f,0.012f,0.88f,0.54f,0.65f,0.73f);
        }
        cub(s*5.17f,0,0.04f,0.70f,1.72f,0.20f,0.075f,0.21f,0.13f);
        cub(s*4.84f,0,1.04f,0.08f,1.78f,0.08f,0.95f,0.97f,1);
        cub(s*5.50f,0,1.04f,0.05f,1.75f,0.05f,0.82f,0.90f,0.98f);
        for(int i=0;i<=10;++i)cub(s*5.50f,-0.85f+i*0.17f,0.60f,0.014f,0.014f,0.88f,0.66f,0.76f,0.82f);
        for(int i=0;i<=5;++i)cub(s*5.50f,0,0.18f+i*0.17f,0.014f,1.70f,0.014f,0.66f,0.76f,0.82f);
    }
    for(int i=0;i<48;++i){
        float a=i*6.2831853f/48.f;
        glm::mat4 m=glm::translate(base,glm::vec3(0.65f*std::cos(a),0.65f*std::sin(a),0.157f));
        m=glm::rotate(m,a+1.5707963f,glm::vec3(0,0,1));
        UAB_Cub(sh,m,glm::vec3(0),glm::vec3(0.09f,0.024f,0.008f),0.9f,0.97f,0.92f);
    }
    auto esfera=[&](float x,float y,float z,float escala,float r,float g,float b){
        CColor color={r,g,b,1};bool mat[5]={false,true,true,true,true};
        SeleccionaColorMaterial(sh,color,mat);
        glUniform1i(glGetUniformLocation(sh,"textur"),GL_FALSE);
        glm::mat4 m=glm::scale(glm::translate(base,glm::vec3(x,y,z)),glm::vec3(escala));
        glUniformMatrix4fv(glGetUniformLocation(sh,"modelMatrix"),1,GL_FALSE,&m[0][0]);
        draw_TriEBO_Object(GLU_SPHERE);
    };
    for(int i=0;i<11;++i){
        const auto p=f.p[i];
        if(i==0){esfera(p.x,p.y,0.32f,0.8f,0.96f,0.96f,0.96f);
            // Small dark patches distinguish the football from the team balls.
            for(int k=0;k<5;++k){float a=k*6.2831853f/5.f;esfera(p.x+0.114f*std::cos(a),p.y+0.114f*std::sin(a),0.419f,0.19f,0.035f,0.04f,0.05f);}
            esfera(p.x,p.y,0.469f,0.19f,0.035f,0.04f,0.05f);
        }else if(i<=5)esfera(p.x,p.y,0.32f,0.8f,0.08f,0.38f,0.92f);
        else esfera(p.x,p.y,0.32f,0.8f,0.91f,0.10f,0.15f);
    }
    if(!f.moviment && f.esperaGol<=0 && !f.guanyador){
        const auto p=f.p[f.seleccio];
        for(int i=0;i<24;++i){float a=i*6.2831853f/24.f;cub(p.x+0.22f*std::cos(a),p.y+0.22f*std::sin(a),0.158f,0.047f,0.047f,0.015f,1,0.78f,0.19f,true);}
        glm::mat4 m=glm::rotate(glm::translate(base,glm::vec3(p.x,p.y,0.32f)),f.angle,glm::vec3(0,0,1));
        UAB_Cub(sh,m,glm::vec3(-0.82f-f.forca*0.45f,0,0),glm::vec3(1.25f,0.05f,0.05f),0.76f,0.52f,0.27f);
        UAB_Cub(sh,m,glm::vec3(-1.59f-f.forca*0.45f,0,0),glm::vec3(0.30f,0.08f,0.08f),0.08f,0.11f,0.16f);
        for(int i=0;i<12;++i)UAB_Cub(sh,m,glm::vec3(0.25f+i*0.14f,0,-0.16f),glm::vec3(0.055f,0.025f,0.008f),1,0.82f,0.2f,true);
    }
}

void dibuixa(GLuint sh_programID, char obj, glm::mat4 MatriuTG, GLuint texturesBoles[16],
	float posicioBolaBlancaX, float posicioBolaBlancaY, glm::mat4 rotacioBolaBlanca,
	bool bolaBlancaActiva, const glm::vec2 posicioBolesColor[15],
	const glm::mat4 rotacioBolesColor[15], const bool bolesColorActives[15], float angleTac, float desplacamentTac, bool mostrarTac, const float enfonsamentTronera[16], bool mostrarLiniaBlanca, COBJModel* taulaOBJ, bool modeEscacs, const FutbolBillar* futbol)
{
	glm::mat4 NormalMatrix(1.0), ModelMatrix(1.0); // TransMatrix(1.0); // ScaleMatrix(1.0), RotMatrix(1.0);
	GLdouble tras[3]; //Sierpinski Sponge

	tras[0] = 0.0;	tras[1] = 0.0; tras[2] = 0.0;

	switch (obj)
	{

		// Cub
	case CUB:
		ModelMatrix = glm::scale(MatriuTG, vec3(5.0f, 5.0f, 5.0f));
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		draw_TriEBO_Object(GLUT_CUBE);	//draw_TriVAO_Object(GLUT_CUBE);  //glutSolidCube(1.0);
		break;

		// Billar
	case BILLAR:
	{
		if(futbol) { DibuixaEstadiFutbol(sh_programID, MatriuTG, *futbol); break; }
        if(modeEscacs) EscacsSala(sh_programID, MatriuTG);
        else DibuixaClubClassicV2(sh_programID, MatriuTG);

		// Restablim l'estat del shader despres del decorat lluminos.

		CColor blancModel = { 1.0f, 1.0f, 1.0f, 1.0f };

		bool materialModel[5] = { false, true, true, true, true };

		SeleccionaColorMaterial(sh_programID, blancModel, materialModel);

		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);

		glUniform1i(glGetUniformLocation(sh_programID, "texturaActiva"), 0);
		if (taulaOBJ != nullptr)
		{
			glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
			glUniform1i(glGetUniformLocation(sh_programID, "texturaActiva"), 0);
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &MatriuTG[0][0]);
			taulaOBJ->draw_TriVAO_OBJ(sh_programID);
		}
		else
		{
		// 1. Dibuixar el tapete de la mesa de billar (SENSE textura)
		CColor verd;
		verd.r = 0.0f; verd.g = 0.6f; verd.b = 0.15f; verd.a = 1.0f;
		bool materialTapet[5] = { false, true, true, false, true };
		SeleccionaColorMaterial(sh_programID, verd, materialTapet);

		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);

		ModelMatrix = glm::scale(MatriuTG, vec3(10.0f, 5.0f, 0.3f));
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		draw_TriEBO_Object(GLUT_CUBE);

		// 1.5. Calaix / Base inferior de la taula (lleugerament més gran per tapar qualsevol escletxa)
		CColor fustaBase;
		fustaBase.r = 0.35f; fustaBase.g = 0.17f; fustaBase.b = 0.05f; fustaBase.a = 1.0f;
		SeleccionaColorMaterial(sh_programID, fustaBase, materialTapet);

		ModelMatrix = glm::translate(MatriuTG, vec3(0.0f, 0.0f, -0.45f));
		ModelMatrix = glm::scale(ModelMatrix, vec3(10.15f, 5.15f, 0.7f));
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		draw_TriEBO_Object(GLUT_CUBE);


		// 2. Marc de fusta exterior (6 bandes separades)
		CColor fusta;
		fusta.r = 0.4f; fusta.g = 0.2f; fusta.b = 0.07f; fusta.a = 1.0f;
		bool materialMarc[5] = { false, true, true, false, true };
		SeleccionaColorMaterial(sh_programID, fusta, materialMarc);

		// 4 Bandes llargues (Superior/Inferior, Esquerra/Dreta)
		float posXLlargues[4] = { -2.45f,  2.45f, -2.45f,  2.45f };
		float posYLlargues[4] = { 2.7f,   2.7f,  -2.7f,  -2.7f };
		for (int i = 0; i < 4; i++) {
			ModelMatrix = glm::translate(MatriuTG, vec3(posXLlargues[i], posYLlargues[i], 0.15f));
			ModelMatrix = glm::scale(ModelMatrix, vec3(4.1f, 0.4f, 0.6f));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE);
		}

		// 2 Bandes curtes (Laterals Esquerra i Dreta)
		float posXCurtes[2] = { -5.2f, 5.2f };
		for (int i = 0; i < 2; i++) {
			ModelMatrix = glm::translate(MatriuTG, vec3(posXCurtes[i], 0.0f, 0.15f));
			ModelMatrix = glm::scale(ModelMatrix, vec3(0.4f, 4.0f, 0.6f));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE);
		}

		// 2.2. Coixins verds interiors amb tall en angle (Bisel cap al forat)
		CColor verdCoixi;
		verdCoixi.r = 0.0f; verdCoixi.g = 0.5f; verdCoixi.b = 0.12f; verdCoixi.a = 1.0f;
		SeleccionaColorMaterial(sh_programID, verdCoixi, materialTapet);

		// Coixins llargs
		float posYCoixiLlarg[4] = { 2.42f, 2.42f, -2.42f, -2.42f };
		for (int i = 0; i < 4; i++) {
			ModelMatrix = glm::translate(MatriuTG, vec3(posXLlargues[i], posYCoixiLlarg[i], 0.18f));
			ModelMatrix = glm::scale(ModelMatrix, vec3(3.9f, 0.16f, 0.45f));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE);
		}

		// Coixins curts
		float posXCoixiCurt[2] = { -4.92f, 4.92f };
		for (int i = 0; i < 2; i++) {
			ModelMatrix = glm::translate(MatriuTG, vec3(posXCoixiCurt[i], 0.0f, 0.18f));
			ModelMatrix = glm::scale(ModelMatrix, vec3(0.16f, 3.8f, 0.45f));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE);
		}

		// Punts bisellats dels coixins (creen l'embut verd cap als forats com a la foto real)
		float biselX[12] = { -4.42f, -0.48f,  0.48f,  4.42f, -4.42f, -0.48f,  0.48f,  4.42f, -4.92f, -4.92f,  4.92f,  4.92f };
		float biselY[12] = { 2.42f,  2.42f,  2.42f,  2.42f, -2.42f, -2.42f, -2.42f, -2.42f,  1.92f, -1.92f,  1.92f, -1.92f };
		float biselRot[12] = { 35.0f, -35.0f, 35.0f, -35.0f, -35.0f, 35.0f, -35.0f, 35.0f, -35.0f, 35.0f, 35.0f, -35.0f };
		for (int i = 0; i < 12; i++) {
			ModelMatrix = glm::translate(MatriuTG, vec3(biselX[i], biselY[i], 0.18f));
			ModelMatrix = glm::rotate(ModelMatrix, radians(biselRot[i]), vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::scale(ModelMatrix, vec3(0.18f, 0.16f, 0.45f));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE);
		}

		// 2.3. Cantonades i proteccions metàl·liques (en color gris RGB personalitzable)
		CColor grisEsquines;
		grisEsquines.r = 0.5f; grisEsquines.g = 0.5f; grisEsquines.b = 0.5f; grisEsquines.a = 1.0f; // RGB de gris (pots modificar aquests tres números entre 0.0f i 1.0f)
		bool materialMetal[5] = { false, true, true, true, true };
		SeleccionaColorMaterial(sh_programID, grisEsquines, materialMetal);

		// Peces metàl·liques en L per a les 4 cantonades exteriors
		float extX[4] = { -4.95f,  4.95f, -4.95f,  4.95f };
		float extY[4] = { 2.7f,   2.7f,  -2.7f,  -2.7f };
		for (int i = 0; i < 4; i++) {
			ModelMatrix = glm::translate(MatriuTG, vec3(extX[i], extY[i], 0.15f));
			ModelMatrix = glm::scale(ModelMatrix, vec3(0.9f, 0.4f, 0.6f));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE);
		}
		float extX2[4] = { -5.2f,  5.2f, -5.2f,  5.2f };
		float extY2[4] = { 2.25f, 2.25f, -2.25f, -2.25f };
		for (int i = 0; i < 4; i++) {
			ModelMatrix = glm::translate(MatriuTG, vec3(extX2[i], extY2[i], 0.15f));
			ModelMatrix = glm::scale(ModelMatrix, vec3(0.4f, 0.5f, 0.6f));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE);
		}

		// 2 Peces metàl·liques per als forats centrals (A la seva posició original però en gris)
		CColor grisFosc;
		grisFosc.r = 0.45f; grisFosc.g = 0.45f; grisFosc.b = 0.45f; grisFosc.a = 1.0f;
		SeleccionaColorMaterial(sh_programID, grisFosc, materialMetal);

		float centY[2] = { 2.8f, -2.8f }; // Posició original cap enrere
		for (int i = 0; i < 2; i++) {
			ModelMatrix = glm::translate(MatriuTG, vec3(0.0f, centY[i], 0.155f));
			ModelMatrix = glm::scale(ModelMatrix, vec3(1.0f, 0.25f, 0.55f));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE);
		}


		// 2.4. Els 6 forats negres (Mida reduïda i vores suaus sense pics laterals)
		{
			CColor negreTronera;
			negreTronera.r = 0.0f; negreTronera.g = 0.0f; negreTronera.b = 0.0f; negreTronera.a = 1.0f;
			bool materialForat[5] = { false, true, true, false, true };
			SeleccionaColorMaterial(sh_programID, negreTronera, materialForat);

			float zForat = 0.18f;      // Lligerament per sobre del tapet verd
			float gruixForat = 0.04f;  // Gruix pla

			// A. 4 TRONERES DE LES CANTONADES (Més compactes i sense cantonades sortints)
			float sigX[4] = { -1.0f,  1.0f, -1.0f,  1.0f };
			float sigY[4] = { 1.0f,  1.0f, -1.0f, -1.0f };

			for (int i = 0; i < 4; i++) {
				// 1. Fons recte superior/inferior (recollit dins del marge per no fer pics)
				ModelMatrix = glm::translate(MatriuTG, vec3(sigX[i] * 4.76f, sigY[i] * 2.42f, zForat));
				ModelMatrix = glm::scale(ModelMatrix, vec3(0.48f, 0.16f, gruixForat));
				glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
				draw_TriEBO_Object(GLUT_CUBE);

				// 2. Fons recte lateral (recollit contra la paret grisa exterior)
				ModelMatrix = glm::translate(MatriuTG, vec3(sigX[i] * 4.92f, sigY[i] * 2.26f, zForat));
				ModelMatrix = glm::scale(ModelMatrix, vec3(0.16f, 0.48f, gruixForat));
				glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
				draw_TriEBO_Object(GLUT_CUBE);

				// 3. Boca circular reduïda (escala 1.14f en lloc de 1.28f) amb contorn net
				ModelMatrix = glm::translate(MatriuTG, vec3(sigX[i] * 4.72f, sigY[i] * 2.22f, zForat));
				ModelMatrix = glm::scale(ModelMatrix, vec3(1.14f, 1.14f, gruixForat));
				glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
				draw_TriEBO_Object(GLU_SPHERE);
			}

			// B. 2 TRONERES LATERALS CENTRALS (Unió tangent sense esglaons laterals)
			float sigCentY[2] = { 1.0f, -1.0f };

			for (int i = 0; i < 2; i++) {
				// 1. Fons rectangular d'amplada idèntica al diàmetre de l'esfera (0.78f)
				ModelMatrix = glm::translate(MatriuTG, vec3(0.0f, sigCentY[i] * 2.51f, zForat));
				ModelMatrix = glm::scale(ModelMatrix, vec3(0.78f, 0.34f, gruixForat));
				glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
				draw_TriEBO_Object(GLUT_CUBE);

				// 2. Semicercle reduït (escala 1.56f = diàmetre 0.78f) centrat just a la vora del rectangle
				ModelMatrix = glm::translate(MatriuTG, vec3(0.0f, sigCentY[i] * 2.34f, zForat));
				ModelMatrix = glm::scale(ModelMatrix, vec3(1.56f, 1.40f, gruixForat));
				glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
				draw_TriEBO_Object(GLU_SPHERE);
			}
		}

		// 3. Les 4 potes de la taula
		CColor fustaPota;
		fustaPota.r = 0.35f; fustaPota.g = 0.18f; fustaPota.b = 0.05f; fustaPota.a = 1.0f;
		bool materialPota[5] = { false, true, true, false, true };
		SeleccionaColorMaterial(sh_programID, fustaPota, materialPota);

		float posX[4] = { -4.2f,  4.2f, -4.2f,  4.2f };
		float posY[4] = { 2.0f,  2.0f, -2.0f, -2.0f };
		float alturaZ = -2.0f;

		for (int i = 0; i < 4; i++) {
			ModelMatrix = glm::translate(MatriuTG, vec3(posX[i], posY[i], alturaZ));
			ModelMatrix = glm::scale(ModelMatrix, vec3(0.6f, 0.6f, 4.0f));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE);
		}

		}

		bool materialBola[5] = { false, true, true, true, true };
		float escalaBola = 0.55f;
		if(modeEscacs) EscacsTapet(sh_programID,MatriuTG,taulaOBJ!=nullptr); float alturaTapetZ = 0.25f;

		// ACTIVAR TEXTURES PER A LES BOLES
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_TRUE);
		glUniform1i(glGetUniformLocation(sh_programID, "texturaActiva"), 1);
		glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_TRUE);

		// 4. Definici de la Bola Blanca (Cue Ball)
		if (bolaBlancaActiva)
		{
			CColor blanc;
			blanc.r = 1.0f; blanc.g = 1.0f; blanc.b = 1.0f; blanc.a = 1.0f;
			SeleccionaColorMaterial(sh_programID, blanc, materialBola);

			glBindTexture(GL_TEXTURE_2D, texturesBoles[0]);

			// Dibuixem la bola blanca a la seva posicio actual
			glm::mat4 ModelMatrixBola = glm::translate(MatriuTG,
				vec3(posicioBolaBlancaX, posicioBolaBlancaY, alturaTapetZ - 0.30f * enfonsamentTronera[0]));

			// Primer apliquem el gir acumulat en els eixos del tapet XY.
			ModelMatrixBola = ModelMatrixBola * rotacioBolaBlanca;
			ModelMatrixBola = glm::rotate(ModelMatrixBola, radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
			ModelMatrixBola = glm::rotate(ModelMatrixBola, radians(180.0f), vec3(1.0f, 0.0f, 0.0f));
			const float escalaVisual = escalaBola * (1.0f - 0.80f * enfonsamentTronera[0]);
			ModelMatrixBola = glm::scale(ModelMatrixBola, vec3(escalaVisual, escalaVisual, escalaVisual));

			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrixBola[0][0]);
			draw_TriEBO_Object(GLU_SPHERE);
		}

		// 5. Collocaci de les 15 boles de color
		int ordenRack[15] = {
			 1,          // Fila 1
			11,  3,      // Fila 2
			 5,  8, 12,  // Fila 3
			 6,  7, 13,  9, // Fila 4
			 2, 14,  4, 15, 10  // Fila 5
		};

		for (int i = 0; i < 15; ++i) {
			int numBola = ordenRack[i];
			int indiceBola = numBola - 1;
			if (!bolesColorActives[indiceBola]) continue; // La bola ja ha entrat en una tronera.

			if(modeEscacs) {
                int tipus=(numBola<=7?numBola-1:numBola-9); if(tipus==6) tipus=0;
                glm::mat4 p=glm::translate(MatriuTG,glm::vec3(posicioBolesColor[indiceBola].x,posicioBolesColor[indiceBola].y,0.15f-0.30f*enfonsamentTronera[numBola]));
                p=glm::scale(p,glm::vec3(1.0f-0.80f*enfonsamentTronera[numBola]));
                EscacsPeca(sh_programID,p,tipus,numBola<=7); continue;
            }
            CColor col;
            col.r = 1.0f; col.g = 1.0f; col.b = 1.0f; col.a = 1.0f;
            SeleccionaColorMaterial(sh_programID, col, materialBola);

			glBindTexture(GL_TEXTURE_2D, texturesBoles[numBola]);

			glm::mat4 ModelMatrixBola = glm::translate(MatriuTG,
				vec3(posicioBolesColor[indiceBola].x, posicioBolesColor[indiceBola].y,
					alturaTapetZ - 0.30f * enfonsamentTronera[numBola]));
			ModelMatrixBola = ModelMatrixBola * rotacioBolesColor[indiceBola];
			ModelMatrixBola = glm::rotate(ModelMatrixBola, radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
			ModelMatrixBola = glm::rotate(ModelMatrixBola, radians(180.0f), vec3(1.0f, 0.0f, 0.0f));
			const float escalaVisual = escalaBola * (1.0f - 0.80f * enfonsamentTronera[numBola]);
			ModelMatrixBola = glm::scale(ModelMatrixBola, vec3(escalaVisual, escalaVisual, escalaVisual));

			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrixBola[0][0]);
			draw_TriEBO_Object(GLU_SPHERE);
		}

		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
		glUniform1i(glGetUniformLocation(sh_programID, "texturaActiva"), 0);

		if (mostrarLiniaBlanca)
		{
			CColor colorLinia;
			colorLinia.r = 0.95f;
			colorLinia.g = 0.81f;
			colorLinia.b = 0.38f;
			colorLinia.a = 1.0f;
			SeleccionaColorMaterial(sh_programID, colorLinia, materialBola);
			glm::mat4 marca = glm::translate(MatriuTG, vec3(-3.0f, 0.0f, 0.151f));
			marca = glm::scale(marca, vec3(0.025f, taulaOBJ != nullptr ? 4.00f : 4.25f, 0.007f));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &marca[0][0]);
			draw_TriEBO_Object(GLUT_CUBE);
		}

		if (mostrarTac && bolaBlancaActiva)
		{
			glm::mat4 baseTac = glm::translate(MatriuTG,
				vec3(posicioBolaBlancaX, posicioBolaBlancaY, alturaTapetZ));
			baseTac = glm::rotate(baseTac, angleTac, vec3(0.0f, 0.0f, 1.0f));

			CColor blancPunta = { 0.88f, 0.91f, 0.86f, 1.0f };
			CColor fustaTac = { 0.78f, 0.55f, 0.30f, 1.0f };
			CColor gomaTac = { 0.07f, 0.08f, 0.09f, 1.0f };
			CColor plataTac = { 0.65f, 0.68f, 0.68f, 1.0f };
			bool materialTac[5] = { false, true, true, true, true };

			if (!modeEscacs)
            {
                // Eix local Z del cilindre convertit a -X del tac.
                auto segment = [&](float inici, float llarg, float radi, CColor c) {
                    SeleccionaColorMaterial(sh_programID,c,materialTac);
                    glm::mat4 m = glm::translate(baseTac,vec3(-inici-desplacamentTac,0.0f,0.10f));
                    m=glm::rotate(m,radians(-90.0f),vec3(0,1,0));
                    m=glm::scale(m,vec3(radi,radi,llarg));
                    glUniformMatrix4fv(glGetUniformLocation(sh_programID,"modelMatrix"),1,GL_FALSE,&m[0][0]);
                    draw_TriEBO_Object(GLUT_CYLINDER);
                };
                CColor guix={0.12f,0.36f,0.55f,1.0f};
                CColor aur={0.72f,0.49f,0.20f,1.0f};
                CColor noguera={0.43f,0.20f,0.075f,1.0f};
                CColor maple={0.82f,0.64f,0.39f,1.0f};
                segment(0.15f,0.035f,0.022f,guix);
                segment(0.185f,0.065f,0.023f,blancPunta);
                for (int k=0;k<10;++k)
                    segment(0.25f+k*0.115f,0.115f,0.024f+k*0.0012f,maple);
                segment(1.40f,0.045f,0.039f,aur);
                segment(1.445f,0.28f,0.040f,noguera);
                segment(1.725f,0.38f,0.045f,gomaTac);
                for (int k=0;k<8;++k) segment(1.74f+k*0.045f,0.009f,0.046f,noguera);
                segment(2.105f,0.045f,0.047f,aur);
                segment(2.15f,0.07f,0.046f,noguera);
            }
            else {
                SeleccionaColorMaterial(sh_programID, blancPunta, materialTac);
			ModelMatrix = glm::translate(baseTac, vec3(-0.18f - desplacamentTac, 0.0f, 0.12f));
			ModelMatrix = glm::scale(ModelMatrix, vec3(0.06f, 0.045f, 0.045f));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE);

			SeleccionaColorMaterial(sh_programID, fustaTac, materialTac);
			ModelMatrix = glm::translate(baseTac, vec3(-0.83f - desplacamentTac, 0.0f, 0.14f));
			ModelMatrix = glm::scale(ModelMatrix, vec3(1.25f, 0.055f, 0.055f));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE);

			SeleccionaColorMaterial(sh_programID, plataTac, materialTac);
			ModelMatrix = glm::translate(baseTac, vec3(-1.48f - desplacamentTac, 0.0f, 0.14f));
			ModelMatrix = glm::scale(ModelMatrix, vec3(0.06f, 0.083f, 0.083f));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE);

			SeleccionaColorMaterial(sh_programID, gomaTac, materialTac);
			ModelMatrix = glm::translate(baseTac, vec3(-1.77f - desplacamentTac, 0.0f, 0.14f));
			ModelMatrix = glm::scale(ModelMatrix, vec3(0.54f, 0.092f, 0.092f));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE);

			
            }
            CColor colorGuia = { 0.95f, 0.82f, 0.32f, 1.0f };
			SeleccionaColorMaterial(sh_programID, colorGuia, materialTac);
			ModelMatrix = glm::translate(baseTac, vec3(1.22f, 0.0f, -0.09f));
			ModelMatrix = glm::scale(ModelMatrix, vec3(2.0f, 0.015f, 0.01f));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			draw_TriEBO_Object(GLUT_CUBE);
		}

	} break;




	case CUB_RGB:
		ModelMatrix = glm::scale(MatriuTG, vec3(5.0f, 5.0f, 5.0f));
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		draw_TriEBO_Object(GLUT_CUBE_RGB);	//draw_TriVAO_Object(GLUT_CUBE_RGB); // glutSolidCubeRGB(1.0);
		break;

		// Esfera
	case ESFERA:
		ModelMatrix = glm::scale(MatriuTG, vec3(5.0f, 5.0f, 5.0f));
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		draw_TriEBO_Object(GLU_SPHERE); //draw_TriVAO_Object(GLU_SPHERE); //gluSphere(1.0, 40, 40);
		break;

		// Tetera
	case TETERA:
		ModelMatrix = MatriuTG;
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		draw_TriVAO_Object(GLUT_TEAPOT); // glutSolidTeapot(1.0);
		break;
	}
}




// OBJECTE ARC
void arc(GLuint sh_programID, glm::mat4 MatriuTG, bool sw_mat[5])
{
	CColor col_object;
	glm::mat4 ModelMatrix(1.0);
	//glm::mat4 NormalMatrix(1.0);

// Definir VAO del Cub
	//glutSolidCube_VAO(1.0);

// Pota Esquerra
	ModelMatrix = glm::translate(MatriuTG, vec3(0.0f, 0.0f, 2.5f));
	ModelMatrix = glm::scale(ModelMatrix, vec3(1.0f, 1.0f, 5.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLUT_CUBE); // drawSolidCube();	//  glutSolidCube(1.0);

	// Pota Dreta
	ModelMatrix = glm::translate(MatriuTG, vec3(0.0f, 5.0f, 2.5f));
	ModelMatrix = glm::scale(ModelMatrix, vec3(1.0f, 1.0f, 5.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLUT_CUBE); //drawSolidCube();	//  glutSolidCube(1.0);

	// Travesser
	ModelMatrix = glm::translate(MatriuTG, vec3(0.0f, 2.5f, 5.5f));
	ModelMatrix = glm::rotate(ModelMatrix, radians(90.f), vec3(1.0f, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, vec3(1.0f, 1.0f, 6.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLUT_CUBE);	//  glutSolidCube(1.0);

	// Esfera
	col_object.r = 0.0;		col_object.g = 1.0;		col_object.b = 1.0;		col_object.a = 1.0;	// Color blau clar
	SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
	ModelMatrix = glm::translate(MatriuTG, vec3(0.0f, 5.0f, 6.5f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLU_SPHERE); //gluSphere(0.5, 20, 20);

	// Tetera
	col_object.r = 1.0;		col_object.g = 1.0;		col_object.b = 0.0;		col_object.a = 1.0;	// Color groc
	SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
	ModelMatrix = glm::translate(MatriuTG, vec3(0.0f, 0.0f, 6.0f));
	ModelMatrix = glm::rotate(ModelMatrix, radians(-90.f), vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::rotate(ModelMatrix, radians(90.f), vec3(1.0f, 0.0f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, vec3(0.25f, 0.25f, 0.25f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriVAO_Object(GLUT_TEAPOT); //glutSolidTeapot(1.0);
}


// Mar amb ondulacions
CVAO loadSea_VAO(CColor colorM)
{
	int i, j, step, it1, it2;
	double angle, delay, Nx;
	glm::mat4 ModelMatrix(1.0);
	//glm::mat4 NormalMatrix(1.0);

// VAO
	GLuint vaoId = 0; GLuint vboId = 0;
	CVAO seaVAO;
	seaVAO.vaoId = 0;	seaVAO.vboId = 0;	seaVAO.nVertexs = 0;
	std::vector <double> vertices, normals, colors, textures;	// Definici? vectors din?mics per a vertexs, normals i textures 
	vertices.resize(0);		normals.resize(0);		colors.resize(0);	textures.resize(0);			// Reinicialitzar vectors

	// Aigua amb ondulacions simulades a partir de normals sinusoidals
	step = 10; //step = 5;
	delay = 0;
	it1 = 0;

	//   SeleccionaMaterial(MAT_DEFAULT);	
	float h = 2 * PI * step / 500;
	for (j = -250; j < 250 - step; j = j + step)
	{
		delay = 1.0 * h * it1;

		it2 = 0;
		//glColor3f(0.5f, 0.4f, 0.9f);
		for (i = -250; i < 250 - step; i = i + step)
		{	//glBegin(GL_POLYGON);
			angle = 1.0 * it2 * h * 15;
			Nx = -cos(angle);
			//glNormal3f(-cos(angle), 0, 1);
			//glVertex3f(i, j, 0);			// V?rtex P1
			colors.push_back(colorM.r);		colors.push_back(colorM.g);		colors.push_back(colorM.b);		colors.push_back(colorM.a);  // Vector Colors
			normals.push_back(Nx);			normals.push_back(0.0);			normals.push_back(1.0);			// Vector Normals
			textures.push_back(0.0);			textures.push_back(0.0);										// Vector Textures
			vertices.push_back(i);			vertices.push_back(j);			vertices.push_back(0.0);		// Vector Vertices

			angle = 1.0 * (it2 + 1.0) * h * 15;
			Nx = -cos(angle);
			//glNormal3f(-cos(angle), 0, 1);
			//glVertex3f(i + step, j, 0);		// V?rtex P2
			colors.push_back(colorM.r);		colors.push_back(colorM.g);		colors.push_back(colorM.b);		colors.push_back(colorM.a);  // Vector Colors
			normals.push_back(Nx);			normals.push_back(0.0);			normals.push_back(1.0);			// Vector Normals
			textures.push_back(1.0);			textures.push_back(0.0);										// Vector Textures
			vertices.push_back(i + step);		vertices.push_back(j);			vertices.push_back(0.0);		// Vector Vertices

			angle = 1.0 * (it2 + 1.0) * h * 15;
			Nx = -cos(angle);
			//glNormal3f(-cos(angle), 0, 1);
			//glVertex3f(i + step, j + step, 0);// V?rtex P3
			colors.push_back(colorM.r);		colors.push_back(colorM.g);		colors.push_back(colorM.b);		colors.push_back(colorM.a);  // Vector Colors
			normals.push_back(Nx);			normals.push_back(0.0);			normals.push_back(1.0);			// Vector Normals
			textures.push_back(1.0);			textures.push_back(1.0);										// Vector Textures
			vertices.push_back(i + step);		vertices.push_back(j + step);		vertices.push_back(0.0);		// Vector Vertices

			//glEnd();
// ----------------------- VAO
			//std::vector <int>::size_type nv = vertices.size();	// Tamany del vector vertices en elements.
			//draw_GL_TRIANGLES_VAO(vertices, normals, colors, textures);
			//vertices.resize(0);	normals.resize(0);	colors.resize(0);	textures.resize(0);	// Reinicialitzar vectors

			//glBegin(GL_POLYGON);
			angle = 1.0 * it2 * h * 15;
			//glNormal3f(-cos(angle), 0, 1);
			//glVertex3f(i, j, 0);			// V?rtex P1
			colors.push_back(colorM.r);		colors.push_back(colorM.g);		colors.push_back(colorM.b);		colors.push_back(colorM.a);  // Vector Colors
			normals.push_back(-cos(angle));	normals.push_back(0.0);			normals.push_back(1.0);			// Vector Normals
			textures.push_back(1.0);			textures.push_back(1.0);										// Vector Textures
			vertices.push_back(i);			vertices.push_back(j);			vertices.push_back(0.0);		// Vector Vertices

			angle = 1.0 * (it2 + 1) * h * 15;
			//glNormal3f(-cos(angle), 0, 1);
			//glVertex3f(i + step, j + step, 0);// V?rtex P2
			colors.push_back(colorM.r);		colors.push_back(colorM.g);		colors.push_back(colorM.b);		colors.push_back(colorM.a);  // Vector Colors
			normals.push_back(-cos(angle));	normals.push_back(0.0);			normals.push_back(1.0);			// Vector Normals
			textures.push_back(1.0);			textures.push_back(1.0);										// Vector Textures
			vertices.push_back(i + step);		vertices.push_back(j + step);	vertices.push_back(0.0);		// Vector Vertices

			angle = 1.0 * it2 * h * 15;
			//glNormal3f(-cos(angle), 0, 1);
			//glVertex3f(i, j + step, 0);		// V?rtex P3
			colors.push_back(colorM.r);		colors.push_back(colorM.g);		colors.push_back(colorM.b);		colors.push_back(colorM.a);  // Vector Colors
			normals.push_back(-cos(angle));	normals.push_back(0.0);			normals.push_back(1.0);			// Vector Normals
			textures.push_back(0.0);			textures.push_back(1.0);										// Vector Textures
			vertices.push_back(i);			vertices.push_back(j + step);		vertices.push_back(0.0);	// Vector Vertices

			//glEnd();

			it2++;
		}
		it1++;
	}
	// ----------------------- VAO
	std::vector <int>::size_type nv = vertices.size();	// Tamany del vector vertices en elements.

	// Creaci? d'un VAO i un VBO i c?rrega de la geometria. Guardar identificador VAO identificador VBO a struct CVAO.
	seaVAO = load_TRIANGLES_VAO(vertices, normals, colors, textures);

	seaVAO.nVertexs = (int)nv / 3;
	return seaVAO;
}


// OBJECTE TIE: FETS PER ALUMNES -----------------------------------------------------------------

// Objecte TIE
void tie(GLint shaderId, glm::mat4 MatriuTG, bool sw_mat[5])
{
	Motor(shaderId, MatriuTG, sw_mat);
	Alas(shaderId, MatriuTG, sw_mat);
	Canon(shaderId, MatriuTG, sw_mat);
	// Activar transpar?ncia
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Cuerpo(shaderId, MatriuTG, sw_mat);
	Cabina(shaderId, MatriuTG, sw_mat);
	// Desactivar transpar?ncia
	glDisable(GL_BLEND);
};

void Alas(GLint shaderId, glm::mat4 MatriuTG, bool sw_mat[5])
{
	// Matrius Transformaci?
	glm::mat4 TransMatrix(1.0), ModelMatrix(1.0);
	//glm::mat4 NormalMatrix(1.0);
	CColor col_object;

	// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material.
	col_object.r = 1.0;	col_object.g = 1.0;		col_object.b = 1.0;	 col_object.a = 1.0;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);

	draw_TriEBO_Object(GLU_DISK); //int nvertD = gluDisk_VAO(0.0f, 5.0f, 6, 1);

	// Ala 1 Parte central
	//Parte Exterior
		//glPushMatrix();
		  //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material.
	col_object.r = 1.0;	col_object.g = 1.0;		col_object.b = 1.0;	 col_object.a = 1.0;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	//glTranslatef(25.25f, 0.0f, 0.0f);
	//glRotated(90.0f, 0.0f, 1.0f, 0.0f);
	//glScaled(10.0f, 10.0f, 5.0f);
	TransMatrix = glm::translate(MatriuTG, vec3(25.25f, 0.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(10.0f, 10.0f, 5.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLU_CYLINDER);	//gluCylinder(5.0f, 5.0f, 0.5f, 6, 1);
	//glPopMatrix();

	//Tapas
	//glPushMatrix();
//	  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	  //glTranslatef(25.25f, 0.0f, 0.0f);
	  //glRotated(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScaled(10.0f, 10.0f, 5.0f);
	TransMatrix = glm::translate(MatriuTG, vec3(25.25f, 0.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(10.0f, 10.0f, 5.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLU_DISK);	//gluDisk(0.0f, 5.0f, 6, 1);
	//glPopMatrix();

	//glPushMatrix();
//	  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	  //glTranslatef(27.75f, 0.0f, 0.0f);
	  //glRotated(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScaled(10.0f, 10.0f, 5.0f);
	TransMatrix = glm::translate(MatriuTG, vec3(27.75f, 0.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(10.0f, 10.0f, 5.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLU_DISK);	//gluDisk(0.0f, 5.0f, 6, 1);
	//glPopMatrix();

//Parte Interior
	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	SetColor4d(0.0, 0.0, 0.0, 1.0);
	// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material.
	col_object.r = 0.0;	col_object.g = 0.0;		col_object.b = 0.0;	 col_object.a = 1.0;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	//glTranslated(25.25f, 0.0f, 0.0f);
	//glRotated(90.0f, 0.0f, 1.0f, 0.0f);
	//glScaled(2.0f, 2.0f, 2.0f);
	TransMatrix = glm::translate(MatriuTG, vec3(25.25f, 0.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(2.0f, 2.0f, 2.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER1); // gluCylinder(5.0f, 5.0f, 2.0f, 6, 1);
	//glPopMatrix();

//Tapas
	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	  //glTranslatef(25.25f, 0.0f, 0.0f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScalef(2.0f, 2.0f, 2.0f);
	TransMatrix = glm::translate(MatriuTG, vec3(25.75f, 0.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(2.0f, 2.0f, 2.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);

	draw_TriEBO_Object(GLU_DISK);	//gluDisk(0.0f, 5.0f, 6, 1);
	//glPopMatrix();

	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	  //glTranslatef(29.25f, 0.0f, 0.0f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScalef(2.0f, 2.0f, 2.0f);
	TransMatrix = glm::translate(MatriuTG, vec3(29.25f, 0.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(2.0f, 2.0f, 2.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLU_DISK);	//gluDisk(0.0f, 5.0f, 6, 1);
	//glPopMatrix();

//Decoracion Exterior
	int j;
	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	  //glTranslatef(26.5f, 0.0f, 0.0f);
	  //glScalef(15.75f, 13.75f, 13.75f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
	TransMatrix = glm::translate(MatriuTG, vec3(26.5f, 0.0f, 0.0f));
	TransMatrix = glm::scale(TransMatrix, vec3(15.75f, 13.75f, 13.75f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(30.f), vec3(0.0f, 0.0f, 1.0f));
	for (j = 0; j < 3; j = j + 1)
	{	//glRotatef(60, 0, 0, 1);
		TransMatrix = glm::rotate(TransMatrix, radians(60.f), vec3(0.0f, 0.0f, 1.0f));
		//glPushMatrix();
		   //glScalef(12.7f * 0.583f, 0.1f, 0.3f);
		ModelMatrix = glm::scale(TransMatrix, vec3(12.7f * 0.583f, 0.1f, 0.3f));
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		draw_TriEBO_Object(GLUT_CUBE); //glutSolidCube(1.0f);
		//glPopMatrix();
	}
	//glPopMatrix();

	int u;
	//glPushMatrix();
	//glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	TransMatrix = MatriuTG;
	for (u = 0; u < 6; u = u + 1)
	{	//glRotated(60, 1.0, 0.0, 0.0);
		TransMatrix = glm::rotate(TransMatrix, radians(60.f), vec3(1.0f, 0.0f, 0.0f));
		//glPushMatrix();
			//glTranslatef(26.5f, 0.0f, 43.25f);
			//glRotatef(90, 0.0, 0.0, 1.0);
			//glScalef(10 * 5.155f, 5.0f, 3.0f);
		ModelMatrix = glm::translate(TransMatrix, vec3(26.5f, 0.0f, 43.25f));
		ModelMatrix = glm::rotate(ModelMatrix, radians(90.f), vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::scale(ModelMatrix, vec3(10 * 5.155f, 5.0f, 3.0f));
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		draw_TriEBO_Object(GLUT_CUBE); //glutSolidCube(1.0f);
		//glPopMatrix();
	}
	//glPopMatrix();


//Ala 2 Parte central 
//Parte Exterior
	//glPushMatrix();
	  //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	SetColor4d(1.0, 1.0, 1.0, 1.0);
	// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material.
	col_object.r = 1.0;	col_object.g = 1.0;		col_object.b = 1.0;	 col_object.a = 1.0;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	//glTranslated(-27.75f, 0.0f, 0.0f);
	//glRotated(90.0f, 0.0f, 1.0f, 0.0f);
	//glScaled(10.0, 10.0, 5.0);
	TransMatrix = glm::translate(MatriuTG, vec3(-27.75f, 0.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(10.0f, 10.0f, 5.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLU_CYLINDER); //gluCylinder(5.0f, 5.0f, 0.5f, 6, 1);
	//glPopMatrix();

//Tapas
	//glPushMatrix();
	  //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	  //glTranslatef(-27.75f, 0.0f, 0.0f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScalef(10.0f, 10.0f, 5.0f);
	TransMatrix = glm::translate(MatriuTG, vec3(-27.75f, 0.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(10.0f, 10.0f, 5.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLU_DISK);	//gluDisk(0.0f, 5.0f, 6, 1);
	//glPopMatrix();

	//glPushMatrix();
	  //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	  //glTranslatef(-25.25f, 0.0f, 0.0f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScalef(10.0f, 10.0f, 5.0f);
	TransMatrix = glm::translate(MatriuTG, vec3(-25.25f, 0.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(10.0f, 10.0f, 5.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLU_DISK);	//gluDisk(0.0f, 5.0f, 6, 1);
	//glPopMatrix();

//Parte Interior
	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	  // Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material.
	col_object.r = 0.0;	col_object.g = 0.0;		col_object.b = 0.0;	 col_object.a = 1.0;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	//glTranslatef(-29.25f, 0.0f, 0.0f);
	//glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	//glScalef(2.0f, 2.0f, 2.0f);
	TransMatrix = glm::translate(MatriuTG, vec3(-29.25f, 0.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(2.0f, 2.0f, 2.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER1); // gluCylinder(5.0f, 5.0f, 2.0f, 6, 1);
	//glPopMatrix();

//Tapas
	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	  //glTranslatef(-29.25f, 0.0f, 0.0f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScalef(2.0f, 2.0f, 2.0f);
	TransMatrix = glm::translate(MatriuTG, vec3(-29.25f, 0.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(2.0f, 2.0f, 2.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLU_DISK);	//gluDisk(0.0f, 5.0f, 6, 1);
	//glPopMatrix();

	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	  //SetColor4d(0.0, 0.0, 0.0, 0.0);
	  //glTranslatef(-25.25f, 0.0f, 0.0f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glScalef(2.0f, 2.0f, 2.0f);
	TransMatrix = glm::translate(MatriuTG, vec3(-25.25f, 0.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(2.0f, 2.0f, 2.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLU_DISK);	//gluDisk(0.0f, 5.0f, 6, 1);
	//glPopMatrix();

//Decoracion Exterior
	int w;
	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	  //SetColor4d(0.0, 0.0, 0.0, 0.0);
	  //glTranslatef(-26.5f, 0.0f, 0.0f);
	  //glScalef(15.75f, 13.75f, 13.75f);
	  //glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	  //glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
	TransMatrix = glm::translate(MatriuTG, vec3(-26.5f, 0.0f, 0.0f));
	TransMatrix = glm::scale(TransMatrix, vec3(15.75f, 13.75f, 13.75f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(30.f), vec3(0.0f, 0.0f, 1.0f));
	for (w = 0; w < 3; w = w + 1)
	{	//glRotatef(60.0f, 0.0f, 0.0f, 1.0f);
		TransMatrix = glm::rotate(TransMatrix, radians(60.f), vec3(0.0f, 0.0f, 1.0f));
		//glPushMatrix();
			//glScalef(12.7f * 0.583f, 0.1f, 0.3f);
		ModelMatrix = glm::scale(TransMatrix, vec3(12.7f * 0.583f, 0.1f, 0.3f));
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		draw_TriEBO_Object(GLUT_CUBE); //glutSolidCube(1.0f);
		//glPopMatrix();
	}
	//glPopMatrix();

	int h;
	//glPushMatrix();
	  //glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	  //SetColor4d(0.0, 0.0, 0.0, 0.0);
	TransMatrix = MatriuTG;
	for (h = 0; h < 6; h = h + 1)
	{	//glRotated(60, 1.0, 0.0, 0.0);
		TransMatrix = glm::rotate(TransMatrix, radians(60.f), vec3(1.0f, 0.0f, 0.0f));
		//glPushMatrix();
			//glTranslatef(-26.5f, 0.0f, 43.25f);
			//glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
			//glScalef(10 * 5.155f, 5.0f, 3.0f);
		ModelMatrix = glm::translate(TransMatrix, vec3(-26.5f, 0.0f, 43.25f));
		ModelMatrix = glm::rotate(ModelMatrix, radians(90.f), vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::scale(ModelMatrix, vec3(10 * 5.155f, 5.0f, 3.0f));
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		draw_TriEBO_Object(GLUT_CUBE); //glutSolidCube(1.0f);
		//glPopMatrix();
	}
	//glPopMatrix();
	  //deleteVAO(GLU_DISK);
};


void Motor(GLint shaderId, glm::mat4 MatriuTG, bool sw_mat[5])
{
	// Matrius  
	glm::mat4 TransMatrix(1.0), ModelMatrix(1.0);
	//glm:: mat4 NormalMatrix(1.0);
	CColor col_object;

	//Helices Motor
	int j;
	//glPushMatrix();
	  //glColor4f(0.58f, 0.58f, 0.58f, 0.0f);
// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material.
	col_object.r = 0.58;	col_object.g = 0.58;	col_object.b = 0.58;	 col_object.a = 1.0;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	//glTranslatef(0.0f, -18.0f, 0.0f);
	//glRotated(-90.0f, 0.0f, 0.0f, 1.0f);
	//glRotated(90.0f, 0.0f, 1.0f, 0.0f);
	//glRotatef(45, 0, 0, 1);
	TransMatrix = glm::translate(MatriuTG, vec3(0.0f, -18.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(-90.f), vec3(0.0f, 0.0f, 1.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(45.f), vec3(0.0f, 0.0f, 1.0f));
	for (j = 0; j < 6; j = j + 1)
	{	//glRotatef(30, 0, 0, 1);
		TransMatrix = glm::rotate(TransMatrix, radians(30.f), vec3(0.0f, 0.0f, 1.0f));
		//glPushMatrix();
		//glScalef(12 * 0.583f, 0.1f, 0.3f);
		ModelMatrix = glm::scale(TransMatrix, vec3(12 * 0.583f, 0.1f, 0.3f));
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		draw_TriEBO_Object(GLUT_CUBE); //glutSolidCube(1.0f);
		//glPopMatrix();
	}
	//glPopMatrix();

//Soporte Motor
	//glPushMatrix();
	//glColor4f(0.28f, 0.28f, 0.28f, 0.0f);
	// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material.
	col_object.r = 0.28;	col_object.g = 0.28;	col_object.b = 0.28;	 col_object.a = 1.0;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	//glTranslatef(0.0f, -18.0f, 0.0f);
	//glRotated(-90.0f, 0.0f, 0.0f, 1.0f);
	//glRotated(90.0f, 0.0f, 1.0f, 0.0f);
	TransMatrix = glm::translate(MatriuTG, vec3(0.0f, -18.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(-90.f), vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLUT_TORUS); //glutSolidTorus(1.0, 5.0, 20, 20);
	//glPopMatrix();
};

void Canon(GLint shaderId, glm::mat4 MatriuTG, bool sw_mat[5])
{
	// Matrius Transformaci?
	glm::mat4 TransMatrix(1.0), ModelMatrix(1.0);
	//glm:: mat4 NormalMatrix(1.0);
	CColor col_object;

	//Ca?ones

	// Salida ca?on 1
		//glPushMatrix();
			//glColor4d(0.28, 0.28, 0.28, 0.0);
	// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material.
	col_object.r = 0.28;	col_object.g = 0.28;	col_object.b = 0.28;	 col_object.a = 1.0;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	//glTranslated(5.0, 8.0, -10.0);
	//glRotated(-90.0, 1.0, 0.0, 0.0);
	TransMatrix = glm::translate(MatriuTG, vec3(5.0f, 8.0f, -10.0f));
	ModelMatrix = glm::rotate(TransMatrix, radians(-90.f), vec3(1.0f, 0.0f, 0.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER2); //gluCylinder(1.0f, 0.5f, 5.0f, 60, 1);
	//glPopMatrix();

// Salida ca?on 2
	//glPushMatrix();
	//glColor4d(0.28, 0.28, 0.28, 0.0);
	//glTranslated(-5.0, 8.0, -10.0);
	//glRotated(-90.0, 1.0, 0.0, 0.0);
	TransMatrix = glm::translate(MatriuTG, vec3(-5.0f, 8.0f, -10.0f));
	ModelMatrix = glm::rotate(TransMatrix, radians(-90.f), vec3(1.0f, 0.0f, 0.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER2); // gluCylinder(1.0f, 0.5f, 5.0f, 60, 1);
	//glPopMatrix();

// Ca?on 1
	//glPushMatrix();
		//glColor4d(0.58, 0.58, 0.58, 0.0);
// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material.
	col_object.r = 0.58;	col_object.g = 0.58;		col_object.b = 0.58;	 col_object.a = 1.0;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	//glTranslated(5.0, 10.0, -10.0);
	//glRotated(-90.0, 1.0, 0.0, 0.0);
	TransMatrix = glm::translate(MatriuTG, vec3(5.0f, 10.0f, -10.0f));
	ModelMatrix = glm::rotate(TransMatrix, radians(-90.f), vec3(1.0f, 0.0f, 0.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER3); // gluCylinder(0.35f, 0.35f, 5.0f, 80, 1);
	//glPopMatrix();

//Ca?on 2
	//glPushMatrix();
	//glColor4d(0.58, 0.58, 0.58, 1.0);
	//glTranslated(-5.0, 10.0, -10.0);
	//glRotated(-90.0, 1.0, 0.0, 0.0);
	TransMatrix = glm::translate(MatriuTG, vec3(-5.0f, 10.0f, -10.0f));
	ModelMatrix = glm::rotate(TransMatrix, radians(-90.f), vec3(1.0f, 0.0f, 0.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER3); // gluCylinder(0.35f, 0.35f, 5.0f, 80, 1);
	//glPopMatrix();
}

void Cuerpo(GLint shaderId, glm::mat4 MatriuTG, bool sw_mat[5])
{
	// Matrius Transformaci?
	glm::mat4 TransMatrix(1.0), ModelMatrix(1.0);
	//glm::mat4 NormalMatrix(1.0);
	CColor col_object;

	//Sujeccion de las Alas

	//Lado2
		//glPushMatrix();
		//glColor4d(0.16, 0.16, 0.16, 1.0);
	// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material.
	col_object.r = 0.16;	col_object.g = 0.16;	col_object.b = 0.16;	 col_object.a = 0.5;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	//glRotated(-90.0, 0.0, 1.0, 0.0);
	//glRotated(90.0, 0.0, 0.0, 1.0);
	//glScaled(2.0, 2.0, 2.5);
	TransMatrix = glm::rotate(MatriuTG, radians(-90.f), vec3(0.0f, 1.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(2.0f, 2.0f, 2.5f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER4); // gluCylinder(4.0f, 2.0f, 10.25f, 40, 1);
	//glPopMatrix();

//X2
	//glPushMatrix();
	//glColor4d(0.16, 0.16, 0.16, 1.0);
	//glTranslated(-25.0, 0.0, 0.0);
	//glRotated(90.0, 0.0, 1.0, 0.0);
	TransMatrix = glm::translate(MatriuTG, vec3(-25.0f, 0.0f, 0.0f));
	ModelMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLUT_TORUS); // glutSolidTorus(1.0, 5.0, 20, 20);
	//glPopMatrix();

//Lado 1
	//glPushMatrix();
	//glColor4f(0.16f, 0.16f, 0.16f, 1.0f);
	//SetColor4d(0.16, 0.16, 0.16, 1.0);
	//glRotated(90.0, 0.0, 1.0, 0.0);
	//glRotated(90.0, 0.0, 0.0, 1.0);
	//glScaled(2.0, 2.0, 2.5);
	TransMatrix = glm::rotate(MatriuTG, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(2.0f, 2.0f, 2.5f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER4); // gluCylinder(4.0f, 2.0f, 10.25f, 40, 1);
	//glPopMatrix();

//X1
	//glPushMatrix();
	//glColor4d(0.16, 0.16, 0.16, 1.0);
	//SetColor4d(0.16, 0.16, 0.16, 1.0);
	//glTranslated(25.25, 0.0, 0.0);
	//glRotated(90.0, 0.0, 1.0, 0.0);
	TransMatrix = glm::translate(MatriuTG, vec3(25.25f, 0.0f, 0.0f));
	ModelMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLUT_TORUS); //glutSolidTorus(1.0, 5.0, 20, 20);
	//glPopMatrix();

//Cuerpo
	//glPushMatrix();
	//glColor4d(0.16, 0.16, 0.16, 1.0);
	//SetColor4d(0.16, 0.16, 0.16, 1.0);
	//glScaled(1.75f, 1.75f, 1.5f);
	ModelMatrix = glm::scale(MatriuTG, vec3(1.75f, 1.75f, 1.5f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLU_SPHERE); //gluSphere(10.0f, 80, 80);
	//glPopMatrix();
}

void Cabina(GLint shaderId, glm::mat4 MatriuTG, bool sw_mat[5])
{
	// Matrius Transformaci?
	glm::mat4 TransMatrix(1.0), ModelMatrix(1.0);
	//glm:: mat4 NormalMatrix(1.0);
	CColor col_object;

	// Tapa Cabina
		//glPushMatrix();
		//glColor4d(1.0, 0.058, 0.058, 0.5);
		// Definici? propietats de reflexi? (emissi?, ambient, difusa, especular) del material.
	col_object.r = 1.0;	col_object.g = 0.058;	col_object.b = 0.058;	 col_object.a = 0.5;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);
	//glTranslated(0.0, 19.45, 0.0);
	//glRotated(-90.0, 0.0, 0.0, 1.0);
	//glRotated(90.0, 0.0, 1.0, 0.0);
	//glScaled(1.75, 1.75, 1.75);
	TransMatrix = glm::translate(MatriuTG, vec3(0.0f, 19.45f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(-90.f), vec3(0.0f, 0.0f, 1.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(1.75f, 1.75f, 1.75f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER6); //gluDisk(0.0f, 1.5f, 8, 1);
	//glPopMatrix();

// Cristal Cabina
	//glPushMatrix();
	//glColor4d(1.0, 0.058, 0.058, 0.5);
	//glTranslated(0.0f, 19.45f, 0.0f);
	//glRotated(-90.0f, 0.0f, 0.0f, 1.0f);
	//glRotated(90.0f, 0.0f, 1.0f, 0.0f);
	//glScaled(1.75f, 1.75f, 1.75f);
	TransMatrix = glm::translate(MatriuTG, vec3(0.0f, 19.45f, 0.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(-90.f), vec3(0.0f, 0.0f, 1.0f));
	TransMatrix = glm::rotate(TransMatrix, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	ModelMatrix = glm::scale(TransMatrix, vec3(1.75f, 1.75f, 1.75f));
	// Pas ModelView Matrix a shader
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
	draw_TriEBO_Object(GLUT_USER5); //gluCylinder(1.5f, 4.5f, 2.0f, 8, 1);
	//glPopMatrix();
}
// FI OBJECTE TIE: FETS PER ALUMNES -----------------------------------------------------------------