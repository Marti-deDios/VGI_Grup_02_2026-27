// MenuBillar.cpp : Menu principal del videojoc de billar 3D (veure MenuBillar.h)

#include "pch.h"
#include "MenuBillar.h"
#include <algorithm>
#include <cmath>
#include <vector>

/* ------------------------------------------------------------------------- */
/*   TEXTOS I COLORS DEL MENU  (aqui podeu canviar-ho tot sense tocar codi)  */
/* ------------------------------------------------------------------------- */
namespace
{
	const wchar_t* const TITOL_JOC = L"BILLAR 3D";
	const wchar_t* const SUBTITOL_JOC = L"UNA TAULA. INFINITES POSSIBILITATS.";
	const wchar_t* const AJUDA_PRINCIPAL = L"F: PANTALLA COMPLETA / FINESTRA";

	const wchar_t* const ETIQUETES_PRINCIPAL[5] = { L"MODALITATS", L"NORMES DEL JOC", L"CONTROLS I AJUDA", L"CREDITS", L"SORTIR DEL JOC" };
	const wchar_t* const ETIQUETES_PAUSA[6] = { L"CONTINUAR", L"NORMES DEL JOC", L"REINICIAR PARTIDA", L"TORNAR A MODALITATS", L"MENU PRINCIPAL", L"SORTIR DEL JOC" };
	const wchar_t* const ETIQUETES_FINAL[2] = { L"REVENJA", L"MENÚ PRINCIPAL" };
	const wchar_t* const ETIQUETA_TORNAR = L"TORNAR";

	// ---- Pantalla CONTROLS: tecla / accio ----
	struct FilaControl { const wchar_t* tecla; const wchar_t* accio; };
	const FilaControl CONTROLS[] =
	{
		{ L"Moure el ratolí",                  L"Apuntar el tac (esquerra / dreta)" },
		{ L"A / D   o   fletxes",              L"Girar el tac amb més precisió" },
		{ L"Botó esquerre + arrossegar avall", L"Carregar la força del cop" },
		{ L"Deixar anar el botó",              L"Executar el tir" },
		{ L"Mantenir Espai",                   L"Carregar força i deixar anar per tirar" },
		{ L"1",                                L"Càmera de tir (darrere la bola blanca)" },
		{ L"2",                                L"Càmera superior" },
		{ L"V",                                L"Càmera lliure (arrossega i roda per fer zoom)" },
		{ L"R",                                L"Reiniciar la partida" },
		{ L"F",                                L"Pantalla completa" },
		{ L"Esc",                              L"Pausar / reprendre la partida" },
	};
	const int N_CONTROLS = (int)(sizeof(CONTROLS) / sizeof(CONTROLS[0]));

	// ---- Pantalla CREDITS: tipus 0 = espai, 1 = encapçalament, 2 = text normal ----
	struct LiniaCredits { const wchar_t* text; int tipus; };
	const LiniaCredits CREDITS[] =
	{
		{ L"DESENVOLUPAMENT",                              1 },
		{ L"Martí De Dios, Foix Torrents,",                 2 },
		{ L"Victor Titos, Núria Bruch,",                    2 },
		{ L"Sofía Atienza i Arnau Cuixart",                 2 },
		{ L"",                                             0 },
		{ L"ASSIGNATURA",                                  1 },
		{ L"Visualització Gràfica Interactiva",            2 },
		{ L"Escola d'Enginyeria - UAB",                    2 },
		{ L"",                                             0 },
		{ L"TECNOLOGIA",                                   1 },
		{ L"C++  ·  MFC  ·  OpenGL 4.6  ·  GLM  ·  GLEW  ·  SOIL2", 2 },
	};
	const int N_CREDITS = (int)(sizeof(CREDITS) / sizeof(CREDITS[0]));

	// ---- Colors (r,g,b,a) ----
	const float COLOR_DAURAT[4]      = { 0.92f, 0.74f, 0.42f, 1.00f };
	const float COLOR_BLANC[4]       = { 0.96f, 0.95f, 0.90f, 1.00f };
	const float COLOR_GRIS[4]        = { 0.72f, 0.78f, 0.74f, 1.00f };
	const float COLOR_OMBRA[4]       = { 0.00f, 0.00f, 0.00f, 0.65f };
	const float COLOR_OMBRA_BOTO[4]  = { 0.00f, 0.00f, 0.00f, 0.40f };
	const float COLOR_BOTO[4]        = { 0.08f, 0.27f, 0.20f, 0.96f };	// Verd tapet
	const float COLOR_BOTO_SEL[4]    = { 0.15f, 0.44f, 0.31f, 1.00f };
	const float COLOR_VORA_BOTO[4]   = { 0.55f, 0.42f, 0.15f, 1.00f };	// Fusta
	const float COLOR_VORA_SEL[4]    = { 1.00f, 0.82f, 0.25f, 1.00f };	// Daurat
	const float COLOR_PANELL[4]      = { 0.03f, 0.10f, 0.08f, 0.94f };
	const float COLOR_VORA_PANELL[4] = { 0.51f, 0.36f, 0.19f, 1.00f };
	const float COLOR_LINIA[4]       = { 0.92f, 0.74f, 0.42f, 0.45f };
	const float COLOR_BARRA[4]       = { 0.08f, 0.19f, 0.15f, 0.91f };
	const float COLOR_VERD_CLAR[4]  = { 0.26f, 0.63f, 0.43f, 1.00f };

	// Mides (en unitats de disseny; es multipliquen per l'escala s)
	const float CAPCALERA_PANELL = 100.0f;
	const float ALTURA_FILA_CONTROLS = 40.0f;
	const float PEU_PANELL = 110.0f;

	float AlturaLiniaCredits(int tipus) { return (tipus == 0) ? 16.0f : 32.0f; }

	float AlturaContingutCredits()
	{
		float total = 0.0f;
		for (int i = 0; i < N_CREDITS; ++i) total += AlturaLiniaCredits(CREDITS[i].tipus);
		return total;
	}

