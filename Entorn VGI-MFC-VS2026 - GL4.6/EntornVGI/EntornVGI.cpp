
// EntornVGI.cpp: define los comportamientos de las clases para la aplicación.
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "EntornVGI.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "EntornVGIDoc.h"
#include "EntornVGIView.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Variables per gestionar la musica
static ma_engine engine;
static ma_sound musica;

static bool audioInicialitzat = false;
static bool musicaInicialitzada = false;

//variables per gestionar so
static ma_sound soXocBoles;
static ma_sound soCopTac;
static ma_sound soXocBanda;
static ma_sound soBolaForat;

static bool xocBolesInicialitzat = false;
static bool copTacInicialitzat = false;
static bool xocBandaInicialitzat = false;
static bool bolaForatInicialitzat = false;

// CEntornVGIApp

BEGIN_MESSAGE_MAP(CEntornVGIApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CEntornVGIApp::OnAppAbout)
	// Comandos de documento estándar basados en archivo
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// Comando de configuración de impresión estándar
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// Construcción de CEntornVGIApp

CEntornVGIApp::CEntornVGIApp() noexcept
{
	m_bHiColorIcons = TRUE;


	m_nAppLook = 0;
	// admite Administrador de reinicio
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// Si la aplicación se compila para ser compatible con Common Language Runtime (/clr):
	//     1) Esta configuración adicional es necesaria para que la compatibilidad con el Administrador de reinicio funcione correctamente.
	//     2) En el proyecto, debe agregar una referencia a System.Windows.Forms para poder compilarlo.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: reemplace la cadena de identificador de aplicación siguiente por una cadena de identificador único; el formato
	// recomendado para la cadena es NombreCompañía.NombreProducto.Subproducto.InformaciónDeVersión
	SetAppID(_T("EntornVGI.AppID.NoVersion"));

	// TODO: agregar aquí el código de construcción,
	// Colocar toda la inicialización importante en InitInstance
}

// Único objeto CEntornVGIApp

CEntornVGIApp theApp;


void IniciarMusica()
{
	// Inicialitzar el motor d'audio
	if (ma_engine_init(NULL, &engine) != MA_SUCCESS)
	{
		TRACE("Error inicialitzant l'audio\n");
		return;
	}

	audioInicialitzat = true;

	// Carregar la canco
	if (ma_sound_init_from_file(
		&engine,
		"Musica\\billar.mp3",
		MA_SOUND_FLAG_STREAM,
		NULL,
		NULL,
		&musica) != MA_SUCCESS)
	{
		TRACE("Error carregant la musica\n");

		ma_engine_uninit(&engine);
		audioInicialitzat = false;
		return;
	}

	musicaInicialitzada = true;

	// Activar reproduccio en bucle
	ma_sound_set_looping(&musica, MA_TRUE);

	// Comencar a reproduir
	ma_sound_start(&musica);
}
void AturarMusica()
{
	// Alliberem els efectes de so
	if (xocBolesInicialitzat)
	{
		ma_sound_uninit(&soXocBoles);
		xocBolesInicialitzat = false;
	}

	if (copTacInicialitzat)
	{
		ma_sound_uninit(&soCopTac);
		copTacInicialitzat = false;
	}

	if (xocBandaInicialitzat)
	{
		ma_sound_uninit(&soXocBanda);
		xocBandaInicialitzat = false;
	}

	if (bolaForatInicialitzat)
	{
		ma_sound_uninit(&soBolaForat);
		bolaForatInicialitzat = false;
	}

	// Alliberem la música
	if (musicaInicialitzada)
	{
		ma_sound_uninit(&musica);
		musicaInicialitzada = false;
	}

	// Tanquem el motor d'àudio
	if (audioInicialitzat)
	{
		ma_engine_uninit(&engine);
		audioInicialitzat = false;
	}
}

