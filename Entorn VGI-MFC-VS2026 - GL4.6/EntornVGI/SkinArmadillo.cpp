// SkinArmadillo.cpp : Skin "Armadillo" per a la bola blanca (veure SkinArmadillo.h)

#include "pch.h"
#include "SkinArmadillo.h"
#include "constants.h"
#include "material.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace
{
	// ---- Ajustos (editeu-los aqui) ----
	const char* const CARPETA = ".\\objectes\\skins\\armadillo\\";
	const int   MAX_FRAMES = 200;			// Maxim de frames d'una seqüencia
	const float FPS_IDLE = 12.0f;			// Velocitat de l'animacio d'espera quan es una seqüencia d'OBJ
	const float PERIODE_RESPIRACIO = 2.6f;	// Segons per respiracio (nomes quan l'idle es un sol OBJ)

	// Enrotllament PROCEDURAL (nomes quan no hi ha seqüencia armadillo_curl_xxx.obj):
	const float CENTRE_IDLE_Z = 0.62f;		// Altura (en unitats de bola, radi=1) del centre de massa de l'armadillo dret
	const float PLEC_CANVI_BOLA = 0.55f;	// A partir d'aquest grau d'enrotllament es mostra la bola

	float Suavitzat(float a, float b, float x)
	{
		if (b <= a) return x >= b ? 1.0f : 0.0f;
		float t = (x - a) / (b - a);
		t = (std::max)(0.0f, (std::min)(1.0f, t));
		return t * t * (3.0f - 2.0f * t);
	}
}

/* ------------------------------------------------------------------------- */
/*                                  CARREGA                                  */
/* ------------------------------------------------------------------------- */

bool SkinArmadillo::Existeix(const std::string& cami)
{
	return ::GetFileAttributesA(cami.c_str()) != INVALID_FILE_ATTRIBUTES;
}

COBJModel* SkinArmadillo::CarregaModel(const std::string& cami)
{
	if (!Existeix(cami)) return NULL;
	COBJModel* model = ::new COBJModel;
	std::vector<char> ruta(cami.begin(), cami.end());
	ruta.push_back('\0');
	if (model->LoadModel(&ruta[0]) != 0)
	{
		::delete model;
		return NULL;
	}
	return model;
}

// Carrega base_000.obj, base_001.obj, ... fins que en falti un. Retorna fals si no n'hi ha cap.
bool SkinArmadillo::CarregaSequencia(const std::string& base, std::vector<COBJModel*>& frames)
{
	for (int i = 0; i < MAX_FRAMES; ++i)
	{
		char sufix[32];
		sprintf_s(sufix, "_%03d.obj", i);
		COBJModel* frame = CarregaModel(std::string(CARPETA) + base + sufix);
		if (frame == NULL) break;
		frames.push_back(frame);
	}
	return !frames.empty();
}

void SkinArmadillo::AlliberaFrames(std::vector<COBJModel*>& frames)
{
	for (size_t i = 0; i < frames.size(); ++i) ::delete frames[i];
	frames.clear();
}

bool SkinArmadillo::Carrega()
{
	Allibera();
	const std::string carpeta = CARPETA;

	// Idle: seqüencia animada si existeix, o si no un sol OBJ estatic.
	if (!CarregaSequencia("armadillo_idle", m_idle))
	{
		COBJModel* estatic = CarregaModel(carpeta + "armadillo_idle.obj");
		if (estatic == NULL)
		{
			OutputDebugStringA("VGI: skin Armadillo: falta objectes\\skins\\armadillo\\armadillo_idle.obj\n");
			Allibera();
			return false;
		}
		m_idle.push_back(estatic);
	}

	// Curl: opcional. Si no hi ha seqüencia, l'enrotllament es fa amb codi.
	CarregaSequencia("armadillo_curl", m_curl);

	m_ball = CarregaModel(carpeta + "armadillo_ball.obj");
	if (m_ball == NULL)
	{
		OutputDebugStringA("VGI: skin Armadillo: falta objectes\\skins\\armadillo\\armadillo_ball.obj\n");
		Allibera();
		return false;
	}
	return true;
}

void SkinArmadillo::Allibera()
{
	AlliberaFrames(m_idle);
	AlliberaFrames(m_curl);
	if (m_ball != NULL) { ::delete m_ball; m_ball = NULL; }
}

/* ------------------------------------------------------------------------- */
/*                                   DIBUIX                                  */
/* ------------------------------------------------------------------------- */

COBJModel* SkinArmadillo::FrameIdle(float tempsIdle) const
{
	if (m_idle.empty()) return NULL;
	if (m_idle.size() == 1) return m_idle[0];
	const int n = (int)m_idle.size();
	const int frame = (int)(tempsIdle * FPS_IDLE) % n;
	return m_idle[frame < 0 ? 0 : frame];
}