	int Px(float mida, float s) { return (std::max)(8, (int)(mida * s + 0.5f)); }

	void Barreja(const float a[4], const float b[4], float t, float sortida[4])
	{
		for (int i = 0; i < 4; ++i) sortida[i] = a[i] + (b[i] - a[i]) * t;
	}

	// GeneraBitmapText: dibuixa el text amb GDI (Segoe UI, antialiasing en gris) i el
	// converteix a RGBA blanc amb l'alfa = intensitat del text.
	bool GeneraBitmapText(const std::wstring& text, int pxAlt, bool negreta,
		std::vector<unsigned char>& rgba, int& ampl, int& alt)
	{
		HDC dcPantalla = ::GetDC(NULL);
		HDC dc = ::CreateCompatibleDC(dcPantalla);
		::ReleaseDC(NULL, dcPantalla);
		if (dc == NULL) return false;

		HFONT font = ::CreateFontW(-pxAlt, 0, 0, 0, negreta ? FW_BOLD : FW_NORMAL, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
			DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
		HGDIOBJ fontAnterior = ::SelectObject(dc, font);

		SIZE mida = { 0, 0 };
		::GetTextExtentPoint32W(dc, text.c_str(), (int)text.size(), &mida);
		const int marge = 4;
		ampl = mida.cx + 2 * marge;
		alt = mida.cy + 2 * marge;

		bool correcte = false;
		if (mida.cx > 0 && mida.cy > 0 && ampl < 4096 && alt < 1024)
		{
			BITMAPINFO bmi;
			ZeroMemory(&bmi, sizeof(bmi));
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = ampl;
			bmi.bmiHeader.biHeight = -alt;			// Negatiu: fila 0 = part superior
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;

			void* bits = NULL;
			HBITMAP bmp = ::CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
			if (bmp != NULL && bits != NULL)
			{
				HGDIOBJ bmpAnterior = ::SelectObject(dc, bmp);
				const size_t nPixels = (size_t)ampl * (size_t)alt;
				ZeroMemory(bits, nPixels * 4);

				::SetBkMode(dc, TRANSPARENT);
				::SetTextColor(dc, RGB(255, 255, 255));
				::TextOutW(dc, marge, marge, text.c_str(), (int)text.size());
				::GdiFlush();

				const unsigned char* origen = (const unsigned char*)bits;
				rgba.resize(nPixels * 4);
				for (size_t i = 0; i < nPixels; ++i)
				{
					rgba[4 * i + 0] = 255;
					rgba[4 * i + 1] = 255;
					rgba[4 * i + 2] = 255;
					rgba[4 * i + 3] = origen[4 * i + 1];	// Canal verd = intensitat del text
				}
				correcte = true;
				::SelectObject(dc, bmpAnterior);
			}
			if (bmp != NULL) ::DeleteObject(bmp);
		}

		::SelectObject(dc, fontAnterior);
		::DeleteObject(font);
		::DeleteDC(dc);
		return correcte;
	}

	GLuint CompilaShader(GLenum tipus, const char* codi)
	{
		GLuint shader = glCreateShader(tipus);
		glShaderSource(shader, 1, &codi, NULL);
		glCompileShader(shader);
		GLint ok = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
		if (!ok)
		{
			char registre[1024];
			glGetShaderInfoLog(shader, sizeof(registre), NULL, registre);
			::OutputDebugStringA("MenuBillar: error compilant shader:\n");
			::OutputDebugStringA(registre);
			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}

	// ---- Shaders del menu (incrustats al codi: no cal copiar cap fitxer .vert/.frag) ----
	const char* const VERTEX_MENU = R"GLSL(#version 330 core
layout(location = 0) in vec2 aPos;
uniform vec4 uRect;          // x, y, ample, alt (pixels, origen dalt a l'esquerra)
uniform vec2 uViewport;
out vec2 vUV;
void main()
{
	vec2 p = uRect.xy + aPos * uRect.zw;
	gl_Position = vec4(p.x / uViewport.x * 2.0 - 1.0, 1.0 - p.y / uViewport.y * 2.0, 0.0, 1.0);
	vUV = aPos;
}
)GLSL";

	const char* const FRAGMENT_MENU = R"GLSL(#version 330 core
in vec2 vUV;
uniform int uMode;           // 0 = color pla, 1 = text (alfa de la textura), 2 = rectangle arrodonit
uniform vec4 uColor;
uniform vec4 uColor2;        // Color de la vora (mode 2)
uniform vec4 uRect;
uniform float uRadi;
uniform float uVora;
uniform sampler2D uTex;
out vec4 fragColor;
void main()
{
	if (uMode == 1)
	{
		float a = texture(uTex, vUV).a;
		fragColor = vec4(uColor.rgb, uColor.a * a);
	}
	else if (uMode == 2)
	{
		vec2 mig = uRect.zw * 0.5;
		vec2 p = (vUV - 0.5) * uRect.zw;
		vec2 d = abs(p) - (mig - vec2(uRadi));
		float dist = length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - uRadi;
		float cobert = 1.0 - smoothstep(-1.0, 0.0, dist);
		float vora = smoothstep(-uVora - 1.0, -uVora, dist);
		vec3 col = mix(uColor.rgb, uColor2.rgb, vora);
		float alfa = mix(uColor.a, uColor2.a, vora);
		fragColor = vec4(col, alfa * cobert);
	}
	else
	{
		fragColor = uColor;
	}
}
)GLSL";
}

/* ------------------------------------------------------------------------- */
/*                         ESTAT I ENTRADA (RATOLI / TECLAT)                 */
/* ------------------------------------------------------------------------- */

void CMenuBillar::MostraModalitats()
{
    MostraPrincipal();
    m_pantalla = PANT_MODALITATS;
}

void CMenuBillar::MostraPrincipal()
{
    m_normesDesDeModalitats = false;
	m_normesDesDePausa = false;
	m_pantalla = PANT_PRINCIPAL;
	m_seleccio = 0;
	m_ultimaOpcio = 0;
	m_premut = -1;
	for (float& a : m_anim) a = 0.0f;
}

