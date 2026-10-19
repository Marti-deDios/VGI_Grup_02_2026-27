//******** PRACTICA VISUALITZACI� GR�FICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn b�sic VS2026 MULTIFINESTRA amb OpenGL 4.6, interf�cie MFC i llibreries GLM
//******** Ferran Poveda, Marc Vivet, Carme Juli�, D�bora Gil, Enric Mart� G�dia (Setembre 2026)
// escena.cpp : Aqui es on ha d'anar el codi de les funcions que 
//              dibuixin les escenes.
//
//    Versi� 2.0:	- Objectes Cub, Esfera, Tetera (primitives libreria GLUT)
//
//	  Versi� 2.2:	- Objectes Cub, Esfera, Tetera definides en fitxer font glut_geometry amb altres primitives GLUT
//
//	  Versi� 2.5:	- Objectes cubRGB i Tie (nau Star Wars fet per alumnes)
//

#include "pch.h"
#include "material.h"
#include "visualitzacio.h"
#include "escena.h"

// Prototip previ de dibuixa perqu� pugui rebre l'array de textures
void dibuixa(GLuint sh_programID, char obj, glm::mat4 MatriuTG,
	GLuint texturesBoles[16], float posicioBolaBlancaX);

// Dibuixa Eixos Coordenades M�n i Reixes, activant un shader propi.
void dibuixa_Eixos(GLuint ax_programID, bool eix, GLuint axis_Id, CMask3D reixa, CPunt3D hreixa,
	glm::mat4 MatriuProjeccio, glm::mat4 MatriuVista)
{
	// Visualitzaci� Eixos Coordenades M�n
	glUseProgram(ax_programID);

	// Pas Matrius Projecci� i Vista Vista a shader
	glUniformMatrix4fv(glGetUniformLocation(ax_programID, "projectionMatrix"), 1, GL_FALSE, &MatriuProjeccio[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(ax_programID, "viewMatrix"), 1, GL_FALSE, &MatriuVista[0][0]);

	// Attribute Locations must be setup before calling glLinkProgram()
	glBindAttribLocation(ax_programID, 0, "in_Vertex"); // V�rtexs
	glBindAttribLocation(ax_programID, 1, "in_Color");	// Color

	//  Dibuix dels eixos
	if (eix) draw_Eixos(axis_Id);
}

// Dibuixa Skybox en forma de Cub, activant un shader propi.
void dibuixa_Skybox(GLuint sk_programID, GLuint cmTexture, char eix_Polar, glm::mat4 MatriuProjeccio, glm::mat4 MatriuVista)
{
	glm::mat4 ModelMatrix(1.0);

	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content

	// Activaci� shader per a cub skybox
	glUseProgram(sk_programID);

	// Pas Matrius Projecci� i Vista a shader
	glUniformMatrix4fv(glGetUniformLocation(sk_programID, "projectionMatrix"), 1, GL_FALSE, &MatriuProjeccio[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(sk_programID, "viewMatrix"), 1, GL_FALSE, &MatriuVista[0][0]);

	// Rotar skyBox per a orientar sobre eix superior Z o X en Vista Esf�rica (POLARX, POLARY, POLARZ)
	if (eix_Polar == POLARZ) ModelMatrix = glm::rotate(ModelMatrix, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
	else if (eix_Polar == POLARX) ModelMatrix = glm::rotate(ModelMatrix, radians(-90.0f), vec3(0.0f, 0.0f, 1.0f));

	// Escalar Cub Skybox a 5000 per encabir objectes escena a l'interior
	ModelMatrix = glm::scale(ModelMatrix, vec3(5000.0f, 5000.0f, 5000.0f));		//glScaled(5000.0, 5000.0, 5000.0);
	glUniformMatrix4fv(glGetUniformLocation(sk_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);

	// Activar textura cubemaps del Skybox per encabir objectes escena
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cmTexture);

	// Attribute Locations must be setup before calling glLinkProgram()
	glBindAttribLocation(sk_programID, 0, "in_Vertex"); // V�rtexs

	//  Dibuix del Skybox
	drawCubeSkybox();

	glDepthFunc(GL_LESS); // set depth function back to default
}


// dibuixa_EscenaGL: Dibuix de l'escena amb comandes GL
void dibuixa_EscenaGL(GLuint sh_programID, bool eix, GLuint axis_Id, CMask3D reixa, CPunt3D hreixa, char objecte,
	CColor col_object, bool sw_mat[5],
	bool textur, GLuint texturID[NUM_MAX_TEXTURES], bool textur_map, bool flagInvertY,
	int nptsU, CPunt3D PC_u[MAX_PATCH_CORBA], GLfloat pasCS, bool sw_PC, bool dib_TFrenet,
	COBJModel* objecteOBJ,
	glm::mat4 MatriuTG, GLuint texturesBoles[16],
	float posicioBolaBlancaX)
{
	float altfar = 0;
	GLint npunts = 0, nvertexs = 0;
	int i, j, k;
	GLdouble tras[3]; //Sierpinski Sponge
	CColor color_vermell, color_Mar;
	bool sw_material[5];

	// Matrius de Transformaci�
	glm::mat4 ModelMatrix(1.0), TransMatrix(1.0); // ScaleMatrix(1.0), RotMatrix(1.0);
	//glm::mat4 NormalMatrix(1.0);

// VAO
	CVAO objectVAO;
	objectVAO.vaoId = 0;	objectVAO.vboId = 0;	objectVAO.nVertexs = 0;

	tras[0] = 0.0;	tras[1] = 0.0; tras[2] = 0.0;
	color_vermell.r = 1.0;	color_vermell.g = 0.0; color_vermell.b = 0.0; color_vermell.a = 1.0;
	sw_material[0] = false;	sw_material[1] = true; sw_material[2] = true; sw_material[3] = false;	sw_material[4] = true;

	// Shader Visualitzaci� Objectes
	glUseProgram(sh_programID);

	// Parametritzaci� i activaci�/desactivaci� de textures
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
	glBindAttribLocation(sh_programID, 0, "in_Vertex");		// V�rtexs
	glBindAttribLocation(sh_programID, 1, "in_Color");		// Color
	glBindAttribLocation(sh_programID, 2, "in_Normal");		// Normals
	glBindAttribLocation(sh_programID, 3, "in_TexCoord");	// Textura

	// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
	SeleccionaColorMaterial(sh_programID, col_object, sw_mat);

	switch (objecte)
	{

		// Arc
	case ARC:
		// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material pel color de l'objecte.
		SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
		arc(sh_programID, MatriuTG, sw_mat);

		// Activar transpar�ncia
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Dibuix geometria Mar
		color_Mar.r = 0.5;	color_Mar.g = 0.4; color_Mar.b = 0.9; color_Mar.a = 0.5;
		// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material pel color de l'objecte.
		SeleccionaColorMaterial(sh_programID, color_Mar, sw_mat);
		// Pas ModelView Matrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);

		draw_TriVAO_Object(MAR_FRACTAL_VAO);

		// Desactivar transpar�ncia
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

		// Definir caracter�stiques material de cada punt
		SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
		// Objecte OBJ: Dibuix de l'objecte OBJ amb textures amb varis VAO's, un per a cada material.
		objecteOBJ->draw_TriVAO_OBJ(sh_programID);	// Dibuixar VAO a pantalla
		break;

		// Corba Bezier
	case C_BEZIER:
		// Dibuixar Punts de Control
		if (sw_PC)
		{	// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material pel color vermell.
			SeleccionaColorMaterial(sh_programID, color_vermell, sw_material);

			for (int i = 0; i < nptsU; i++)
			{
				ModelMatrix = glm::translate(MatriuTG, vec3(PC_u[i].x, PC_u[i].y, PC_u[i].z));
				// Pas ModelViewMatrix a shader
				glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
				draw_TriEBO_Object(GLU_SPHERE);	//draw_TriVAO_Object(GLU_SPHERE); //gluSphere(5.0, 20, 20);
			}

		}
		// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material pel color de l'objecte.
		SeleccionaColorMaterial(sh_programID, col_object, sw_mat);
		ModelMatrix = MatriuTG;
		// Pas ModelViewMatrix a shader
		glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
		draw_LinEBO_Object(CRV_BEZIER); //draw_LinVAO_Object(CRV_BEZIER); //draw_Bezier_Curve(nptsU, PC_u, pasCS, false);
		if (dib_TFrenet) draw_TFBezier_Curve(sh_programID, PC_u, nptsU, pasCS, false); // Dibuixar Triedre de Frenet
		break;

		// Corba Lemniscata
	case C_LEMNISCATA:
		// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material pel color de l'objecte.
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
		{	// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material pel color vermell.
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
		{	// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material pel color vermell.
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
		{	// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material pel color vermell.
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

		// Matriu de Primitives SENSE pre-compilaci� pr�via en VBO (precompilaci�, draw i alliberaci� VBO en una funci�)
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

		// Matriu de Primitives AMB pre-compilaci� pr�via en VBO 
		//	(separaci� en 3 funcions: *_VBO() per precompilaci�, draw*() per dibuix i deleteVBO() per alliberar VBO)
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
		dibuixa(sh_programID, objecte, MatriuTG, texturesBoles, posicioBolaBlancaX);
		break;
	}

	// Enviar les comandes gr�fiques a pantalla
	//	glFlush();
}


// dibuixa: Funci� que dibuixa objectes simples de la llibreria GLUT segons obj
void dibuixa(GLuint sh_programID, char obj, glm::mat4 MatriuTG, GLuint texturesBoles[16], float posicioBolaBlancaX)
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
		verdCoixi.r = 0.0f; verdCoixi.g = 0.5f; verdCoixi.b = 0.12f; verdCoixi.a = 1.0f; // Verd una mica més fosc
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

		// 2.3. Cantonades i proteccions metàl·liques (ajustades per tancar bé els laterals)
		CColor plata;
		plata.r = 0.82f; plata.g = 0.82f; plata.b = 0.82f; plata.a = 1.0f;
		bool materialMetal[5] = { false, true, true, true, true };
		SeleccionaColorMaterial(sh_programID, plata, materialMetal);

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

		// 2.4. Els 6 forats negres (ben alineats amb les noves mides)
		CColor negre;
		negre.r = 0.02f; negre.g = 0.02f; negre.b = 0.02f; negre.a = 1.0f;
		SeleccionaColorMaterial(sh_programID, negre, materialTapet);

		glm::vec3 posForats[6] = {
			glm::vec3(-4.85f,  2.4f, 0.16f), // Cantonada Superior Esquerra
			glm::vec3(4.85f,  2.4f, 0.16f), // Cantonada Superior Dreta
			glm::vec3(-4.85f, -2.4f, 0.16f), // Cantonada Inferior Esquerra
			glm::vec3(4.85f, -2.4f, 0.16f), // Cantonada Inferior Dreta
			glm::vec3(0.0f,   2.55f, 0.16f), // Centre Superior (ajustat a la nova posició)
			glm::vec3(0.0f,  -2.55f, 0.16f)  // Centre Inferior (ajustat a la nova posició)
		};

		for (int i = 0; i < 6; i++) {
			ModelMatrix = glm::translate(MatriuTG, posForats[i]);
			ModelMatrix = glm::scale(ModelMatrix, vec3(0.52f, 0.52f, 0.03f));
			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
			draw_TriEBO_Object(GLUT_SPHERE);
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

		bool materialBola[5] = { false, true, true, true, true };
		float escalaBola = 0.55f;
		float alturaTapetZ = 0.25f;

		// ACTIVAR TEXTURES PER A LES BOLES
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_TRUE);
		glUniform1i(glGetUniformLocation(sh_programID, "texturaActiva"), 1);
		glUniform1i(glGetUniformLocation(sh_programID, "modulate"), GL_TRUE);

		// 4. Definici de la Bola Blanca (Cue Ball)
		{
			CColor blanc;
			blanc.r = 1.0f; blanc.g = 1.0f; blanc.b = 1.0f; blanc.a = 1.0f;
			SeleccionaColorMaterial(sh_programID, blanc, materialBola);

			glBindTexture(GL_TEXTURE_2D, texturesBoles[0]);

			// Dibuixem la bola blanca a la seva posicio actual
			glm::mat4 ModelMatrixBola = glm::translate(MatriuTG,
				vec3(posicioBolaBlancaX, 0.0f, alturaTapetZ));

			ModelMatrixBola = glm::rotate(ModelMatrixBola, radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
			ModelMatrixBola = glm::rotate(ModelMatrixBola, radians(180.0f), vec3(1.0f, 0.0f, 0.0f));
			ModelMatrixBola = glm::scale(ModelMatrixBola, vec3(escalaBola, escalaBola, escalaBola));

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

		float startX = 2.6f;
		float startY = 0.0f;
		float dX = escalaBola * 0.4f;
		float dY = escalaBola * 0.5f;

		int index = 0;
		for (int fila = 0; fila < 5; ++fila) {
			float posX = startX + (fila * dX);
			float posYIni = startY + (fila * (dY * 0.5f));

			for (int b = 0; b <= fila; ++b) {
				float posY = posYIni - (b * dY);
				int numBola = ordenRack[index++];

				CColor col;
				col.r = 1.0f; col.g = 1.0f; col.b = 1.0f; col.a = 1.0f;
				SeleccionaColorMaterial(sh_programID, col, materialBola);

				glBindTexture(GL_TEXTURE_2D, texturesBoles[numBola]);

				glm::mat4 ModelMatrixBola = glm::translate(MatriuTG, vec3(posX, posY, alturaTapetZ));
				ModelMatrixBola = glm::rotate(ModelMatrixBola, radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
				ModelMatrixBola = glm::rotate(ModelMatrixBola, radians(180.0f), vec3(1.0f, 0.0f, 0.0f));
				ModelMatrixBola = glm::scale(ModelMatrixBola, vec3(escalaBola, escalaBola, escalaBola));

				glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrixBola[0][0]);
				draw_TriEBO_Object(GLU_SPHERE);
			}
		}

		glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);
		glUniform1i(glGetUniformLocation(sh_programID, "texturaActiva"), 0);

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
	std::vector <double> vertices, normals, colors, textures;	// Definici� vectors din�mics per a vertexs, normals i textures 
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
			//glVertex3f(i, j, 0);			// V�rtex P1
			colors.push_back(colorM.r);		colors.push_back(colorM.g);		colors.push_back(colorM.b);		colors.push_back(colorM.a);  // Vector Colors
			normals.push_back(Nx);			normals.push_back(0.0);			normals.push_back(1.0);			// Vector Normals
			textures.push_back(0.0);			textures.push_back(0.0);										// Vector Textures
			vertices.push_back(i);			vertices.push_back(j);			vertices.push_back(0.0);		// Vector Vertices

			angle = 1.0 * (it2 + 1.0) * h * 15;
			Nx = -cos(angle);
			//glNormal3f(-cos(angle), 0, 1);
			//glVertex3f(i + step, j, 0);		// V�rtex P2
			colors.push_back(colorM.r);		colors.push_back(colorM.g);		colors.push_back(colorM.b);		colors.push_back(colorM.a);  // Vector Colors
			normals.push_back(Nx);			normals.push_back(0.0);			normals.push_back(1.0);			// Vector Normals
			textures.push_back(1.0);			textures.push_back(0.0);										// Vector Textures
			vertices.push_back(i + step);		vertices.push_back(j);			vertices.push_back(0.0);		// Vector Vertices

			angle = 1.0 * (it2 + 1.0) * h * 15;
			Nx = -cos(angle);
			//glNormal3f(-cos(angle), 0, 1);
			//glVertex3f(i + step, j + step, 0);// V�rtex P3
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
			//glVertex3f(i, j, 0);			// V�rtex P1
			colors.push_back(colorM.r);		colors.push_back(colorM.g);		colors.push_back(colorM.b);		colors.push_back(colorM.a);  // Vector Colors
			normals.push_back(-cos(angle));	normals.push_back(0.0);			normals.push_back(1.0);			// Vector Normals
			textures.push_back(1.0);			textures.push_back(1.0);										// Vector Textures
			vertices.push_back(i);			vertices.push_back(j);			vertices.push_back(0.0);		// Vector Vertices

			angle = 1.0 * (it2 + 1) * h * 15;
			//glNormal3f(-cos(angle), 0, 1);
			//glVertex3f(i + step, j + step, 0);// V�rtex P2
			colors.push_back(colorM.r);		colors.push_back(colorM.g);		colors.push_back(colorM.b);		colors.push_back(colorM.a);  // Vector Colors
			normals.push_back(-cos(angle));	normals.push_back(0.0);			normals.push_back(1.0);			// Vector Normals
			textures.push_back(1.0);			textures.push_back(1.0);										// Vector Textures
			vertices.push_back(i + step);		vertices.push_back(j + step);	vertices.push_back(0.0);		// Vector Vertices

			angle = 1.0 * it2 * h * 15;
			//glNormal3f(-cos(angle), 0, 1);
			//glVertex3f(i, j + step, 0);		// V�rtex P3
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

	// Creaci� d'un VAO i un VBO i c�rrega de la geometria. Guardar identificador VAO identificador VBO a struct CVAO.
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
	// Activar transpar�ncia
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Cuerpo(shaderId, MatriuTG, sw_mat);
	Cabina(shaderId, MatriuTG, sw_mat);
	// Desactivar transpar�ncia
	glDisable(GL_BLEND);
};

void Alas(GLint shaderId, glm::mat4 MatriuTG, bool sw_mat[5])
{
	// Matrius Transformaci�
	glm::mat4 TransMatrix(1.0), ModelMatrix(1.0);
	//glm::mat4 NormalMatrix(1.0);
	CColor col_object;

	// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
	col_object.r = 1.0;	col_object.g = 1.0;		col_object.b = 1.0;	 col_object.a = 1.0;
	SeleccionaColorMaterial(shaderId, col_object, sw_mat);

	draw_TriEBO_Object(GLU_DISK); //int nvertD = gluDisk_VAO(0.0f, 5.0f, 6, 1);

	// Ala 1 Parte central
	//Parte Exterior
		//glPushMatrix();
		  //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
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
	// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
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
	// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
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
	  // Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
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
// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
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
	// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
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
	// Matrius Transformaci�
	glm::mat4 TransMatrix(1.0), ModelMatrix(1.0);
	//glm:: mat4 NormalMatrix(1.0);
	CColor col_object;

	//Ca�ones

	// Salida ca�on 1
		//glPushMatrix();
			//glColor4d(0.28, 0.28, 0.28, 0.0);
	// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
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

// Salida ca�on 2
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

// Ca�on 1
	//glPushMatrix();
		//glColor4d(0.58, 0.58, 0.58, 0.0);
// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
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

//Ca�on 2
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
	// Matrius Transformaci�
	glm::mat4 TransMatrix(1.0), ModelMatrix(1.0);
	//glm::mat4 NormalMatrix(1.0);
	CColor col_object;

	//Sujeccion de las Alas

	//Lado2
		//glPushMatrix();
		//glColor4d(0.16, 0.16, 0.16, 1.0);
	// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
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
	// Matrius Transformaci�
	glm::mat4 TransMatrix(1.0), ModelMatrix(1.0);
	//glm:: mat4 NormalMatrix(1.0);
	CColor col_object;

	// Tapa Cabina
		//glPushMatrix();
		//glColor4d(1.0, 0.058, 0.058, 0.5);
		// Definici� propietats de reflexi� (emissi�, ambient, difusa, especular) del material.
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