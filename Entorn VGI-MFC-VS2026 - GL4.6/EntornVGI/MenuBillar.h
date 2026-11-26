// MenuBillar.h : Menu principal del videojoc de billar 3D
//
//   Pantalles:  - PRINCIPAL: botons JUGAR, CONTROLS i CREDITS
//               - CONTROLS : explicacio de les tecles i del ratoli
//               - CREDITS  : equip i tecnologia
//
//   Es dibuixa en 2D amb OpenGL per sobre de l'escena 3D (la vista la posa a
//   CEntornVGIView::OnPaint). El text es genera amb GDI (Segoe UI) i es puja
//   com a textura, de manera que no cal cap llibreria de fonts addicional.
//
//   IMPORTANT: Dibuixa() i Allibera() s'han de cridar amb el context OpenGL actiu.

#pragma once

#include <gl/glew.h>
#include <map>
#include <string>

class CMenuBillar
{
public:
	enum Pantalla { PANT_PRINCIPAL, PANT_NORMES, PANT_CONTROLS, PANT_CREDITS, PANT_PAUSA, PANT_VICTORIA, PANT_MODALITATS };
	enum Accio { ACCIO_CAP, ACCIO_JUGAR, ACCIO_CONTINUAR, ACCIO_REINICIAR, ACCIO_MENU, ACCIO_ESCACS, ACCIO_MODALITATS, ACCIO_SORTIR, ACCIO_FUTBOL };

	struct HUDInfo
	{
		bool escacs = false;
        bool futbol = false;
        int jugadorFutbol=1, seleccioFutbol=1, golFutbol=0;
        int punts1 = 0, punts2 = 0;
        bool exploracio = false;
        int jugador = 1;
		int restants1 = 7;
		int restants2 = 7;
		int tirsDisponibles = 1;
		bool embocades[16] = {};
		std::wstring missatge;
		bool missatgeFalta = false;
		int grup1 = 0;
		int grup2 = 0;
		int segons = 0;
		float forca = 0.0f;
		bool superior = true;
		bool blancaEnMa = false;
	};

	CMenuBillar() {}
	~CMenuBillar() {}

	// ---- Dibuix (context OpenGL actiu) ----
	void Dibuixa(int ampladaFinestra, int alcadaFinestra, const HUDInfo* hud = nullptr);
	void Allibera();

	// ---- Entrada. (x,y) en coordenades de la finestra (origen dalt a l'esquerra) ----
	void   RatoliMou(int x, int y, int ampladaFinestra, int alcadaFinestra);
	void   RatoliPrem(int x, int y, int ampladaFinestra, int alcadaFinestra);
	Accio  RatoliSolta(int x, int y, int ampladaFinestra, int alcadaFinestra);
	Accio  TeclaPremuda(UINT nChar);

	// ---- Estat ----
	void     MostraPrincipal();
    void MostraModalitats();
    bool BotoPausa(int x, int y, int w, int h) const;
    void ModeEscacs(bool actiu) { m_normesEscacs = actiu; }
    void ModeFutbol(bool actiu) { m_normesFutbol = actiu; }
	void     MostraPausa();
	void     MostraVictoria(int jugador);
	Pantalla PantallaActual() const { return m_pantalla; }

private:
	struct Rect
	{
		float x, y, w, h;
		bool Conte(float px, float py) const { return px >= x && px <= x + w && py >= y && py <= y + h; }
	};

	// Posicions de tots els elements, calculades a partir de la mida de la finestra
	struct Disseny
	{
		float s;				// Factor d'escala (1.0 = finestra de 1100x760)
		float w, h;				// Mides de la finestra
		int   nBotons;			// Botons visibles a la pantalla actual
		Rect  boto[6];
		Rect  panell;			// Panell de fons (Controls i Credits)
		float titolY;			// Centre vertical del titol
		float subtitolY;		// Centre vertical del subtitol
	};

	struct Text { GLuint tex; int w; int h; };

	void   CalculaDisseny(int w, int h, Disseny& d) const;
	int    BotoSota(float x, float y, const Disseny& d) const;
	Accio  ExecutaBoto(int index);
	void   TornaEnrere();

	// Dibuix de cada pantalla
	void DibuixaPrincipal(const Disseny& d);
	void DibuixaNormes(const Disseny& d);
	void DibuixaControls(const Disseny& d);
	void DibuixaPausa(const Disseny& d);
	void DibuixaVictoria(const Disseny& d);
	void DibuixaHUDIntern(const Disseny& d, const HUDInfo& hud);
	void DibuixaCredits(const Disseny& d);
	void DibuixaBoto(const Rect& r, const wchar_t* etiqueta, float anim, float s);

	// Primitives 2D
	bool InicialitzaGL();
	void Pla(float x, float y, float w, float h, const float color[4]);
	void RectArrodonit(const Rect& r, const float colorFons[4], const float colorVora[4], float radi, float gruixVora);
	void DibuixaText(const std::wstring& text, float x, float yCentre, int pxAlt, bool negreta,
		const float color[4], int alineacio);	// alineacio: 0=esquerra, 1=centre, 2=dreta
	const Text* ObteText(const std::wstring& text, int pxAlt, bool negreta);
	void NetejaTextos();

	// Estat del menu
	Pantalla m_pantalla = PANT_PRINCIPAL;
	int   m_seleccio = 0;				// Boto ressaltat (ratoli o teclat)
	int   m_premut = -1;				// Boto sobre el qual s'ha premut el ratoli
	int   m_ultimaOpcio = 0;
	bool  m_normesDesDePausa = false;
    bool m_normesEscacs = false;
    bool m_normesFutbol = false;
    bool m_normesDesDeModalitats = false;
	int   m_guanyador = 0;			// Opcio del menu principal d'on venim (per tornar-hi)
	float m_anim[6] = {};	// Animacio de ressaltat de cada boto (0..1)
	ULONGLONG m_ultimTemps = 0;
	int   m_ampladaAnt = 0;
	int   m_alcadaAnt = 0;

	// Recursos OpenGL
	GLuint m_programa = 0;
	GLuint m_vao = 0;
	GLuint m_vbo = 0;
	bool   m_errorGL = false;
	GLint  m_uRect = -1, m_uViewport = -1, m_uMode = -1, m_uColor = -1, m_uColor2 = -1;
	GLint  m_uRadi = -1, m_uVora = -1, m_uTex = -1;

	std::map<std::wstring, Text> m_textos;	// Cache de textures de text
};
