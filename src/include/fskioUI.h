// generated by Fast Light User Interface Designer (fluid) version 1.0308

#ifndef fskioUI_h
#define fskioUI_h
#include <FL/Fl.H>
#include "config.h"
#include "compat.h"
#include <FL/Fl_Double_Window.H>
extern Fl_Double_Window *fsklog_viewer; 
#include <FL/Fl_Double_Window.H>
#include "status.h"
#include "fsk.h"
#include "ValueSlider.h"
extern Fl_Input2 *FSK_txt_to_send;
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
extern Fl_Button *btn_fskio_config;
extern Fl_Button *btn_fskio_clear;
#include <FL/Fl_Light_Button.H>
extern Fl_Light_Button *btn_fskioSEND;
extern Fl_Button *FSK_btn_msg[12];
Fl_Double_Window* fskio_window();
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Box.H>
#include "flinput2.h"
extern Fl_Input2 *FSK_edit_label[12];
extern Fl_Input2 *FSK_edit_msg[12];
extern Fl_Button *FSK_btn_cancel_edit;
extern Fl_Button *FSK_btn_apply_edit;
extern Fl_Button *FSK_btn_FSK_done_edit;
Fl_Double_Window* FSK_make_message_editor();
#include "combo.h"
extern Fl_ComboBox *select_fskioPORT;
extern Fl_Light_Button *btn_fskioCONNECT;
#include <FL/Fl_Check_Button.H>
extern Fl_Check_Button *btn_fskioCAT;
extern Fl_Check_Button *btn_fskioAUX;
extern Fl_Check_Button *btn_fskioSEP;
extern Fl_Check_Button *btn_fskioSHARED;
extern Fl_Check_Button *btn_fskioINVERTED;
extern Fl_Check_Button *btn_fskioSTOPBITS;
extern Fl_ListBox *listbox_fskioKEYLINE;
#include <FL/Fl_Spinner.H>
extern Fl_Spinner *cntr_fskioPTT;
#include <FL/Fl_Counter.H>
extern Fl_Counter *fsk_idles;
Fl_Double_Window* fskio_config_dialog();
#include <FL/Fl_Menu_Bar.H>
extern Fl_Menu_Bar *FSKlog_menubar;
#include <FL/Fl_Output.H>
extern Fl_Output *txt_fsklog_file;
extern Fl_Group *fsk_grp_qso_data;
extern Fl_Input2 *fsk_op_freq;
extern Fl_Button *btn_fsklog_clear_qso;
extern Fl_Button *btn_fsklog_save_qso;
extern Fl_Button *btn_fsklog_edit_entry;
extern Fl_Button *btn_fsklog_delete_entry;
extern Fl_Input2 *fsk_date;
extern Fl_Input2 *fsk_time;
extern Fl_Input2 *fsk_rst_out;
extern Fl_Input2 *fsk_rst_in;
extern Fl_Input2 *fsk_xchg_in;
extern Fl_Counter *cntr_fsk_log_nbr;
extern Fl_Input2 *fsk_op_name;
extern Fl_Input2 *fsk_op_call;
extern Fl_Button *btn_fsk_datetime;
extern Fl_Button *btn_fsk_time;
extern Fl_Button *btn_fsk_freq;
extern Fl_Button *btn_fsk_call;
extern Fl_Button *btn_fsk_name;
extern Fl_Button *btn_fsk_in;
extern Fl_Button *btn_fsk_out;
extern Fl_Button *btn_fsk_sent_nbr;
extern Fl_Button *btn_fsk_exchange;
#include <FL/Fl_Browser.H>
extern Fl_Browser *brwsr_fsklog_entries;
Fl_Double_Window* new_fsklogbook_dialog();
extern unsigned char menu_FSKlog_menubar_i18n_done;
extern Fl_Menu_Item menu_FSKlog_menubar[];
#define FSKlog_menu (menu_FSKlog_menubar+0)
#define fsklog_fsk_menu_open (menu_FSKlog_menubar+1)
#define fsklog_fsk_menu_new (menu_FSKlog_menubar+2)
#define fsklog_fsk_menu_save (menu_FSKlog_menubar+3)
#define fsklog_fsk_menu_save_as (menu_FSKlog_menubar+4)
#define fsklog_fsk_menu_export_adif (menu_FSKlog_menubar+5)
#define fsklog_fsk_menu_import_adif (menu_FSKlog_menubar+6)
#define fsklog_fsk_menu_dupcheck (menu_FSKlog_menubar+9)
#define fsklog_fsk_menu_leading_zeros (menu_FSKlog_menubar+10)
#define fsklog_fsk_menu_cut_numbers (menu_FSKlog_menubar+11)
#endif