void CMenuBillar::MostraPausa()
{
	m_normesDesDePausa = false;
	m_pantalla = PANT_PAUSA;
	m_seleccio = 0;
	m_premut = -1;
}

void CMenuBillar::MostraVictoria(int jugador)
{
	m_guanyador = jugador;
	m_pantalla = PANT_VICTORIA;
	m_seleccio = 0;
	m_premut = -1;
}

void CMenuBillar::TornaEnrere()
{
    if (m_normesDesDeModalitats) { m_normesDesDeModalitats = false; m_pantalla = PANT_MODALITATS; m_seleccio = 2; m_premut = -1; return; }
	if (m_normesDesDePausa)
	{
		m_normesDesDePausa = false;
		m_pantalla = PANT_PAUSA;
		m_seleccio = 1;
	}
	else
	{
		m_pantalla = PANT_PRINCIPAL;
		m_seleccio = m_ultimaOpcio;
	}
	m_premut = -1;
	for (float& a : m_anim) a = 0.0f;
}

CMenuBillar::Accio CMenuBillar::ExecutaBoto(int index)
{
	if (index < 0) return ACCIO_CAP;
    if (m_pantalla == PANT_MODALITATS) {
        if (index == 0) return ACCIO_JUGAR;
        if (index == 1) return ACCIO_ESCACS;
        if (index == 2) { m_normesEscacs = true; m_normesDesDeModalitats = true; m_pantalla = PANT_NORMES; m_seleccio = 0; return ACCIO_CAP; }
        TornaEnrere(); return ACCIO_CAP;
    }

	if (m_pantalla == PANT_PAUSA)
	{
		if (index == 0) return ACCIO_CONTINUAR;
		if (index == 1)
		{
			m_normesDesDePausa = true;
			m_pantalla = PANT_NORMES;
			m_seleccio = 0;
			return ACCIO_CAP;
		}
		if (index == 2) return ACCIO_REINICIAR;
        if (index == 3) return ACCIO_MODALITATS;
        if (index == 4) return ACCIO_MENU;
        return ACCIO_SORTIR;
	}
	if (m_pantalla == PANT_VICTORIA)
		return index == 0 ? ACCIO_REINICIAR : ACCIO_MENU;

	if (m_pantalla == PANT_PRINCIPAL)
	{
		if (index == 4) return ACCIO_SORTIR;
        m_ultimaOpcio = index;
		if (index == 0) { m_pantalla = PANT_MODALITATS; m_seleccio = 0; m_premut = -1; for(float& v : m_anim) v=0; return ACCIO_CAP; }
		if (index == 1) { m_normesEscacs = false; m_pantalla = PANT_NORMES; }
		else if (index == 2) m_pantalla = PANT_CONTROLS;
		else m_pantalla = PANT_CREDITS;
		m_seleccio = 0;
		m_premut = -1;
		for (float& v : m_anim) v = 0.0f;
		return ACCIO_CAP;
	}

	TornaEnrere();
	return ACCIO_CAP;
}

int CMenuBillar::BotoSota(float x, float y, const Disseny& d) const
{
	for (int i = 0; i < d.nBotons; ++i)
		if (d.boto[i].Conte(x, y)) return i;
	return -1;
}

void CMenuBillar::RatoliMou(int x, int y, int ampladaFinestra, int alcadaFinestra)
{
	Disseny d;
	CalculaDisseny(ampladaFinestra, alcadaFinestra, d);
	const int i = BotoSota((float)x, (float)y, d);
	if (i >= 0) m_seleccio = i;
}

void CMenuBillar::RatoliPrem(int x, int y, int ampladaFinestra, int alcadaFinestra)
{
	Disseny d;
	CalculaDisseny(ampladaFinestra, alcadaFinestra, d);
	m_premut = BotoSota((float)x, (float)y, d);
	if (m_premut >= 0) m_seleccio = m_premut;
}

CMenuBillar::Accio CMenuBillar::RatoliSolta(int x, int y, int ampladaFinestra, int alcadaFinestra)
{
	Disseny d;
	CalculaDisseny(ampladaFinestra, alcadaFinestra, d);
	const int sota = BotoSota((float)x, (float)y, d);
	const int premut = m_premut;
	m_premut = -1;

	// Nomes s'activa si s'ha premut i deixat anar sobre el mateix boto
	if (sota >= 0 && sota == premut) return ExecutaBoto(sota);
	return ACCIO_CAP;
}

CMenuBillar::Accio CMenuBillar::TeclaPremuda(UINT nChar)
{
	const int n = m_pantalla == PANT_PRINCIPAL ? 5 : m_pantalla == PANT_MODALITATS ? 4 : m_pantalla == PANT_PAUSA ? 6 : m_pantalla == PANT_VICTORIA ? 2 : 1;

	switch (nChar)
	{
	case VK_UP:
	case 'W':
		m_seleccio = (m_seleccio + n - 1) % n;
		break;
	case VK_DOWN:
	case 'S':
		m_seleccio = (m_seleccio + 1) % n;
		break;
	case VK_RETURN:
	case VK_SPACE:
		return ExecutaBoto(m_seleccio);
	case VK_ESCAPE:
	case VK_BACK:
		if (m_pantalla == PANT_PAUSA) return ACCIO_CONTINUAR;
		if (m_pantalla == PANT_MODALITATS || m_pantalla == PANT_NORMES || m_pantalla == PANT_CONTROLS || m_pantalla == PANT_CREDITS) TornaEnrere();
		break;
	}
	return ACCIO_CAP;
}

/* ------------------------------------------------------------------------- */
/*                                   DISSENY                                 */
/* ------------------------------------------------------------------------- */