void SkinArmadillo::DibuixaModel(GLuint sh_programID, COBJModel* model, const glm::mat4& matriu) const
{
	if (model == NULL) return;
	glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &matriu[0][0]);
	model->draw_TriVAO_OBJ(sh_programID);		// Un VAO per a cada material del .mtl
}

void SkinArmadillo::Dibuixa(GLuint sh_programID, const EstatVisualSkin& ev, const ContextVisualBlanca& c)
{
	const float plec = (std::max)(0.0f, (std::min)(1.0f, ev.plec));

	// ---- Orientacio: mira cap on apunta el tac; en enrotllar-se passa a l'orientacio fisica de la bola ----
	const glm::quat qApuntat = glm::angleAxis(c.angleApuntat, glm::vec3(0.0f, 0.0f, 1.0f));
	const glm::quat qFisica = glm::normalize(glm::quat_cast(glm::mat3(c.rotacioFisica)));
	const float mescla = Suavitzat(PLEC_CANVI_BOLA, 1.0f, plec);
	const glm::quat q = (mescla >= 1.0f) ? qFisica : glm::normalize(glm::slerp(qApuntat, qFisica, mescla));

	// Els models estan fets amb la bola de radi 1: s'escalen al radi visual de la bola (el collider no es toca).
	glm::mat4 base = glm::translate(c.matriuBase, c.centre) * glm::mat4_cast(q);
	base = glm::scale(base, glm::vec3(c.radi, c.radi, c.radi));

	// ---- Estat OpenGL: materials del .mtl, sense textura ----
	CColor blanc;
	blanc.r = 1.0f; blanc.g = 1.0f; blanc.b = 1.0f; blanc.a = 1.0f;
	bool materialSkin[5] = { false, true, true, true, true };
	SeleccionaColorMaterial(sh_programID, blanc, materialSkin);
	glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_FALSE);

	if (plec >= 1.0f)
	{
		// BallReady / Rolling: es una bola; gira amb la rotacio de la fisica (ja inclosa a q).
		DibuixaModel(sh_programID, m_ball, base);
	}
	else if (plec <= 0.0f)
	{
		// Idle: armadillo dret. Si l'idle es un sol OBJ, una respiracio suau (escalat des dels peus).
		glm::mat4 m = base;
		if (m_idle.size() <= 1)
		{
			const float resp = 1.0f + 0.018f * std::sin(6.2831853f * ev.tempsIdle / PERIODE_RESPIRACIO);
			m = glm::translate(m, glm::vec3(0.0f, 0.0f, -1.0f));
			m = glm::scale(m, glm::vec3(1.0f, 1.0f, resp));
			m = glm::translate(m, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		DibuixaModel(sh_programID, FrameIdle(ev.tempsIdle), m);
	}
	else if (!m_curl.empty())
	{
		// Animacio d'enrotllar de l'artista: el frame segueix el grau d'enrotllament (i al reves en desenrotllar).
		const int n = (int)m_curl.size();
		int idx = (int)(plec * (float)(n - 1) + 0.5f);
		idx = (std::max)(0, (std::min)(n - 1, idx));
		DibuixaModel(sh_programID, m_curl[idx], base);
	}
	else if (plec < PLEC_CANVI_BOLA)
	{
		// Enrotllament PROCEDURAL, fase 1: l'armadillo dret es deixa caure endavant i s'encongeix cap al centre.
		const float e = Suavitzat(0.0f, PLEC_CANVI_BOLA, plec);
		glm::mat4 m = glm::translate(base, glm::vec3(0.0f, 0.0f, CENTRE_IDLE_Z * (1.0f - e)));
		m = glm::rotate(m, glm::radians(75.0f) * e, glm::vec3(0.0f, 1.0f, 0.0f));		// cap a +X
		m = glm::scale(m, glm::vec3(1.0f + 0.20f * e, 1.0f + 0.20f * e, 1.0f - 0.55f * e));
		m = glm::translate(m, glm::vec3(0.0f, 0.0f, -CENTRE_IDLE_Z));
		DibuixaModel(sh_programID, FrameIdle(0.0f), m);
	}
	else
	{
		// Fase 2: ja es la bola, que "rebota" una mica en tancar-se.
		const float u = (plec - PLEC_CANVI_BOLA) / (1.0f - PLEC_CANVI_BOLA);
		const float esc = 0.80f + 0.20f * u + 0.10f * std::sin(3.14159265f * u);
		DibuixaModel(sh_programID, m_ball, glm::scale(base, glm::vec3(esc, esc, esc)));
	}

	// L'escena continua dibuixant les altres boles amb textura.
	glUniform1i(glGetUniformLocation(sh_programID, "textur"), GL_TRUE);
}