//EFECTES DE SO
void IniciarEfectesSo()
{
	if (!audioInicialitzat)
		return;

	if (ma_sound_init_from_file(
		&engine,
		"Sons\\xoc_boles.wav",
		MA_SOUND_FLAG_DECODE,
		NULL, NULL,
		&soXocBoles) == MA_SUCCESS)
	{
		xocBolesInicialitzat = true;
	}

	if (ma_sound_init_from_file(
		&engine,
		"Sons\\cop_tac.wav",
		MA_SOUND_FLAG_DECODE,
		NULL, NULL,
		&soCopTac) == MA_SUCCESS)
	{
		copTacInicialitzat = true;
	}

	if (ma_sound_init_from_file(
		&engine,
		"Sons\\xoc_banda.wav",
		MA_SOUND_FLAG_DECODE,
		NULL, NULL,
		&soXocBanda) == MA_SUCCESS)
	{
		xocBandaInicialitzat = true;
	}

	if (ma_sound_init_from_file(
		&engine,
		"Sons\\bola_forat.wav",
		MA_SOUND_FLAG_DECODE,
		NULL, NULL,
		&soBolaForat) == MA_SUCCESS)
	{
		bolaForatInicialitzat = true;
	}
}

void ReproduirEfecte(ma_sound* so, bool inicialitzat)
{
	if (!inicialitzat || so == NULL)
		return;

	// Aturem la reproducció anterior d'aquest efecte
	ma_sound_stop(so);

	// Tornem al principi del fitxer
	ma_sound_seek_to_pcm_frame(so, 0);

	// Reproduïm l'efecte
	ma_sound_start(so);
}

void SoXocBoles()
{
	ReproduirEfecte(&soXocBoles, xocBolesInicialitzat);
}

void SoCopTac()
{
	ReproduirEfecte(&soCopTac, copTacInicialitzat);
}

void SoXocBanda()
{
	ReproduirEfecte(&soXocBanda, xocBandaInicialitzat);
}

void SoBolaForat()
{
	ReproduirEfecte(&soBolaForat, bolaForatInicialitzat);
}


// Inicialización de CEntornVGIApp

BOOL CEntornVGIApp::InitInstance()
{
	// Windows XP requiere InitCommonControlsEx() si un manifiesto de
	// aplicación especifica el uso de ComCtl32.dll versión 6 o posterior para habilitar
	// estilos visuales.  De lo contrario, se generará un error al crear ventanas.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Establecer para incluir todas las clases de control comunes que desee utilizar
	// en la aplicación.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// Inicializar bibliotecas OLE
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction();

	// Se necesita AfxInitRichEdit2() para usar el control RichEdit
	// AfxInitRichEdit2();

	// Inicialización estándar
	// Si no utiliza estas funcionalidades y desea reducir el tamaño
	// del archivo ejecutable final, debe quitar
	// las rutinas de inicialización específicas que no necesite
	// Cambie la clave del Registro en la que se almacena la configuración
	// TODO: debe modificar esta cadena para que contenga información correcta
	// como el nombre de su compañía u organización
	SetRegistryKey(_T("Aplicaciones generadas con el Asistente para aplicaciones local"));
	LoadStdProfileSettings(4);  // Cargar opciones de archivo INI estándar (incluidas las de la lista MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Registrar las plantillas de documento de la aplicación.  Las plantillas de documento
	//  sirven como conexión entre documentos, ventanas de marco y vistas
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_EntornVGITYPE,
		RUNTIME_CLASS(CEntornVGIDoc),
		RUNTIME_CLASS(CChildFrame), // Marco MDI secundario personalizado
		RUNTIME_CLASS(CEntornVGIView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// Crear ventana de marco principal MDI
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;


	// Analizar línea de comandos para comandos Shell estándar, DDE, Archivo Abrir
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// Enviar comandos especificados en la línea de comandos.  Devolverá FALSE si
	// la aplicación se inició con los modificadores /RegServer, /Register, /Unregserver o /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// Iniciar musica de fons
	IniciarMusica();
	IniciarEfectesSo();

	// La ventana principal se ha inicializado; mostrarla y actualizarla
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

int CEntornVGIApp::ExitInstance()
{
	AturarMusica();

	//TODO: controlar recursos adicionales que se hayan podido agregar
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// Controladores de mensajes de CEntornVGIApp


// Cuadro de diálogo CAboutDlg utilizado para el comando Acerca de

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

	// Datos del cuadro de diálogo
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Compatibilidad con DDX/DDV

	// Implementación
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// Comando de la aplicación para ejecutar el cuadro de diálogo
void CEntornVGIApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// Métodos para cargar o guardar personalizaciones de CEntornVGIApp

void CEntornVGIApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CEntornVGIApp::LoadCustomState()
{
}

void CEntornVGIApp::SaveCustomState()
{
}

// Controladores de mensajes de CEntornVGIApp
