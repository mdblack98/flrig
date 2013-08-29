#include "config.h"

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/x.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Menu_Item.H>

#ifdef WIN32
#  include "flrigrc.h"
#  include "compat.h"
#  define dirent fl_dirent_no_thanks
#endif

#include <FL/filename.H>

#ifdef __MINGW32__
#	if FLRIG_FLTK_API_MAJOR == 1 && FLRIG_FLTK_API_MINOR < 3
#		undef dirent
#		include <dirent.h>
#	endif
#else
#	include <dirent.h>
#endif

#include <FL/x.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Image.H>

//#include "images/flrig.xpm"
#include "support.h"
#include "dialogs.h"
#include "rig.h"
#include "status.h"
#include "debug.h"
#include "util.h"
#include "gettext.h"
#include "xml_io.h"

#include "flrig_icon.cxx"

int parse_args(int argc, char **argv, int& idx);

Fl_Double_Window *mainwindow;

string HomeDir;
string RigHomeDir;
string TempDir;
string defFileName;
string title;

pthread_t *serial_thread = 0;
pthread_t *digi_thread = 0;

pthread_mutex_t mutex_serial = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_xmlrpc = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_queX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_queA = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_queB = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_ptt = PTHREAD_MUTEX_INITIALIZER;

bool RIG_DEBUG = false;
bool XML_DEBUG = false;
bool EXPAND_CONTROLS = false;

//----------------------------------------------------------------------
void about()
{
	string msg = "\
%s\n\
Version %s\n\
copyright W1HKJ  <w1hkj@@w1hkj.com>\n\
Developer:  Dave,  W1HKJ";
	fl_message(msg.c_str(), PACKAGE_TARNAME, PACKAGE_VERSION);
}

void visit_URL(void* arg)
{
	const char* url = reinterpret_cast<const char *>(arg);
#ifndef __WOE32__
	const char* browsers[] = {
#  ifdef __APPLE__
		getenv("FLDIGI_BROWSER"), // valid for any OS - set by user
		"open"                    // OS X
#  else
		"fl-xdg-open",            // Puppy Linux
		"xdg-open",               // other Unix-Linux distros
		getenv("FLDIGI_BROWSER"), // force use of spec'd browser
		getenv("BROWSER"),        // most Linux distributions
		"sensible-browser",
		"firefox",
		"mozilla"                 // must be something out there!
#  endif
	};
	switch (fork()) {
	case 0:
#  ifndef NDEBUG
		unsetenv("MALLOC_CHECK_");
		unsetenv("MALLOC_PERTURB_");
#  endif
		for (size_t i = 0; i < sizeof(browsers)/sizeof(browsers[0]); i++)
			if (browsers[i])
				execlp(browsers[i], browsers[i], url, (char*)0);
		exit(EXIT_FAILURE);
	case -1:
		fl_alert(_("Could not run a web browser:\n%s\n\n"
			 "Open this URL manually:\n%s"),
			 strerror(errno), url);
	}
#else
	// gurgle... gurgle... HOWL
	// "The return value is cast as an HINSTANCE for backward
	// compatibility with 16-bit Windows applications. It is
	// not a true HINSTANCE, however. The only thing that can
	// be done with the returned HINSTANCE is to cast it to an
	// int and compare it with the value 32 or one of the error
	// codes below." (Error codes omitted to preserve sanity).
	if ((int)ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL) <= 32)
		fl_alert(_("Could not open url:\n%s\n"), url);
#endif
}

//----------------------------------------------------------------------

extern void saveFreqList();

void * flrig_terminate(void) {
	std::cerr << "terminating" << std::endl;
	fl_message("Closing flrig");
	cbExit();
	return 0;
}

void showEvents(void *)
{
	debug::show();
}

#if defined(__WIN32__) && defined(PTW32_STATIC_LIB)
static void ptw32_cleanup(void)
{
	(void)pthread_win32_process_detach_np();
}

void ptw32_init(void)
{
	(void)pthread_win32_process_attach_np();
	atexit(ptw32_cleanup);
}
#endif // __WIN32__

#define KNAME "Flrig"
#if !defined(__WIN32__) && !defined(__APPLE__)
Pixmap  Rig_icon_pixmap;