void CMenuBillar::CalculaDisseny(int w, int h, Disseny& d) const
{
	d.w = (float)w;
	d.h = (float)h;
	float s = (std::min)(h / 760.0f, w / 1100.0f);
	s = (std::max)(0.45f, (std::min)(2.5f, s));
	d.s = s;
	d.nBotons = 0;
	d.panell = Rect{ 0, 0, 0, 0 };
	d.titolY = d.subtitolY = 0.0f;
	for (int i = 0; i < 6; ++i) d.boto[i] = Rect{ 0, 0, 0, 0 };

	if (m_pantalla == PANT_PRINCIPAL || m_pantalla == PANT_MODALITATS)
	{
		const float ample = 510.0f * s;
        d.panell = Rect{ (d.w-ample)*0.5f, (d.h-650.0f*s)*0.5f, ample, 650.0f*s };
        d.titolY = d.panell.y + 125.0f*s;
        d.subtitolY = d.panell.y + 185.0f*s;
        d.nBotons = m_pantalla == PANT_PRINCIPAL ? 5 : 4;
        for (int i=0;i<d.nBotons;++i)
            d.boto[i] = Rect{d.panell.x+38.0f*s,d.panell.y+(225.0f+i*69.0f)*s,ample-76.0f*s,55.0f*s};
	}
	else
	{
		const bool controls = (m_pantalla == PANT_CONTROLS);
		const bool credits = (m_pantalla == PANT_CREDITS);
		const float contingut = controls ? N_CONTROLS * ALTURA_FILA_CONTROLS :
			credits ? AlturaContingutCredits() : m_pantalla == PANT_NORMES ? 330.0f : 250.0f;
		const float amplada = (controls ? 1000.0f : 760.0f) * s;
		const float alcada = m_pantalla == PANT_PAUSA ? 610.0f*s : (CAPCALERA_PANELL + contingut + PEU_PANELL)*s;
		d.panell = Rect{ (d.w - amplada) * 0.5f, (d.h - alcada) * 0.5f, amplada, alcada };
		const bool pausa = m_pantalla == PANT_PAUSA;
		const bool final = m_pantalla == PANT_VICTORIA;
		d.nBotons = pausa ? 6 : final ? 2 : 1;
		const float ampladaBoto = (pausa || final ? 340.0f : 260.0f) * s;
		for (int i = 0; i < d.nBotons; ++i)
		{
			const float y = (pausa || final) ? d.panell.y + (136.0f + i * 68.0f) * s : d.panell.y + alcada - 82.0f * s;
			d.boto[i] = Rect{ (d.w - ampladaBoto) * 0.5f, y, ampladaBoto, 54.0f * s };
		}
	}
}

/* ------------------------------------------------------------------------- */
/*                                    DIBUIX                                 */
/* ------------------------------------------------------------------------- */

void CMenuBillar::Dibuixa(int w, int h, const HUDInfo* hud)
{
	if (w <= 0 || h <= 0) return;
	if (!InicialitzaGL()) return;

	// ---- Temps per a l'animacio de ressaltat ----
	const ULONGLONG ara = GetTickCount64();
	float dt = (m_ultimTemps == 0) ? 0.016f : (float)(ara - m_ultimTemps) / 1000.0f;
	m_ultimTemps = ara;
	dt = (std::min)(dt, 0.1f);

	// Les textures depenen de la mida de la finestra: es regeneren quan canvia
	if (w != m_ampladaAnt || h != m_alcadaAnt)
	{
		NetejaTextos();
		m_ampladaAnt = w;
		m_alcadaAnt = h;
	}

	Disseny d;
	CalculaDisseny(w, h, d);

	for (int i = 0; i < 6; ++i)
	{
		const float objectiu = (i == m_seleccio && i < d.nBotons) ? 1.0f : 0.0f;
		m_anim[i] += (objectiu - m_anim[i]) * (std::min)(1.0f, dt * 14.0f);
	}

	// ---- Guardem l'estat OpenGL que modifiquem, per no alterar l'escena 3D ----
	GLint programaAnt = 0, textureAnt = 0, unitatAnt = 0, vaoAnt = 0;
	GLint viewportAnt[4] = { 0, 0, w, h };
	glGetIntegerv(GL_VIEWPORT, viewportAnt);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vaoAnt);
	GLint blendSrcRGB = 0, blendDstRGB = 0, blendSrcA = 0, blendDstA = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programaAnt);
	glGetIntegerv(GL_ACTIVE_TEXTURE, &unitatAnt);
	glActiveTexture(GL_TEXTURE0);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureAnt);
	glGetIntegerv(GL_BLEND_SRC_RGB, &blendSrcRGB);
	glGetIntegerv(GL_BLEND_DST_RGB, &blendDstRGB);
	glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrcA);
	glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDstA);
	const GLboolean teDepth = glIsEnabled(GL_DEPTH_TEST);
	const GLboolean teCull = glIsEnabled(GL_CULL_FACE);
	const GLboolean teBlend = glIsEnabled(GL_BLEND);
	const GLboolean teScissor = glIsEnabled(GL_SCISSOR_TEST);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, w, h);

	glUseProgram(m_programa);
	glUniform2f(m_uViewport, (float)w, (float)h);
	glUniform1i(m_uTex, 0);
	glBindVertexArray(m_vao);

	if (hud)
	{
		DibuixaHUDIntern(d, *hud);
	}
	else
	{
		const float vel[4] = { 0.0f, 0.0f, 0.0f, m_pantalla == PANT_PRINCIPAL ? 0.18f : 0.58f };
		glUniform1i(m_uMode, 0);
		Pla(0.0f, 0.0f, d.w, d.h, vel);
		switch (m_pantalla)
		{
		case PANT_MODALITATS: case PANT_PRINCIPAL: DibuixaPrincipal(d); break;
		case PANT_NORMES: DibuixaNormes(d); break;
		case PANT_CONTROLS: DibuixaControls(d); break;
		case PANT_CREDITS: DibuixaCredits(d); break;
		case PANT_PAUSA: DibuixaPausa(d); break;
		case PANT_VICTORIA: DibuixaVictoria(d); break;
		}
	}

	// ---- Restaurem l'estat OpenGL ----
	glBindVertexArray((GLuint)vaoAnt);
	glBindTexture(GL_TEXTURE_2D, (GLuint)textureAnt);
	glActiveTexture((GLenum)unitatAnt);
	glUseProgram((GLuint)programaAnt);
	glBlendFuncSeparate((GLenum)blendSrcRGB, (GLenum)blendDstRGB, (GLenum)blendSrcA, (GLenum)blendDstA);
	if (!teBlend) glDisable(GL_BLEND);
	if (teDepth) glEnable(GL_DEPTH_TEST);
	if (teCull) glEnable(GL_CULL_FACE);
	if (teScissor) glEnable(GL_SCISSOR_TEST);
	glViewport(viewportAnt[0], viewportAnt[1], viewportAnt[2], viewportAnt[3]);
}

