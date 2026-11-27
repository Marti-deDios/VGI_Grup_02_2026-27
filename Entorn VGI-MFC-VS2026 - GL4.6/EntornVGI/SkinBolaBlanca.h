// SkinBolaBlanca.h : Sistema de "skins" visuals per a la bola blanca
//
//   PRINCIPI: la FISICA no coneix les skins. El collider de la bola (radi, velocitat, xocs, troneres)
//   no es toca mai; una skin nomes decideix COM ES DIBUIXA la bola blanca.
//
//   Peces:
//     - ISkinBlanca              Interficie que implementa cada skin (Classica, Armadillo, ...).
//     - ControladorSkinBlanca    Maquina d'estats Idle / Curling / BallReady / Rolling (+ Uncurling)
//                                i registre d'skins. El View li passa cada fotograma una
//                                IntencioSkinBlanca derivada dels flags que ja existeixen
//                                (carregantTir, animantTir, tirAplicat, tirEnCurs...).
//     - ContextVisualBlanca      Dades de nomes lectura que la skin necessita per dibuixar.
//
//   COM AFEGIR UNA SKIN NOVA:
//     1. Crear una classe que hereti d'ISkinBlanca (veure SkinArmadillo.h).
//     2. Afegir-ne una linia al REGISTRE de SkinBolaBlanca.cpp.
//
//   IMPORTANT: Carrega(), Dibuixa() i Allibera() s'han de cridar amb el context OpenGL actiu.

#pragma once

#include <gl/glew.h>
#include <glm/glm.hpp>
#include <vector>

// Estats visuals de la bola blanca.
//   Idle      : l'skin esta en la seva forma normal i fa l'animacio d'espera.
//   Curling   : el jugador comença a apuntar -> l'skin s'enrotlla.
//   BallReady : ja es una bola, esperant el cop.
//   Rolling   : la bola ha estat colpejada i roda amb la fisica.
//   Uncurling : (transitori) es desenrotlla en cancel.lar l'apuntat o en acabar el torn.
enum class EstatSkinBlanca { Idle, Curling, BallReady, Rolling, Uncurling };

// Lo que passa al joc en aquest fotograma. El View ho calcula nomes llegint els seus flags.
struct IntencioSkinBlanca
{
	bool blancaViva = true;		// bolaBlancaActiva (fisica). Si es fals (embocada) l'skin queda congelada.
	bool apuntant = false;		// carregantTir || m_carregantEspai
	bool tirEnProces = false;	// animantTir && !tirAplicat: el tac ja ha sortit pero encara no ha colpejat
	bool rodant = false;		// tirAplicat && torn en curs: bola colpejada, torn encara sense resoldre
};

// Estat visual que la skin rep per dibuixar.
struct EstatVisualSkin
{
	EstatSkinBlanca estat = EstatSkinBlanca::Idle;
	float plec = 0.0f;			// Grau d'enrotllament: 0 = desenrotllat, 1 = bola completa
	float tempsIdle = 0.0f;		// Rellotge de l'animacio d'espera (s)
	float tempsEstat = 0.0f;	// Temps transcorregut dins l'estat actual (s)
};

// Dades de nomes lectura per dibuixar la bola (res del que hi ha aqui es torna a escriure a la fisica).
struct ContextVisualBlanca
{
	glm::mat4 matriuBase = glm::mat4(1.0f);		// Matriu global de l'escena (MatriuTG)
	glm::vec3 centre = glm::vec3(0.0f);			// Centre visual de la bola (ja amb l'enfonsament de tronera)
	glm::mat4 rotacioFisica = glm::mat4(1.0f);	// rotacioBolaBlanca: gir acumulat pel rodolament fisic
	float radi = 0.11f;							// Radi visual de la bola (ja amb l'enfonsament aplicat)
	float angleApuntat = 0.0f;					// Direccio d'apuntat del tac (radians, pla XY de la taula)
	GLuint textura = 0;							// Textura de la bola blanca (la fa servir la skin classica)
};

// Interficie d'una skin.
class ISkinBlanca
{
public:
	virtual ~ISkinBlanca() {}
	virtual const char* Nom() const = 0;
	virtual bool Carrega() = 0;								// Retorna fals si no es pot fer servir (fitxers que falten...)
	virtual void Allibera() {}								// Allibera els recursos OpenGL
	virtual float DuradaEnrotllat() const { return 0.22f; }		// Segons de Idle -> bola
	virtual float DuradaDesenrotllat() const { return 0.30f; }	// Segons de bola -> Idle
	virtual void Dibuixa(GLuint sh_programID, const EstatVisualSkin& estat, const ContextVisualBlanca& ctx) = 0;
};

class ControladorSkinBlanca
{
public:
	ControladorSkinBlanca();
	~ControladorSkinBlanca() {}		// No toca OpenGL: cal cridar Allibera() abans amb el context actiu.

	bool Carrega();					// Carrega la skin seleccionada (i la classica com a reserva)
	void Allibera();

	// ---- Maquina d'estats ----
	void Reinicia();											// Torna a Idle (nova partida, blanca recolocada...)
	void Actualitza(const IntencioSkinBlanca& intencio, float dt);	// Un cop per fotograma de joc
	void TickIdle(float dt);									// Nomes avanca el rellotge d'espera (p.ex. al menu)

	// ---- Dibuix ----
	bool Dibuixa(GLuint sh_programID, const ContextVisualBlanca& ctx);	// Fals si no hi ha cap skin utilitzable

	// ---- Skins ----
	static int NumSkins();
	static const char* NomSkin(int index);
	bool SeleccionaSkin(int index);
	int  SkinActual() const { return m_actual; }

	EstatSkinBlanca Estat() const { return m_estat; }
	float Plec() const { return m_plec; }

private:
	void Entra(EstatSkinBlanca nou);
	ISkinBlanca* ObteSkin(int index);		// Crea i carrega (una sola vegada) la skin del registre
	ISkinBlanca* SkinActiva();

	EstatSkinBlanca m_estat = EstatSkinBlanca::Idle;
	float m_plec = 0.0f;
	float m_tempsIdle = 0.0f;
	float m_tempsEstat = 0.0f;

	int m_actual = 1;						// Skin per defecte: Armadillo (index 1 del registre)
	std::vector<ISkinBlanca*> m_skins;		// Una entrada per skin del registre (NULL = encara no creada)
	std::vector<int> m_estatCarrega;		// 0 = sense provar, 1 = carregada, -1 = ha fallat
};