void make_pixmap(Pixmap *xpm, const char **data)
{
	Fl_Window w(0,0, KNAME);
	w.xclass(KNAME);
	w.show();
	w.make_current();
	Fl_Pixmap icon(data);
	int maxd = (icon.w() > icon.h()) ? icon.w() : icon.h();
	*xpm = fl_create_offscreen(maxd, maxd);
	fl_begin_offscreen(*xpm);
	fl_color(FL_BACKGROUND_COLOR);
	fl_rectf(0, 0, maxd, maxd);
	icon.draw(maxd - icon.w(), maxd - icon.h());
	fl_end_offscreen();
}

#endif

static void checkdirectories(void)
{
	struct {
		string& dir;
		const char* suffix;
		void (*new_dir_func)(void);
	} dirs[] = {
		{ RigHomeDir, 0, 0 }
	};

	int r;
	for (size_t i = 0; i < sizeof(dirs)/sizeof(*dirs); i++) {
		if (dirs[i].suffix)
			dirs[i].dir.assign(RigHomeDir).append(dirs[i].suffix).append("/");

		if ((r = mkdir(dirs[i].dir.c_str(), 0777)) == -1 && errno != EEXIST) {
			cerr << _("Could not make directory") << ' ' << dirs[i].dir
				 << ": " << strerror(errno) << '\n';
			exit(EXIT_FAILURE);
		}
		else if (r == 0 && dirs[i].new_dir_func)
			dirs[i].new_dir_func();
	}
}

void exit_main(Fl_Widget *w)
{
	if (Fl::event_key() == FL_Escape)
		return;
	cbExit();
}

extern void open_rig_socket();
extern bool run_digi_loop;

void expand_controls(void*)
{
	show_controls();
}

void close_controls(void*)
{
//	show_controls();
	if (progStatus.UIsize == wide_ui) {
		if (EXPAND_CONTROLS && selrig->has_extras) return;
		btn_show_controls->label("@-22->");
		btn_show_controls->redraw_label();
		grpTABS->hide();
		mainwindow->resizable(grpTABS);
		mainwindow->size(progStatus.mainW, 150);
		mainwindow->size_range(735, 150, 0, 150);
	} else if (progStatus.UIsize == small_ui) {
		if (EXPAND_CONTROLS && selrig->has_extras)
			Fl::add_timeout(1.0, expand_controls);
	}
}

void startup(void*)
{
	initStatusConfigDialog();

	Fl::add_timeout(0.0, close_controls);
}

int main (int argc, char *argv[])
{
	std::terminate_handler(flrig_terminate);

	int arg_idx;
	HomeDir.clear();
	RigHomeDir.clear();

	Fl::args(argc, argv, arg_idx, parse_args);
	Fl::set_fonts(0);

	char dirbuf[FL_PATH_MAX + 1];
#ifdef __WIN32__
	string appname = argv[0];
	size_t p = appname.find(":\\NBEMSapps\\");
	if (p == 1) {
		HomeDir.assign(appname.substr(0,13));
		strcpy(dirbuf, HomeDir.c_str());
		strcat(dirbuf, "NBEMS.files\\");
	} else {
		fl_filename_expand(dirbuf, sizeof(dirbuf) -1, "$USERPROFILE");
		HomeDir = dirbuf;
		fl_filename_expand(dirbuf, sizeof(dirbuf) - 1, "$USERPROFILE/flrig.files/");
	}
//	fl_filename_expand(dirbuf, sizeof(dirbuf) -1, "$USERPROFILE");
//	HomeDir = dirbuf;
//	fl_filename_expand(dirbuf, sizeof(dirbuf) - 1, "$USERPROFILE/flrig.files/");
#else
	fl_filename_expand(dirbuf, sizeof(dirbuf) - 1, "$HOME");
	HomeDir = dirbuf;
	fl_filename_expand(dirbuf, sizeof(dirbuf) - 1, "$HOME/.flrig/");
#endif
	if (RigHomeDir.empty())
		RigHomeDir = dirbuf;
	checkdirectories();

	try {
		debug::start(string(RigHomeDir).append("debug_log.txt").c_str());
		time_t t = time(NULL);
// for testing purposes only
		debug::level = debug::DEBUG_LEVEL;
		RIG_DEBUG = true;
//
		LOG(debug::INFO_LEVEL, debug::LOG_OTHER, _("%s log started on %s"), PACKAGE_STRING, ctime(&t));
	}
	catch (const char* error) {
		cerr << error << '\n';
		debug::stop();
		exit(1);
	}

	progStatus.loadLastState();

	if (progStatus.UIsize == small_ui)
		mainwindow = Small_rig_window();
	else
		mainwindow = Wide_rig_window();

	mainwindow->callback(exit_main);

	progStatus.UI_laststate();

	fntbrowser = new Font_Browser;
	dlgMemoryDialog = Memory_Dialog();
	dlgDisplayConfig = DisplayDialog();

	Fl::lock();

#if defined(__WIN32__) && defined(PTW32_STATIC_LIB)
	ptw32_init();
#endif

	bypass_serial_thread_loop = true;
	serial_thread = new pthread_t;
	if (pthread_create(serial_thread, NULL, serial_thread_loop, NULL)) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}