void CMenuBillar::DibuixaPrincipal(const Disseny& d)
{
	const float s = d.s;
	const Rect& p = d.panell;
	const float centreX = p.x + p.w * 0.5f;
	RectArrodonit(p, COLOR_PANELL, COLOR_VORA_PANELL, 20.0f * s, 2.0f * s);
	DibuixaText(L"BILLIARDS ARENA", centreX, p.y + 45.0f * s, Px(18.0f, s), true, COLOR_DAURAT, 1);
	Pla(p.x + 44.0f * s, p.y + 75.0f * s, p.w - 88.0f * s, 1.0f * s, COLOR_LINIA);
	DibuixaText((m_pantalla == PANT_MODALITATS ? L"JOCS" : TITOL_JOC), centreX + 2.0f * s, d.titolY + 3.0f * s, Px(71.0f, s), true, COLOR_OMBRA, 1);
	DibuixaText((m_pantalla == PANT_MODALITATS ? L"JOCS" : TITOL_JOC), centreX, d.titolY, Px(71.0f, s), true, COLOR_BLANC, 1);
	DibuixaText(SUBTITOL_JOC, centreX, d.subtitolY, Px(19.0f, s), true, COLOR_DAURAT, 1);
	for (int i = 0; i < d.nBotons; ++i)
		DibuixaBoto(d.boto[i], m_pantalla == PANT_MODALITATS ? (i==0 ? L"BILLAR CLASSIC" : i==1 ? L"CHESS BILLIARDS" : i==2 ? L"NORMES ESCACS" : L"TORNAR") : ETIQUETES_PRINCIPAL[i], m_anim[i], s);
	Pla(p.x + 44.0f * s, p.y + p.h - 50.0f * s, p.w - 88.0f * s, 1.0f * s, COLOR_LINIA);
	DibuixaText(AJUDA_PRINCIPAL, centreX, p.y + p.h - 26.0f * s, Px(14.0f, s), false, COLOR_GRIS, 1);
}

void CMenuBillar::DibuixaNormes(const Disseny& d)
{
	const float s = d.s;
	const Rect& p = d.panell;
	RectArrodonit(p, COLOR_PANELL, COLOR_VORA_PANELL, 20.0f * s, 2.0f * s);
	DibuixaText(L"NORMES DEL JOC", d.w * 0.5f, p.y + 47.0f * s, Px(41.0f, s), true, COLOR_DAURAT, 1);
	Pla(p.x + 42.0f * s, p.y + 87.0f * s, p.w - 84.0f * s, 2.0f * s, COLOR_LINIA);
	const wchar_t* escacs[] = {
    L"OBJECTIU: captura mes peces rivals que l'altre jugador.",
    L"J1: ivori. J2: obsidiana. Set peces per equip.",
    L"Cada captura rival suma un punt i permet repetir tir.",
    L"Una peca propia embocada no dona punts.",
    L"Falta: blanca embocada o sense contacte.",
    L"En falta no puntues; rival amb blanca en ma i dos tirs.",
    L"En acabar les 14 peces, guanya qui te mes punts.",
    L"Si els punts coincideixen, es un empat.",
    L"C: explorar sala. WASD: caminar. Fletxes: mirar."
};
const wchar_t* linies[] = {
		L"OBJECTIU: emboca les boles del teu grup i la negra.",
		L"Llises: 1-7. Ratllades: 9-15. Negra: 8.",
		L"Si emboces una bola legal, continues tirant.",
		L"FALTA: entra la blanca -> el rival te DOS TIRS.",
		L"FALTA: no toques cap bola -> el rival te DOS TIRS.",
		L"FALTA: primer contacte incorrecte -> DOS TIRS.",
		L"FALTA: ni embocada ni banda posterior -> DOS TIRS.",
		L"Despres de falta: blanca a la linia amb ratoli o I/K.",
		L"Negra prematura o amb falta: partida perduda."
	};
	for (int i = 0; i < 9; ++i)
		DibuixaText(m_normesEscacs ? escacs[i] : linies[i], p.x + 45.0f * s, p.y + (120.0f + i * 36.0f) * s, Px(20.0f, s), false, COLOR_BLANC, 0);
	DibuixaBoto(d.boto[0], ETIQUETA_TORNAR, m_anim[0], s);
}

void CMenuBillar::DibuixaPausa(const Disseny& d)
{
	const float s = d.s;
	const Rect& p = d.panell;
	RectArrodonit(p, COLOR_PANELL, COLOR_VORA_PANELL, 20.0f * s, 2.0f * s);
	DibuixaText(L"PARTIDA EN PAUSA", d.w * 0.5f, p.y + 53.0f * s, Px(37.0f, s), true, COLOR_DAURAT, 1);
	for (int i = 0; i < d.nBotons; ++i) DibuixaBoto(d.boto[i], ETIQUETES_PAUSA[i], m_anim[i], s);
}

