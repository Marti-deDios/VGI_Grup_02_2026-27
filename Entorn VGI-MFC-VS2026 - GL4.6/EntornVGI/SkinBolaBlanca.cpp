// SkinBolaBlanca.cpp : Maquina d'estats, registre i skin classica (veure SkinBolaBlanca.h)

#include "pch.h"
#include "SkinBolaBlanca.h"
#include "SkinArmadillo.h"
#include "constants.h"
#include "material.h"
#include "glut_geometry.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

/* ------------------------------------------------------------------------- */
/*   SKIN CLASSICA: la bola texturada de sempre (identica a la que dibuixava   */
/*   escena.cpp abans de tenir skins). Es la skin de reserva.                */
/* ------------------------------------------------------------------------- */
namespace
{
	class SkinClassica : public ISkinBlanca
	{
	public:
		const char* Nom() const override { return "Classica"; }
		bool Carrega() override { return true; }

		void Dibuixa(GLuint sh_programID, const EstatVisualSkin&, const ContextVisualBlanca& ctx) override
		{
			CColor blanc;
			blanc.r = 1.0f; blanc.g = 1.0f; blanc.b = 1.0f; blanc.a = 1.0f;
			bool materialBola[5] = { false, true, true, true, true };
			SeleccionaColorMaterial(sh_programID, blanc, materialBola);

			glBindTexture(GL_TEXTURE_2D, ctx.textura);

			// Mateix ordre de transformacions que el codi original de la bola blanca.
			glm::mat4 model = glm::translate(ctx.matriuBase, ctx.centre);
			model = model * ctx.rotacioFisica;
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			const float escala = ctx.radi / 0.2f;		// L'esfera unitat del joc te radi 0.2
			model = glm::scale(model, glm::vec3(escala, escala, escala));

			glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &model[0][0]);
			draw_TriEBO_Object(GLU_SPHERE);
		}
	};

	/* ---------------------------------------------------------------------- */
	/*   REGISTRE DE SKINS.  Per afegir-ne una: crear la classe i afegir una   */
	/*   linia aqui. L'index 0 ha de ser sempre la classica (skin de reserva). */
	/* ---------------------------------------------------------------------- */
	ISkinBlanca* CreaClassica() { return new SkinClassica(); }
	ISkinBlanca* CreaArmadillo() { return new SkinArmadillo(); }

	struct EntradaRegistre
	{
		const char* nom;
		ISkinBlanca* (*crea)();
	};

	const EntradaRegistre REGISTRE[] =
	{
		{ "Classica",  CreaClassica  },
		{ "Armadillo", CreaArmadillo },
	};
	const int N_SKINS = (int)(sizeof(REGISTRE) / sizeof(REGISTRE[0]));
}

/* ------------------------------------------------------------------------- */
/*                        CONTROLADOR (MAQUINA D'ESTATS)                     */
/* ------------------------------------------------------------------------- */

ControladorSkinBlanca::ControladorSkinBlanca()
	: m_skins(N_SKINS, (ISkinBlanca*)NULL), m_estatCarrega(N_SKINS, 0)
{
}

int ControladorSkinBlanca::NumSkins() { return N_SKINS; }

const char* ControladorSkinBlanca::NomSkin(int index)
{
	return (index >= 0 && index < N_SKINS) ? REGISTRE[index].nom : "";
}

bool ControladorSkinBlanca::SeleccionaSkin(int index)
{
	if (index < 0 || index >= N_SKINS) return false;
	m_actual = index;
	Reinicia();
	return true;
}

ISkinBlanca* ControladorSkinBlanca::ObteSkin(int index)
{
	if (index < 0 || index >= N_SKINS) return NULL;
	if (m_estatCarrega[index] == 0)
	{
		ISkinBlanca* skin = REGISTRE[index].crea();
		if (skin->Carrega())
		{
			m_skins[index] = skin;
			m_estatCarrega[index] = 1;
		}
		else
		{
			skin->Allibera();
			delete skin;
			m_estatCarrega[index] = -1;
			OutputDebugStringA("VGI: no s'ha pogut carregar una skin de la bola blanca; s'usa la classica.\n");
		}
	}
	return (m_estatCarrega[index] == 1) ? m_skins[index] : NULL;
}