//	progStatus.loadLastState();

	open_rig_xmlrpc();

	digi_thread = new pthread_t;      
	if (pthread_create(digi_thread, NULL, digi_loop, NULL)) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}

	createXcvrDialog();

	btnALC_SWR->image(image_swr);
	sldrRcvSignal->clear();
	sldrFwdPwr->clear();
	sldrALC->clear();
	sldrSWR->clear();

	if (progStatus.UIsize == small_ui)
		mainwindow->resize( progStatus.mainX, progStatus.mainY, mainwindow->w(), btnInitializing->h() + 24 );
	else
		mainwindow->resize( progStatus.mainX, progStatus.mainY, progStatus.mainW, mainwindow->h());

	btnInitializing->show();

	mainwindow->xclass(KNAME);

#if defined(__WOE32__)
#  ifndef IDI_ICON
#    define IDI_ICON 101
#  endif
	mainwindow->icon((char*)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON)));
	mainwindow->show (argc, argv);
#elif !defined(__APPLE__)
	make_pixmap(&Rig_icon_pixmap, flrig_icon);
	mainwindow->icon((char *)Rig_icon_pixmap);
	mainwindow->show(argc, argv);
#else
	mainwindow->show(argc, argv);
#endif

	btn_show_controls->label("@-28->");
	btn_show_controls->redraw_label();

//	show_controls();

	Fl::add_timeout(0.50, startup);

	return Fl::run();

}

int parse_args(int argc, char **argv, int& idx)
{
	if (strcasecmp("--help", argv[idx]) == 0) {
		printf("Usage: \n\
  --help this help text\n\
  --version\n\
  --config-dir <DIR>\n\
  --debug-level N (0..4)\n\
  --rig_debug\n\
  --xml_debug\n\
  --exp (expand menu tab controls)\n\n");
		exit(0);
	} 
	if (strcasecmp("--version", argv[idx]) == 0) {
		printf("Version: "VERSION"\n");
		exit (0);
	}
	if (strcasecmp("--rig_debug", argv[idx]) == 0) {
		RIG_DEBUG = true;
		idx++;
		return 1;
	}
	if (strcasecmp("--xml_debug", argv[idx]) == 0) {
		XML_DEBUG = true;
		idx++;
		return 1;
	}
	if (strcasecmp("--debug-level", argv[idx]) == 0) {
		string level = argv[idx + 1];
		switch (level[0]) {
			case '0': debug::level = debug::QUIET_LEVEL; break;
			case '1': debug::level = debug::ERROR_LEVEL; break;
			case '2': debug::level = debug::WARN_LEVEL; break;
			case '3': debug::level = debug::INFO_LEVEL; break;
			case '4': debug::level = debug::DEBUG_LEVEL; break;
			default : debug::level = debug::WARN_LEVEL;
		}
		idx += 2;
		return 1;
	}
	if (strcasecmp("--config-dir", argv[idx]) == 0) {
		RigHomeDir = argv[idx + 1];
		if (RigHomeDir[RigHomeDir.length()-1] != '/')
			RigHomeDir += '/';
		idx += 2;
		return 1;
	}
	if (strcasecmp("--exp", argv[idx]) == 0) {
		EXPAND_CONTROLS = true;
		idx++;
		return 1;
	}
	return 0;
}