void CMenuBillar::DibuixaVictoria(const Disseny& d)
{
	const float s = d.s;
	const Rect& p = d.panell;
	RectArrodonit(p, COLOR_PANELL, COLOR_VORA_PANELL, 20.0f * s, 2.0f * s);
	DibuixaText(L"CAMPIONS DEL TAPET", d.w * 0.5f, p.y + 49.0f * s, Px(36.0f, s), true, COLOR_DAURAT, 1);
	Pla(p.x + 75.0f * s, p.y + 81.0f * s, p.w - 150.0f * s, 2.0f * s, COLOR_LINIA);
	const std::wstring text = m_guanyador == 3 ? L"EMPAT" : L"VICTORIA DEL JUGADOR " + std::to_wstring(m_guanyador);
	DibuixaText(text, d.w * 0.5f, p.y + 106.0f * s, Px(30.0f, s), true, COLOR_BLANC, 1);
	DibuixaText(L"UNA PARTIDA PER RECORDAR", d.w * 0.5f, p.y + 145.0f * s, Px(17.0f, s), false, COLOR_DAURAT, 1);
	for (int i = 0; i < 2; ++i) DibuixaBoto(d.boto[i], ETIQUETES_FINAL[i], m_anim[i], s);
}

bool CMenuBillar::BotoPausa(int x, int y, int w, int h) const
{
    Disseny d; CalculaDisseny(w,h,d);
    const Rect r{d.w-132.0f*d.s, 16.0f*d.s, 116.0f*d.s, 42.0f*d.s};
    return r.Conte((float)x,(float)y);
}

void CMenuBillar::DibuixaHUDIntern(const Disseny& d, const HUDInfo& info)
{
	const float s = d.s;
	const float marge = 16.0f * s;
	const float separacio = 10.0f * s;
	const float ample = (d.w - marge * 2.0f - separacio) * 0.5f;
	const float alt = 113.0f * s;
	const float actiu[4] = { 0.10f, 0.34f, 0.24f, 0.94f };
	const float voraFosca[4] = { 0.17f, 0.24f, 0.21f, 1.0f };
	for (int j = 1; j <= 2; ++j)
	{
		const bool act = info.jugador == j;
		const Rect r{ marge + (j - 1) * (ample + separacio), marge, ample, alt };
		RectArrodonit(r, act ? actiu : COLOR_BARRA, act ? COLOR_DAURAT : voraFosca, 12.0f * s, act ? 2.0f * s : 1.0f * s);
		DibuixaText(info.escacs ? (j == 1 ? L"IVORI" : L"OBSIDIANA") : (j == 1 ? L"JUGADOR 1" : L"JUGADOR 2"), r.x + 20.0f * s, r.y + 23.0f * s, Px(20.0f, s), true, COLOR_BLANC, 0);
		const int grup = j == 1 ? info.grup1 : info.grup2;
		const int restants = j == 1 ? info.restants1 : info.restants2;
		const wchar_t* grupText = grup == 1 ? L"LLISES" : grup == 2 ? L"RATLLADES" : L"SENSE ASSIGNAR";
		DibuixaText(info.escacs ? L"CHESS BILLIARDS" : grupText, r.x + 20.0f * s, r.y + 52.0f * s, Px(16.0f, s), false, COLOR_GRIS, 0);
		DibuixaText(info.escacs ? std::to_wstring(j == 1 ? info.punts1 : info.punts2) + L" PTS" : std::to_wstring(restants) + L" / 7", r.x + r.w - 20.0f * s, r.y + 37.0f * s, Px(25.0f, s), true, act ? COLOR_DAURAT : COLOR_BLANC, 2);
		const float colors[7][4] = {
			{ 0.95f, 0.75f, 0.12f, 1.0f }, { 0.12f, 0.40f, 0.83f, 1.0f },
			{ 0.84f, 0.16f, 0.16f, 1.0f }, { 0.45f, 0.23f, 0.68f, 1.0f },
			{ 0.94f, 0.43f, 0.11f, 1.0f }, { 0.13f, 0.52f, 0.35f, 1.0f },
			{ 0.53f, 0.18f, 0.15f, 1.0f }
		};
		const float apagada[4] = { 0.25f, 0.30f, 0.27f, 0.75f };
		const int primer = grup == 2 || (grup == 0 && j == 2) ? 9 : 1;
		for (int i = 0; i < 7; ++i)
		{
			const int bola = primer + i;
			const Rect cercle{ r.x + (20.0f + i * 30.0f) * s, r.y + 73.0f * s, 22.0f * s, 22.0f * s };
			RectArrodonit(cercle, info.embocades[bola] ? apagada : (info.escacs ? (j==1 ? COLOR_BLANC : COLOR_PANELL) : colors[i]), COLOR_DAURAT, 11.0f * s, 1.0f * s);
			DibuixaText(std::to_wstring(bola), cercle.x + cercle.w * 0.5f, cercle.y + cercle.h * 0.5f,
				Px(13.0f, s), true, info.embocades[bola] ? COLOR_GRIS : COLOR_BLANC, 1);
		}
	}
	if (!info.missatge.empty())
	{
		const float ampleAvis = (std::min)(680.0f * s, d.w - 40.0f * s);
		const Rect avis{ (d.w - ampleAvis) * 0.5f, 128.0f * s, ampleAvis, 59.0f * s };
		const float vermell[4] = { 0.36f, 0.09f, 0.09f, 0.95f };
		RectArrodonit(avis, info.missatgeFalta ? vermell : COLOR_PANELL, COLOR_DAURAT, 12.0f * s, 2.0f * s);
		DibuixaText(info.missatge, d.w * 0.5f, avis.y + 30.0f * s,
			Px(20.0f, s), true, COLOR_BLANC, 1);
	}
	const float yPeu = d.h - 71.0f * s;
	const Rect peu{ marge, yPeu, d.w - 2.0f * marge, 56.0f * s };
	RectArrodonit(peu, COLOR_BARRA, voraFosca, 10.0f * s, 1.0f * s);
	DibuixaText(L"FORÇA", peu.x + 19.0f * s, yPeu + 19.0f * s, Px(16.0f, s), true, COLOR_GRIS, 0);
	const float ampladaBarra = (std::max)(110.0f * s, peu.w * 0.41f);
	const float xBarra = peu.x + 100.0f * s;
	const float fraccio = (std::max)(0.0f, (std::min)(1.0f, info.forca));
	Pla(xBarra, yPeu + 14.0f * s, ampladaBarra, 12.0f * s, voraFosca);
	Pla(xBarra, yPeu + 14.0f * s, ampladaBarra * fraccio, 12.0f * s, COLOR_DAURAT);
	DibuixaText(info.superior ? L"CÀMERA SUPERIOR" : L"CÀMERA DE TIR", peu.x + peu.w * 0.61f, yPeu + 21.0f * s, Px(15.0f, s), false, COLOR_BLANC, 0);
	const int minuts = info.segons / 60;
	const int segons = info.segons % 60;
	wchar_t temps[32];
	swprintf_s(temps, 32, L"%02d:%02d", minuts, segons);
	DibuixaText(temps, peu.x + peu.w - 22.0f * s, yPeu + 22.0f * s, Px(19.0f, s), true, COLOR_DAURAT, 2);
	const wchar_t* indicacio = info.tirsDisponibles > 1 ?
		(info.blancaEnMa ? L"FALTA: 2 TIRS. MOU LA BLANCA A LA LINIA" : L"TENS 2 TIRS") :
		(info.blancaEnMa ? L"BLANCA EN MA: ARROSSEGA SOBRE LA LINIA (I/K)" : L"ESC: PAUSA  ·  R: REINICIAR");
	DibuixaText(info.exploracio ? L"WASD: CAMINAR | FLETXES: GIRAR | C: SORTIR" : (info.escacs ? L"C: EXPLORAR SALA | ESC: PAUSA" : indicacio), peu.x + 19.0f*s, yPeu + 43.0f*s, Px(13.0f,s), false, COLOR_GRIS, 0);
    const Rect pausa{d.w-132.0f*s,16.0f*s,116.0f*s,42.0f*s};
    RectArrodonit(pausa,COLOR_PANELL,COLOR_DAURAT,8.0f*s,1.5f*s);
    DibuixaText(L"II  PAUSA",pausa.x+pausa.w*0.5f,pausa.y+pausa.h*0.5f,Px(16.0f,s),true,COLOR_BLANC,1);
}