ISkinBlanca* ControladorSkinBlanca::SkinActiva()
{
	ISkinBlanca* skin = ObteSkin(m_actual);
	if (skin == NULL && m_actual != 0) skin = ObteSkin(0);		// Reserva: la classica
	return skin;
}

bool ControladorSkinBlanca::Carrega()
{
	return SkinActiva() != NULL;
}

void ControladorSkinBlanca::Allibera()
{
	for (size_t i = 0; i < m_skins.size(); ++i)
	{
		if (m_skins[i] != NULL)
		{
			m_skins[i]->Allibera();
			delete m_skins[i];
			m_skins[i] = NULL;
		}
		m_estatCarrega[i] = 0;
	}
}

void ControladorSkinBlanca::Entra(EstatSkinBlanca nou)
{
	m_estat = nou;
	m_tempsEstat = 0.0f;
}

void ControladorSkinBlanca::Reinicia()
{
	m_estat = EstatSkinBlanca::Idle;
	m_plec = 0.0f;
	m_tempsEstat = 0.0f;
}

void ControladorSkinBlanca::TickIdle(float dt)
{
	if (dt > 0.0f) m_tempsIdle += dt;
}

void ControladorSkinBlanca::Actualitza(const IntencioSkinBlanca& in, float dt)
{
	if (dt < 0.0f) dt = 0.0f;
	m_tempsIdle += dt;

	// Bola embocada: l'skin es queda tal com estava (s'enfonsa a la tronera) fins que
	// la bola es recol.loqui o es reiniciï la partida, que crida Reinicia().
	if (!in.blancaViva) return;

	m_tempsEstat += dt;

	ISkinBlanca* skin = SkinActiva();
	const float durEnrotlla = (std::max)(0.01f, skin != NULL ? skin->DuradaEnrotllat() : 0.22f);
	const float durDesenrotlla = (std::max)(0.01f, skin != NULL ? skin->DuradaDesenrotllat() : 0.30f);

	// "Apuntar" = el jugador manté premut el botó/espai, o el tac ja ha sortit i encara no ha colpejat.
	const bool apuntar = in.apuntant || in.tirEnProces;

	switch (m_estat)
	{
	case EstatSkinBlanca::Idle:
		if (in.rodant) { m_plec = 1.0f; Entra(EstatSkinBlanca::Rolling); }		// Seguretat: tir sense passar per apuntar
		else if (apuntar) Entra(EstatSkinBlanca::Curling);
		break;

	case EstatSkinBlanca::Curling:
		if (in.rodant) { m_plec = 1.0f; Entra(EstatSkinBlanca::Rolling); break; }	// Cop molt rapid: ja es bola
		if (!apuntar) { Entra(EstatSkinBlanca::Uncurling); break; }					// Apuntat cancel.lat
		m_plec += dt / durEnrotlla;
		if (m_plec >= 1.0f) { m_plec = 1.0f; Entra(EstatSkinBlanca::BallReady); }
		break;

	case EstatSkinBlanca::BallReady:
		if (in.rodant) Entra(EstatSkinBlanca::Rolling);
		else if (!apuntar) Entra(EstatSkinBlanca::Uncurling);						// Apuntat cancel.lat
		break;

	case EstatSkinBlanca::Rolling:
		if (!in.rodant) Entra(EstatSkinBlanca::Uncurling);							// El torn s'ha resolt
		break;

	case EstatSkinBlanca::Uncurling:
		if (in.rodant) { m_plec = 1.0f; Entra(EstatSkinBlanca::Rolling); break; }
		if (apuntar) { Entra(EstatSkinBlanca::Curling); break; }					// Torna a apuntar: s'enrotlla des d'on era
		m_plec -= dt / durDesenrotlla;
		if (m_plec <= 0.0f) { m_plec = 0.0f; Entra(EstatSkinBlanca::Idle); }
		break;
	}
}

bool ControladorSkinBlanca::Dibuixa(GLuint sh_programID, const ContextVisualBlanca& ctx)
{
	ISkinBlanca* skin = SkinActiva();
	if (skin == NULL) return false;

	EstatVisualSkin ev;
	ev.estat = m_estat;
	ev.plec = m_plec;
	ev.tempsIdle = m_tempsIdle;
	ev.tempsEstat = m_tempsEstat;
	skin->Dibuixa(sh_programID, ev, ctx);
	return true;
}
