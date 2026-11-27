// SkinArmadillo.h : Skin "Armadillo" per a la bola blanca (veure SkinBolaBlanca.h)
//
//   FITXERS (carpeta objectes\skins\armadillo\):
//     armadillo_idle.obj        OBLIGATORI  Armadillo dret (forma normal).
//     armadillo_ball.obj        OBLIGATORI  Armadillo enrotllat (bola de radi 1).
//     armadillo.mtl             Materials (Ka, Kd, Ks, Ns).
//
//     OPCIONALS (millora l'animacio; si no hi son, es fa amb codi a partir dels dos OBJ anteriors):
//     armadillo_idle_000.obj, _001.obj, ...   Animacio d'espera en bucle (substitueix armadillo_idle.obj).
//     armadillo_curl_000.obj, _001.obj, ...   Animacio d'enrotllar-se. El primer frame ha de coincidir amb
//                                             l'armadillo dret i l'ultim amb armadillo_ball.obj.
//                                             El desenrotllar es la mateixa animacio reproduida al reves.
//
//   CONVENCIO DELS MODELS (tots els OBJ):
//     - Z amunt, nas cap a +X, origen al centre de la bola, bola de RADI 1.0.
//     - Els peus de l'armadillo dret toquen el terra a Z = -1.0 (el mateix punt de contacte que la bola).
//     - Mateixos noms de material a tots els fitxers.

#pragma once

#include "SkinBolaBlanca.h"
#include "objLoader.h"
#include <string>
#include <vector>

class SkinArmadillo : public ISkinBlanca
{
public:
	SkinArmadillo() {}
	~SkinArmadillo() {}		// No toca OpenGL: es neteja a Allibera()

	const char* Nom() const override { return "Armadillo"; }
	bool Carrega() override;
	void Allibera() override;
	float DuradaEnrotllat() const override { return 0.22f; }
	float DuradaDesenrotllat() const override { return 0.30f; }
	void Dibuixa(GLuint sh_programID, const EstatVisualSkin& estat, const ContextVisualBlanca& ctx) override;

private:
	static bool Existeix(const std::string& cami);
	static COBJModel* CarregaModel(const std::string& cami);
	static bool CarregaSequencia(const std::string& base, std::vector<COBJModel*>& frames);
	static void AlliberaFrames(std::vector<COBJModel*>& frames);

	COBJModel* FrameIdle(float tempsIdle) const;
	void DibuixaModel(GLuint sh_programID, COBJModel* model, const glm::mat4& matriu) const;

	std::vector<COBJModel*> m_idle;		// 1 frame (estatic) o N (animacio d'espera en bucle)
	std::vector<COBJModel*> m_curl;		// buit = enrotllament procedural; N frames = animacio de l'artista
	COBJModel* m_ball = NULL;
};