void CMenuBillar::DibuixaControls(const Disseny& d)
{
	const float s = d.s;
	const Rect& p = d.panell;

	RectArrodonit(p, COLOR_PANELL, COLOR_VORA_PANELL, 20.0f * s, 2.0f * s);
	DibuixaText(L"CONTROLS", d.w * 0.5f, p.y + 48.0f * s, Px(50.0f, s), true, COLOR_DAURAT, 1);
	Pla(p.x + 40.0f * s, p.y + 90.0f * s, p.w - 80.0f * s, 2.0f * s, COLOR_LINIA);

	const float xTecla = p.x + 40.0f * s;
	const float xAccio = p.x + 420.0f * s;
	for (int i = 0; i < N_CONTROLS; ++i)
	{
		const float yc = p.y + (CAPCALERA_PANELL + ALTURA_FILA_CONTROLS * (i + 0.5f)) * s;
		DibuixaText(CONTROLS[i].tecla, xTecla, yc, Px(22.0f, s), true, COLOR_DAURAT, 0);
		DibuixaText(CONTROLS[i].accio, xAccio, yc, Px(22.0f, s), false, COLOR_BLANC, 0);
	}

	DibuixaBoto(d.boto[0], ETIQUETA_TORNAR, m_anim[0], s);
}

void CMenuBillar::DibuixaCredits(const Disseny& d)
{
	const float s = d.s;
	const Rect& p = d.panell;

	RectArrodonit(p, COLOR_PANELL, COLOR_VORA_PANELL, 20.0f * s, 2.0f * s);
	DibuixaText(L"CRÈDITS", d.w * 0.5f, p.y + 48.0f * s, Px(50.0f, s), true, COLOR_DAURAT, 1);
	Pla(p.x + 40.0f * s, p.y + 90.0f * s, p.w - 80.0f * s, 2.0f * s, COLOR_LINIA);

	float y = p.y + CAPCALERA_PANELL * s;
	for (int i = 0; i < N_CREDITS; ++i)
	{
		const float alcada = AlturaLiniaCredits(CREDITS[i].tipus) * s;
		if (CREDITS[i].tipus == 1)
			DibuixaText(CREDITS[i].text, d.w * 0.5f, y + alcada * 0.5f, Px(20.0f, s), true, COLOR_DAURAT, 1);
		else if (CREDITS[i].tipus == 2)
			DibuixaText(CREDITS[i].text, d.w * 0.5f, y + alcada * 0.5f, Px(24.0f, s), false, COLOR_BLANC, 1);
		y += alcada;
	}

	DibuixaBoto(d.boto[0], ETIQUETA_TORNAR, m_anim[0], s);
}

void CMenuBillar::DibuixaBoto(const Rect& r, const wchar_t* etiqueta, float anim, float s)
{
	// El boto creix una mica quan esta ressaltat
	const float creix = 6.0f * s * anim;
	const Rect rr = { r.x - creix, r.y - creix * 0.5f, r.w + 2.0f * creix, r.h + creix };

	float fons[4], vora[4];
	Barreja(COLOR_BOTO, COLOR_BOTO_SEL, anim, fons);
	Barreja(COLOR_VORA_BOTO, COLOR_VORA_SEL, anim, vora);

	const Rect ombra = { rr.x, rr.y + 5.0f * s, rr.w, rr.h };
	RectArrodonit(ombra, COLOR_OMBRA_BOTO, COLOR_OMBRA_BOTO, 14.0f * s, 0.0f);
	RectArrodonit(rr, fons, vora, 14.0f * s, 2.5f * s);

	float colorText[4];
	Barreja(COLOR_BLANC, COLOR_DAURAT, anim, colorText);
	DibuixaText(etiqueta, rr.x + rr.w * 0.5f, rr.y + rr.h * 0.5f, Px(28.0f, s), true, colorText, 1);
}

/* ------------------------------------------------------------------------- */
/*                         PRIMITIVES 2D (OpenGL)                            */
/* ------------------------------------------------------------------------- */

bool CMenuBillar::InicialitzaGL()
{
	if (m_programa != 0) return true;
	if (m_errorGL) return false;

	const GLuint vs = CompilaShader(GL_VERTEX_SHADER, VERTEX_MENU);
	const GLuint fs = CompilaShader(GL_FRAGMENT_SHADER, FRAGMENT_MENU);
	if (vs == 0 || fs == 0)
	{
		if (vs != 0) glDeleteShader(vs);
		if (fs != 0) glDeleteShader(fs);
		m_errorGL = true;
		return false;
	}

	GLuint programa = glCreateProgram();
	glAttachShader(programa, vs);
	glAttachShader(programa, fs);
	glLinkProgram(programa);
	glDeleteShader(vs);
	glDeleteShader(fs);

	GLint ok = 0;
	glGetProgramiv(programa, GL_LINK_STATUS, &ok);
	if (!ok)
	{
		char registre[1024];
		glGetProgramInfoLog(programa, sizeof(registre), NULL, registre);
		::OutputDebugStringA("MenuBillar: error enllacant el programa:\n");
		::OutputDebugStringA(registre);
		glDeleteProgram(programa);
		m_errorGL = true;
		return false;
	}

	m_uRect = glGetUniformLocation(programa, "uRect");
	m_uViewport = glGetUniformLocation(programa, "uViewport");
	m_uMode = glGetUniformLocation(programa, "uMode");
	m_uColor = glGetUniformLocation(programa, "uColor");
	m_uColor2 = glGetUniformLocation(programa, "uColor2");
	m_uRadi = glGetUniformLocation(programa, "uRadi");
	m_uVora = glGetUniformLocation(programa, "uVora");
	m_uTex = glGetUniformLocation(programa, "uTex");

	// Quadrat unitat (0..1) com a TRIANGLE_STRIP
	const float vertexs[8] = { 0.0f, 0.0f,  1.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f };

	GLint vaoAnt = 0, vboAnt = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vaoAnt);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &vboAnt);

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexs), vertexs, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*)0);

	glBindVertexArray((GLuint)vaoAnt);
	glBindBuffer(GL_ARRAY_BUFFER, (GLuint)vboAnt);

	m_programa = programa;
	return true;
}

void CMenuBillar::Allibera()
{
	NetejaTextos();
	if (m_vbo != 0) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
	if (m_vao != 0) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
	if (m_programa != 0) { glDeleteProgram(m_programa); m_programa = 0; }
	m_errorGL = false;
}

void CMenuBillar::Pla(float x, float y, float w, float h, const float color[4])
{
	glUniform1i(m_uMode, 0);
	glUniform4f(m_uRect, x, y, w, h);
	glUniform4fv(m_uColor, 1, color);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void CMenuBillar::RectArrodonit(const Rect& r, const float colorFons[4], const float colorVora[4],
	float radi, float gruixVora)
{
	glUniform1i(m_uMode, 2);
	glUniform4f(m_uRect, r.x, r.y, r.w, r.h);
	glUniform4fv(m_uColor, 1, colorFons);
	glUniform4fv(m_uColor2, 1, colorVora);
	glUniform1f(m_uRadi, radi);
	glUniform1f(m_uVora, gruixVora);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void CMenuBillar::DibuixaText(const std::wstring& text, float x, float yCentre, int pxAlt, bool negreta,
	const float color[4], int alineacio)
{
	if (text.empty()) return;
	const Text* t = ObteText(text, pxAlt, negreta);
	if (t == NULL) return;

	float x0 = x;
	if (alineacio == 1) x0 = x - t->w * 0.5f;
	else if (alineacio == 2) x0 = x - (float)t->w;

	// Ajust vertical: yCentre es el centre visual de les lletres (no de la caixa de la textura)
	float y0 = yCentre - 4.0f - 0.76f * (float)pxAlt;

	x0 = std::floor(x0 + 0.5f);
	y0 = std::floor(y0 + 0.5f);

	glBindTexture(GL_TEXTURE_2D, t->tex);
	glUniform1i(m_uMode, 1);
	glUniform4f(m_uRect, x0, y0, (float)t->w, (float)t->h);
	glUniform4fv(m_uColor, 1, color);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

const CMenuBillar::Text* CMenuBillar::ObteText(const std::wstring& text, int pxAlt, bool negreta)
{
	const std::wstring clau = text + L"|" + std::to_wstring(pxAlt) + (negreta ? L"B" : L"N");
	std::map<std::wstring, Text>::iterator it = m_textos.find(clau);
	if (it != m_textos.end()) return &it->second;

	std::vector<unsigned char> pixels;
	int ampl = 0, alt = 0;
	if (!GeneraBitmapText(text, pxAlt, negreta, pixels, ampl, alt)) return NULL;

	if (m_textos.size() > 200) NetejaTextos();		// Evita que la cache creixi sense limit

	Text t;
	t.w = ampl;
	t.h = alt;
	t.tex = 0;

	GLint textureAnt = 0, alineacioAnt = 4;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureAnt);
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &alineacioAnt);

	glGenTextures(1, &t.tex);
	glBindTexture(GL_TEXTURE_2D, t.tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ampl, alt, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, alineacioAnt);
	glBindTexture(GL_TEXTURE_2D, (GLuint)textureAnt);

	std::pair<std::map<std::wstring, Text>::iterator, bool> r = m_textos.insert(std::make_pair(clau, t));
	return &r.first->second;
}

void CMenuBillar::NetejaTextos()
{
	for (std::map<std::wstring, Text>::iterator it = m_textos.begin(); it != m_textos.end(); ++it)
		if (it->second.tex != 0) glDeleteTextures(1, &it->second.tex);
	m_textos.clear();
}
