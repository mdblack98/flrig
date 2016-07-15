// ----------------------------------------------------------------------------
// Copyright (C) 2014
//              David Freese, W1HKJ
//
// This file is part of flrig.
//
// flrig is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// flrig is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------
/*
 * Copyright (c) 2014 Andy Burnett, G0HIX
*/

#include "config.h"

#include "TS990.h"
#include "support.h"

static const char TS990name_[] = "TS-990";

static const char *TS990modes_[] = {
"LSB",    "USB",    "CW",    "FM",     "AM",
"FSK",    "PSK",    "CW-R",  "FSK-R",  "PSK-R",
"LSB-D1", "USB-D1", "FM-D1", "AM-D1",
"LSB-D2", "USB-D2", "FM-D2",  "AM-D2",
"LSB-D3", "USB-D3", "FM-D3",  "AM-D3",  NULL};

static const char TS990_mode_chr[] =  {
'1', '2', '3', '4', '5',
'6', 'A', '7', '9', 'B',
'C', 'D', 'E', 'F',
'G', 'H', 'I', 'J',
'K', 'L', 'M', 'N' };

static const char TS990_mode_type[] = {
'L', 'U', 'U', 'U', 'U',
'U', 'U', 'L', 'U', 'U',
'L', 'U', 'U', 'U',
'L', 'U', 'U', 'U',
'L', 'U', 'U', 'U' };

//==============================================================================
static const char *TS990_empty[] = { "N/A", NULL };

//==============================================================================
// SSB - Width / Shift  filters
//
// BW indicates 0x8000 & (shift << 8) & width
// Maximum assignable BW value 0x910C
// Default BW value 0x9005
//==============================================================================

static const char *TS990_filt_width[] = {
  "50",   "80",  "100",  "150",  "200",
 "250",  "300",  "400",  "500",  "600",
"1000", "1500", "2000", "2200", "2400",
"2600", "2800", "3000", NULL };
static int TS990_WIDTH_bw_vals[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,WVALS_LIMIT};

static const char *TS990_CAT_filt_width[] = {
"00;", "01;", "02;", "03;", "04;",
"05;", "06;", "07;", "08;", "09;",
"10;", "11;", "12;", "13;", "14;",
"15;", "16;", "17;", NULL };

static const char *TS990_filt_width_tooltip = "width";
static const char *TS990_filt_width_label = "W";

static const char *TS990_filt_shift[] = {
"1000", "1100", "1200", "1300", "1400",
"1500", "1600", "1700", "1800", "1900",
"2000", "2100", "2210", NULL };

static const char *TS990_CAT_filt_shift[] = {
"00;", "01;", "02;", "03;", "04;",
"05;", "06;", "07;", "08;", "09;",
"10;", "11;", "12;", NULL };

static const char *TS990_filt_shift_tooltip = "shift";
static const char *TS990_filt_shift_label = "S";

#define DEFAULT_SH_WI     0x8510  // SHIFT 1500, WIDTH 2800
#define DEFAULT_SH_WI_D1  0x850E  // SHIFT 1500, WIDTH 2400
#define DEFAULT_SH_WI_D2  0x850A  // SHIFT 1500, WIDTH 1000
#define DEFAULT_SH_WI_D3  0x8508  // SHIFT 1500, WIDTH 500

//==============================================================================
// SSB, FM, Other SL/SH cut filters
//
// BW indicates 0x8000 & (SH << 8) & SL
//==============================================================================

static const char *TS990_filt_SH[] = {
"1000", "1200", "1400", "1600", "1800",
"2000", "2200", "2400", "2600", "2800",
"3000", "3400", "4000", "5000", NULL };
static int TS990_HI_bw_vals[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,WVALS_LIMIT};

static const char *TS990_CAT_filt_SH[] = {
"00;", "01;", "02;", "03;", "04;",
"05;", "06;", "07;", "08;", "09;",
"10;", "11;", "12;", "13;", NULL };

static const char *TS990_filt_SH_tooltip = "hi cut";
static const char *TS990_filt_SH_label = "H";

static const char *TS990_filt_SL[] = {
"0", "50", "100", "200", "300",
"400", "500", "600", "700", "800",
"900", "1000", NULL };

static const char *TS990_CAT_filt_SL[] = {
"00;", "01;", "02;", "03;", "04;",
"05;", "06;", "07;", "08;", "09;",
"10;", "11;", NULL };

static const char *TS990_filt_SL_tooltip = "lo cut";
static const char *TS990_filt_SL_label = "L";

#define DEFAULT_HI_LO     0x8A03 // LO 200, HI 3000
#define DEFAULT_FM        0x8C05 // LO 400, HI 4000
#define DEFAULT_FM_D1     0x8903 // LO 200, HI 2800
#define DEFAULT_FM_D2     0x8707 // LO 600, HI 2400
#define DEFAULT_FM_D3     0x850B // LO 1000, HI 2000

//==============================================================================
// CW filters
//
// BW indicates 0x8000 & (width << 8) & shift
// Maximum assignable BW value 0x8D20
// Default BW value 0x8810 --> no shift, bandwidth = 500 Hz
//==============================================================================

static const char *TS990_CW_width[] = {
  "50",   "80",  "100",  "150", "200",
 "250",  "300",  "400",  "500", "600",
"1000", "1500", "2000", "2500",  NULL };
static int TS990_CW_bw_vals[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,WVALS_LIMIT};

static const char *TS990_CAT_CW_width[] = {
"00;", "01;", "02;", "03;", "04;",
"05;", "06;", "07;", "08;", "09;",
"10;", "11;", "12;", "13;", NULL };

static const char *TS990_CW_W_tooltip = "width";
static const char *TS990_CW_W_btn_label = "W";

static const char *TS990_CW_shift[] = {
"-800", "-750", "-700", "-650", "-600",
"-550", "-500", "-450", "-400", "-350",
"-300", "-250", "-200", "-150", "-100",
"-50",  "0",    "50",   "100",  "150",
"200",  "250",  "300",  "350",  "400",
"450",  "500",  "550",  "600",  "650",
"700",  "750",  "800", NULL };

static const char *TS990_CAT_CW_shift[] = {
"00;", "01;", "02;", "03;", "04;",
"05;", "06;", "07;", "08;", "09;",
"10;", "11;", "12;", "13;", "14;",
"15;", "16;", "17;", "18;", "19;",
"20;", "21;", "22;", "23;", "24;",
"25;", "26;", "27;", "28;", "29;",
"30;", "31;", "32;", NULL };

static const char *TS990_CW_S_tooltip = "shift";
static const char *TS990_CW_S_btn_label = "S";

#define DEFAULT_CW        0x9008 // SHIFT 0, WIDTH 500

//==============================================================================

static const char *TS990_AM_SL[] = {
"0", "100", "200", "300", NULL };

static const char *TS990_CAT_AM_SL[] = {
"00;", "01;", "02;", "03;", NULL};

static const char *TS990_AM_SL_tooltip = "lo cut";
static const char *TS990_AM_btn_SL_label = "L";

static const char *TS990_AM_SH[] = {
"2500", "3000", "4000", "5000", NULL };
static int TS990_AM_HI_bw_vals[] = { 1,2,3,4,WVALS_LIMIT};

static const char *TS990_CAT_AM_SH[] = {
"00;", "01;", "02;", "03;", NULL};

static const char *TS990_AM_SH_tooltip = "hi cut";
static const char *TS990_AM_btn_SH_label = "H";

#define DEFAULT_AM        0x8102 // LO 100, HI 4000
#define DEFAULT_AM_D1     0x8202 // LO 200, HI 4000 
#define DEFAULT_AM_D2     0x8201 // LO 200, HI 3000
#define DEFAULT_AM_D3     0x8200 // LO 200, HI 2500
//==============================================================================

static const char *TS990_FSK_filt[] = {
"250", "300", "400", "500", "1000", "1500", NULL};
static int TS990_FSK_bw_vals[] = {1,2,3,4,5,6,WVALS_LIMIT};

static const char *TS990_CAT_FSK_filt[] = {
"00;", "01;", "02;", "03;", "04;", "05;", NULL };

#define DEFAULT_FSK       0x03 // WIDTH 300

//==============================================================================

static const char *TS990_PSK_filt[] = {
"50",   "80",  "100",  "150", "200", 
"250",  "300",  "400",  "500", "600",
"1000", "1500", NULL};
static int TS990_PSK_bw_vals[] = {1,2,3,4,5,6,7,8,9,10,11,12,WVALS_LIMIT};

static const char *TS990_CAT_PSK_filt[] = {
"00;", "01;", "02;", "03;", "04;",
"05;", "06;", "07;", "08;", "09;",
"10;", "11;", NULL };

#define DEFAULT_PSK       0x06 // WIDTH 300

//==============================================================================

static GUI rig_widgets[]= {
	{ (Fl_Widget *)btnVol,        2, 125,  50 },
	{ (Fl_Widget *)sldrVOLUME,   54, 125, 156 },
	{ (Fl_Widget *)sldrRFGAIN,   54, 145, 156 },
	{ (Fl_Widget *)btnIFsh,     214, 105,  50 },
	{ (Fl_Widget *)sldrIFSHIFT, 266, 105, 156 },
	{ (Fl_Widget *)btnNotch,    214, 125,  50 },
	{ (Fl_Widget *)sldrNOTCH,   266, 125, 156 },
	{ (Fl_Widget *)sldrMICGAIN, 266, 145, 156 },
	{ (Fl_Widget *)sldrPOWER,   266, 165, 156 },
	{ (Fl_Widget *)btnNR,         2, 165,  50 },
	{ (Fl_Widget *)sldrNR,       54, 165, 156 },
	{ (Fl_Widget *)NULL,          0,   0,   0 }
};

static string menu_0001;

void RIG_TS990::initialize()
{
	rig_widgets[0].W = btnVol;
	rig_widgets[1].W = sldrVOLUME;
	rig_widgets[2].W = sldrRFGAIN;
	rig_widgets[3].W = btnIFsh;
	rig_widgets[4].W = sldrIFSHIFT;
	rig_widgets[5].W = btnNotch;
	rig_widgets[6].W = sldrNOTCH;
	rig_widgets[7].W = sldrMICGAIN;
	rig_widgets[8].W = sldrPOWER;
	rig_widgets[9].W = btnNR;
	rig_widgets[10].W = sldrNR;

	RIG_DEBUG = true;

	cmd = "AC000;";
	sendCommand(cmd);

	cmd = "EX00100;";
	if (wait_char(';', 11, 100, "read ex 00100", ASC) == 11)
		menu_0001 = replystr;
	cmd = "EX00100 00"; // turn off beeps
	sendCommand(cmd);

	selectA();
	get_preamp();
	get_attenuator();

	read_menu_0607();
	read_menu_0608();
	set_menu_0607(false); // SSB uses lower/upper cutoff frequencies
	set_menu_0608(true);  // SSB data uses shift/width frequencies

	A_default_SH_WI    = DEFAULT_SH_WI;
	A_default_SH_WI_D1 = DEFAULT_SH_WI_D1;
	A_default_SH_WI_D2 = DEFAULT_SH_WI_D2;
	A_default_SH_WI_D3 = DEFAULT_SH_WI_D3;
	A_default_HI_LO    = DEFAULT_HI_LO;
	A_default_FM       = DEFAULT_FM;
	A_default_FM_D1    = DEFAULT_FM_D1;
	A_default_FM_D2    = DEFAULT_FM_D2;
	A_default_FM_D3    = DEFAULT_FM_D3;
	A_default_CW       = DEFAULT_CW;
	A_default_AM       = DEFAULT_AM;
	A_default_AM_D1    = DEFAULT_AM_D1;
	A_default_AM_D2    = DEFAULT_AM_D2;
	A_default_AM_D3    = DEFAULT_AM_D3;
	A_default_FSK      = DEFAULT_FSK;
	A_default_PSK      = DEFAULT_PSK;

	B_default_SH_WI    = DEFAULT_SH_WI;
	B_default_SH_WI_D1 = DEFAULT_SH_WI_D1;
	B_default_SH_WI_D2 = DEFAULT_SH_WI_D2;
	B_default_SH_WI_D3 = DEFAULT_SH_WI_D3;
	B_default_HI_LO    = DEFAULT_HI_LO;
	B_default_FM       = DEFAULT_FM;
	B_default_FM_D1    = DEFAULT_FM_D1;
	B_default_FM_D2    = DEFAULT_FM_D2;
	B_default_FM_D3    = DEFAULT_FM_D3;
	B_default_CW       = DEFAULT_CW;
	B_default_AM       = DEFAULT_AM;
	B_default_AM_D1    = DEFAULT_AM_D1;
	B_default_AM_D2    = DEFAULT_AM_D2;
	B_default_AM_D3    = DEFAULT_AM_D3;
	B_default_FSK      = DEFAULT_FSK;
	B_default_PSK      = DEFAULT_PSK;
}

void RIG_TS990::shutdown()
{
	set_menu_0607(save_menu_0607);
	set_menu_0608(save_menu_0608);
	cmd = menu_0001; // restore beep level
	sendCommand(cmd);
}

//==============================================================================

RIG_TS990::RIG_TS990() {

	name_ = TS990name_;
	modes_ = TS990modes_;

	B.freq = A.freq = 14107500;

	A.imode = USB;
	A.iBW = A_default_HI_LO;
	B.imode = USB;
	B.iBW = B_default_HI_LO;

	bandwidths_ = TS990_filt_SH;
	bw_vals_    = TS990_HI_bw_vals;

	dsp_SL      = TS990_filt_SL;
	SL_tooltip  = TS990_filt_SL_tooltip;
	SL_label    = TS990_filt_SL_label;

	dsp_SH      = TS990_filt_SH;
	SH_tooltip  = TS990_filt_SH_label;
	SH_label    = TS990_filt_SH_label;

	widgets = rig_widgets;

	comm_baudrate = BR115200;
	stopbits = 1;
	comm_retries = 2;
	comm_wait = 5;
	comm_timeout = 50;
	comm_rtscts = true;
	comm_rtsplus = false;
	comm_dtrplus = false;
	comm_catptt = true;
	comm_rtsptt = false;
	comm_dtrptt = false;

	can_change_alt_vfo = true;

	nb_level = 1;

	has_auto_notch =
	has_notch_control =
	has_sql_control =
	has_swr_control =
	has_noise_reduction =
	has_noise_reduction_control =
	has_alc_control =
	has_dsp_controls =
	has_smeter =
	has_power_out =
	has_split =
	has_split_AB =
	has_noise_control =
	has_micgain_control =
	has_rf_control =
	has_volume_control =
	has_power_control =
	has_tune_control =
	has_attenuator_control =
	has_preamp_control =
	has_mode_control =
	has_bandwidth_control =
	has_ifshift_control =
	has_ptt_control = true;

	rxtxa = true;

	precision = 1;
	ndigits = 8;
}

//==============================================================================

void RIG_TS990::selectA()
{
	cmd = "CB0;";
	sendCommand(cmd);
	showresp(INFO, ASC, "Rx A, Tx A", cmd, "");
	rxtxa = true;
	get_attenuator();
	get_preamp();
	get_smeter();
	get_power_out();
	get_volume_control();
	get_rf_gain();
	get_noise_reduction_val();
	get_auto_notch();
	get_agc();
}

void RIG_TS990::selectB()
{
	cmd = "CB1;";
	sendCommand(cmd);
	showresp(INFO, ASC, "Rx B, Tx B", cmd, "");
	rxtxa = false;
	get_attenuator();
	get_preamp();
	get_smeter();
	get_power_out();
	get_volume_control();
	get_rf_gain();
	get_noise_reduction_val();
	get_auto_notch();
	get_agc();
}

//==============================================================================
//  Get Attenuator
//  The TS990 actually has 4 attenuator settings.
//  RA00; = Off,  RA01; = 6dB,  RA02; = 12dB,  RA03; = 18dB
//==============================================================================

void RIG_TS990::set_attenuator(int val) {

	if (useB) {
		if (att_level == 0) {			// If attenuator level = 0 (off)
			att_level = 1;				// then turn it on, at 6dB
			cmd = "RA11;";				// this is the command...
			atten_label("Att 6", true);	// show it in the button...
		}
		else if (att_level == 1) {		// If attenuator level = 1 (6dB)
			att_level = 2;				// then make it 12dB
			cmd = "RA12;";
			atten_label("Att 12", true);
		} else if (att_level == 2) {		// if it's 12dB
			att_level = 3;				// go to 18dB
			cmd = "RA13;";
			atten_label("Att 18", true);
		} else if (att_level == 3) {		// If it's 18dB
			att_level = 0;				// Loop back to none.
			cmd = "RA10;";
			atten_label("Att", false);
		}
		sendCommand(cmd);
		showresp(INFO, ASC, "set Att B", cmd, "");

	} else {
		if (att_level == 0) {
			att_level = 1;
			cmd = "RA01;";
			atten_label("Att 6", true);
		}
		else if (att_level == 1) {
			att_level = 2;
			cmd = "RA02;";
			atten_label("Att 12", true);
		} else if (att_level == 2) {
			att_level = 3;
			cmd = "RA03;";
			atten_label("Att 18", true);
		} else if (att_level == 3) {
			att_level = 0;
			cmd = "RA00;";
			atten_label("Att", false);
		}
		sendCommand(cmd);
		showresp(INFO, ASC, "set Att A", cmd, "");
	}
}

//==============================================================================
//  Modified to read and show the actual radio setting, in the button.
//==============================================================================
int RIG_TS990::get_attenuator() {

	if (useB) {
		cmd = "RA1;";
		if (wait_char(';', 5, 100, "get Att B", ASC) < 5) return att_on;

		size_t p = replystr.rfind("RA");
		if (p == string::npos) return att_on;

		if (replystr[p + 2] == '1' && replystr[p + 3] == '0') {
			att_on = 0;						// Attenuator is OFF
			att_level = 0;					// remember it...
			atten_label("Att", false);		// show it...
		} else if (replystr[p + 2] == '1' && replystr[p + 3] == '1') {
			att_on = 1;						// Attenuator is ON, 6dB
			att_level = 1;					// remember the level
			atten_label("Att 6", true);		// show it...
		} else if (replystr[p + 2] == '1' && replystr[p + 3] == '2') {
			att_on = 1;						// .. still ON, 12dB
			att_level = 2;					// remember this level
			atten_label("Att 12", true);	// show it.
		} else if (replystr[p + 2] == '1' && replystr[p + 3] == '3') {
			att_on = 1;						// .. still ON 18dB
			att_level = 3;					// remember...
			atten_label("Att 18", true);	// show this too..
		}
	} else {
		cmd = "RA0;";
		if (wait_char(';', 5, 100, "get Att B", ASC) < 5) return att_on;

		size_t p = replystr.rfind("RA");
		if (p == string::npos) return att_on;

		if (replystr[p + 2] == '0' && replystr[p + 3] == '0') {
			att_on = 0;
			att_level = 0;
			atten_label("Att", false);
		} else if (replystr[p + 2] == '0' && replystr[p + 3] == '1') {
			att_on = 1;
			att_level = 1;
			atten_label("Att 6", true);
		} else if (replystr[p + 2] == '0' && replystr[p + 3] == '2') {
			att_on = 1;
			att_level = 2;
			atten_label("Att 12", true);
		} else if (replystr[p + 2] == '0' && replystr[p + 3] == '3') {
			att_on = 1;
			att_level = 3;
			atten_label("Att 18", true);
		}
	}
	return att_on;
}


//==============================================================================
//Get PreAmp
//==============================================================================

void RIG_TS990::set_preamp(int val)
{

	if (useB) {
		preamp_level = val;
		if (val) cmd = "PA11;";
		else     cmd = "PA10;";
		sendCommand(cmd);
	} else {
		preamp_level = val;
		if (val) cmd = "PA01;";
		else     cmd = "PA00;";
		sendCommand(cmd);
	}
	showresp(INFO, ASC, "set preamp", cmd, "");
}

int RIG_TS990::get_preamp()
{
	if (useB) {
		cmd = "PA1;";
		if (wait_char(';', 5, 20, "get preamp", ASC) < 5) return 0;

		size_t p = replystr.rfind("PA");
		if (p == string::npos) return 0;

		if (replystr[p  + 3] == '1')
			preamp_level = 1;
		else
			preamp_level = 0;

	} else {
		cmd = "PA0;";
		if (wait_char(';', 5, 20, "get preamp", ASC) < 5) return 0;

		size_t p = replystr.rfind("PA");
		if (p == string::npos) return 0;

		if (replystr[p  + 3] == '1')
			preamp_level = 1;
		else
			preamp_level = 0;
	}

	return preamp_level;
}

//==============================================================================

void RIG_TS990::set_split(bool val)
{
	split = val;
	if (useB) {
		if (val) {
			cmd = "MV10;TB0;";
			sendCommand(cmd);
			showresp(INFO, ASC, "Rx on B, Tx on A", cmd, "");
		} else {
			cmd = "MV10;TB1;";
			sendCommand(cmd);
			showresp(INFO, ASC, "Rx on B, Tx on B", cmd, "");
		}
	} else {
		if (val) {
			cmd = "MV00;TB1;";
			sendCommand(cmd);
			showresp(INFO, ASC, "Rx on A, Tx on B", cmd, "");
		} else {
			cmd = "MV00;TB0;";
			sendCommand(cmd);
			showresp(INFO, ASC, "Rx on A, Tx on A", cmd, "");
		}
	}
	Fl::awake(highlight_vfo, (void *)0);
}


int RIG_TS990::get_split()
{
	size_t p;
	int split = 0;
	char rx = 0, tx = 0;
// tx vfo
	cmd = rsp = "TB";
	cmd.append(";");
	if (wait_char(';', 4, 20, "get split tx vfo", ASC) == 4) {
		p = replystr.rfind(rsp);
		if (p == string::npos) return split;
		tx = replystr[p+2];
	}
// rx vfo
	cmd = rsp = "CB";
	cmd.append(";");
	if (wait_char(';', 4, 20, "get split rx vfo", ASC) == 4) {
		p = replystr.rfind(rsp);
		if (p == string::npos) return split;
		rx = replystr[p+2];
 //split test
		split = (tx == '1' ? 2 : 0) + (rx == '1' ? 1 : 0);
	}
	return split;
}

//==============================================================================
const char * RIG_TS990::get_bwname_(int n, int md)
{
	static char bwname[20];
	if (n > 256) {
		int SH = (n >> 8) & 0x7F;
		int SL = n & 0x7F;
		snprintf(bwname, sizeof(bwname), "%s/%s",
			(md == LSB || md == USB || md == FM) ? TS990_filt_SL[SL] :
			(md == AM || md == AMD1 || md == AMD2 || md == AMD3) ? TS990_AM_SL[SL] :
			(md == CW ||md == CWR) ? TS990_CAT_CW_width [SL]:
			(md == FSK ||md == FSKR) ? TS990_FSK_filt [SL]:
			(md == PSK ||md == PSKR) ? TS990_PSK_filt [SL]:
			TS990_filt_shift[SL],
			(md == LSB || md == USB || md == FM) ? TS990_filt_SH[SH] :
			(md == AM || md == AMD1 || md == AMD2 || md == AMD3) ? TS990_AM_SH[SH] :
			(md == CW ||md == CWR) ? TS990_CAT_CW_shift [SH]:
			(md == FSK ||md == FSKR) ? TS990_FSK_filt [SH]:
			(md == PSK ||md == PSKR) ? TS990_PSK_filt [SH]:
			TS990_filt_width[SH] );
	}
	return bwname;
}

//==============================================================================
// Get/Set VFO for A and B
//==============================================================================

long RIG_TS990::get_vfoA ()
{
	cmd = "FA;";
	if (wait_char(';', 14, 20, "get vfoA", ASC) < 14) return A.freq;

	size_t p = replystr.rfind("FA");
	if (p == string::npos) return A.freq;

	long f = 0L;
	long mul = 1L;
	for (size_t n = 12; n > 1; n--) {
		f += (replystr[p + n] - '0') * mul;
		mul *= 10;
	}
	A.freq = f;
	return A.freq;
}

void RIG_TS990::set_vfoA (long freq)
{
	A.freq = freq;
	cmd = "FA00000000000;";
	for (int i = 12; i > 1; i--) {
		cmd[i] += freq % 10;
		freq /= 10;
	}
	sendCommand(cmd);
	showresp(INFO, ASC, "set vfo A", cmd, "");
}

long RIG_TS990::get_vfoB ()
{
	cmd = "FB;";
	if (wait_char(';', 14, 20, "get vfoB", ASC) < 14) return B.freq;

	size_t p = replystr.rfind("FB");
	if (p == string::npos) return B.freq;

	long f = 0L;
	long mul = 1L;
	for (size_t n = 12; n > 1; n--) {
		f += (replystr[p + n] - '0') * mul;
		mul *= 10;
	}
	B.freq = f;

	return B.freq;
}

void RIG_TS990::set_vfoB (long freq)
{
	B.freq = freq;
	cmd = "FB00000000000;";
	for (int i = 12; i > 1; i--) {
		cmd[i] += freq % 10;
		freq /= 10;
	}
	sendCommand(cmd);
	showresp(INFO, ASC, "set vfo B", cmd, "");
}

//==============================================================================
// Smeter reading
//==============================================================================

int RIG_TS990::get_smeter()
{
	if (useB) cmd = "SM1;";
	else      cmd = "SM0;";

	int mtr = 0;
	if (wait_char(';', 8, 20, "get", ASC) < 8)
		return 0;
	size_t p = replystr.find("SM");
	if (p == string::npos)
		return 0;
	mtr = atoi(&replystr[p + 3]);
	mtr *= 10;
	mtr /= 7;

	return mtr;
}

//==============================================================================
// Power out reading
//==============================================================================

int RIG_TS990::get_power_out()
{
	static int meter[] = 
	{  0,  7, 15, 23, 29,  34,  40,  46,  51,  58 };
	static float val[] =
	{  0, 10, 25, 50, 75, 100, 125, 150, 175, 200 };
	if (useB) cmd = "SM1;";
	else      cmd = "SM0;";

	int mtr = 0;
	if (wait_char(';', 8, 20, "get", ASC) < 8)
		return 0;
	size_t p = replystr.find("SM");
	if (p == string::npos)
		return 0;
	mtr = atoi(&replystr[p + 3]);

	int i = 0;
	while (i < 9 && (mtr > meter[i])) i++;
	float value = val[i] + (val[i+1] - val[i]) * (mtr - meter[i]) / (meter[i+1] - meter[i]);

	return (int)value;
}

//==============================================================================
// ALC, SWR readings
//==============================================================================

static bool read_alc = false;
static int alc_val = 0;

int RIG_TS990::get_swr(void)
{

	int mtr = 0;

	read_alc = false;

	cmd = "RM21;";
	sendCommand(cmd);

	cmd = "RM;";
	if (wait_char(';', 8, 20, "get swr", ASC) < 16) return 0;

	size_t p = replystr.find("RM2");
	if (p == string::npos) return 0;

	mtr = atoi(&replystr[p + 3]);
	mtr *= 10;
	mtr /= 7;
	if (mtr > 100) mtr = 100;

	return mtr;
}

int RIG_TS990::get_alc(void)
{
	if (read_alc) {
		read_alc = false;
		return alc_val;
	}
	cmd = "RM11;";
	sendCommand(cmd);

	cmd = "RM;";
	if (wait_char(';', 8, 20, "get alc", ASC) < 8) return 0;

	size_t p = replystr.find("RM1");
	if (p == string::npos) return 0;

	alc_val = atoi(&replystr[p + 3]);
	alc_val *= 10;
	alc_val /= 7;
	if (alc_val > 100) alc_val = 100;
	return alc_val;
}

//==============================================================================
// Transceiver power level
//==============================================================================

void RIG_TS990::set_power_control(double val)
{
	int ival = (int)val;
	cmd = "PC000;";
	for (int i = 4; i > 1; i--) {
		cmd[i] += ival % 10;
		ival /= 10;
	}
	sendCommand(cmd);
}

int RIG_TS990::get_power_control()
{
	cmd = "PC;";
	if (wait_char(';', 6, 20, "get pwr ctrl", ASC) < 6) return 0;

	size_t p = replystr.rfind("PC");
	if (p == string::npos) return 0;
	int mtr = 0;
	replystr[p + 5] = 0;
	mtr = atoi(&replystr[p + 2]);
	return mtr;
}

//==============================================================================
// Volume control return 0 ... 100
//==============================================================================

int RIG_TS990::get_volume_control()
{
	if (useB) {

	cmd = "AG1;";
	if (wait_char(';', 7, 20, "get vol ctrl", ASC) < 7) return 0;

	size_t p = replystr.rfind("AG");
	if (p == string::npos) return 0;

	replystr[p + 6] = 0;
	int val = atoi(&replystr[p + 3]);
	return (int)(val / 2.55);

	} else {

	cmd = "AG0;";
	if (wait_char(';', 7, 20, "get vol ctrl", ASC) < 7) return 0;

	size_t p = replystr.rfind("AG");
	if (p == string::npos) return 0;

	replystr[p + 6] = 0;
	int val = atoi(&replystr[p + 3]);
	return (int)(val / 2.55);
	}

}

void RIG_TS990::set_volume_control(int val)
{
	if (useB) {

	int ivol = (int)(val * 2.55);
	cmd = "AG1000;";
	for (int i = 5; i > 2; i--) {
		cmd[i] += ivol % 10;
		ivol /= 10;
	}
	sendCommand(cmd);

	} else {

	int ivol = (int)(val * 2.55);
	cmd = "AG0000;";
	for (int i = 5; i > 2; i--) {
		cmd[i] += ivol % 10;
		ivol /= 10;
	}
	sendCommand(cmd);
	}
}

//==============================================================================
// Tranceiver PTT on/off
//==============================================================================

void RIG_TS990::set_PTT_control(int val)
{
	if (val) {
		if (data_mode)
			cmd = "TX1;";
		else
			cmd = "TX0;";
	} else
		cmd = "RX;";
	sendCommand(cmd);
}

//==============================================================================
// Transceiver Tune
//==============================================================================

void RIG_TS990::tune_rig()
{
	cmd = "AC111;";
	sendCommand(cmd);
}

//==============================================================================
// Mode A / B
//==============================================================================

void RIG_TS990::set_modeA(int val)
{
	active_mode = A.imode = val;
	cmd = "OM0";
	cmd += TS990_mode_chr[val];
	cmd += ';';
	sendCommand(cmd);
	showresp(INFO, ASC, "set mode main band", cmd, "");
	set_widths(val);
	get_bwA();
}

int RIG_TS990::get_modeA()
{
	int md = A.imode;
	cmd = "OM0;";
	if (wait_char(';', 5, 20, "get mode main band", ASC) < 5) return A.imode;

	size_t p = replystr.rfind("OM");
	if (p == string::npos) return A.imode;

// this switch statement can be replaced with
// md = replystr[p+3] - 1;
// if (md < LSB) md = LSB;
// if (md > AMD3) md = AMD3;

	switch (replystr[p + 3]) {
		case '1' : md = LSB; break;
		case '2' : md = USB; break;
		case '3' : md = CW; break;
		case '4' : md = FM; break;
		case '5' : md = AM; break;
		case '6' : md = FSK; break;
		case '7' : md = CWR; break;
		case '9' : md = FSKR; break;
		case 'A' : md = PSK; break;
		case 'B' : md = PSKR; break;
		case 'C' : md = LSBD1; break;
		case 'D' : md = USBD1; break;
		case 'E' : md = FMD1; break;
		case 'F' : md = AMD1; break;
		case 'G' : md = LSBD2; break;
		case 'H' : md = USBD2; break;
		case 'I' : md = FMD2; break;
		case 'J' : md = AMD2; break;
		case 'K' : md = LSBD3; break;
		case 'L' : md = USBD3; break;
		case 'M' : md = FMD3; break;
		case 'N' : md = AMD3; break;
		default : md = A.imode;
	}
	if (md != A.imode) {
		active_mode = A.imode = md;
		set_widths(md);
	}
	get_agc();
	return A.imode;
}

void RIG_TS990::set_modeB(int val)
{
	active_mode = B.imode = val;
	cmd = "OM1";
	cmd += TS990_mode_chr[val];
	cmd += ';';
	sendCommand(cmd);
	showresp(INFO, ASC, "set mode sub band", cmd, "");
	set_widths(val);
	get_bwB();
}

int RIG_TS990::get_modeB()
{

	int md = B.imode;
	cmd = "OM1;";
	if (wait_char(';', 5, 20, "get mode sub band", ASC) < 5) return B.imode;

	size_t p = replystr.rfind("OM");
	if (p == string::npos) return B.imode;

// this switch statement can be replaced with
// md = replystr[p+3] - 1;
// if (md < LSB) md = LSB;
// if (md > AMD3) md = AMD3;

	switch (replystr[p + 3]) {
		case '1' : md = LSB; break;
		case '2' : md = USB; break;
		case '3' : md = CW; break;
		case '4' : md = FM; break;
		case '5' : md = AM; break;
		case '6' : md = FSK; break;
		case '7' : md = CWR; break;
		case '9' : md = FSKR; break;
		case 'A' : md = PSK; break;
		case 'B' : md = PSKR; break;
		case 'C' : md = LSBD1; break;
		case 'D' : md = USBD1; break;
		case 'E' : md = FMD1; break;
		case 'F' : md = AMD1; break;
		case 'G' : md = LSBD2; break;
		case 'H' : md = USBD2; break;
		case 'I' : md = FMD2; break;
		case 'J' : md = AMD2; break;
		case 'K' : md = LSBD3; break;
		case 'L' : md = USBD3; break;
		case 'M' : md = FMD3; break;
		case 'N' : md = AMD3; break;
		default : md = B.imode;
	}
	if (md != B.imode) {
		active_mode = B.imode = md;
		set_widths(md);
	}
	get_agc();
	return B.imode;
}

//==============================================================================
// Microphone gain
//==============================================================================

void RIG_TS990::set_mic_gain(int val)
{
	cmd = "MG000;";
	for (int i = 3; i > 0; i--) {
		cmd[1+i] += val % 10;
		val /= 10;
	}
	sendCommand(cmd);
}

int RIG_TS990::get_mic_gain()
{
	int val = 0;
	cmd = "MG;";
	if (wait_char(';', 6, 20, "get mic ctrl", ASC)  >= 6) {
		size_t p = replystr.rfind("MG");
		if (p == string::npos) return val;
		replystr[p + 5] = 0;
		val = atoi(&replystr[p + 2]);
	}
	return val;
}

void RIG_TS990::get_mic_min_max_step(int &min, int &max, int &step)
{
	min = 0;
	max = 100;
	step = 1;
}

//==============================================================================
//  Read/Write SSB and Data Mode Filter Settings
//==============================================================================

void RIG_TS990::read_menu_0607()
{
	save_menu_0607 = false;
	cmd = "EX00607;"; sendCommand(cmd);
	if (wait_char(';', 12, 20, "Read menu 0607", ASC) >= 12) {
		size_t p = replystr.rfind("EX00607");
		if (p != string::npos)
			save_menu_0607 = (replystr[p+10] == '1');
	}
}

void RIG_TS990::read_menu_0608()
{
	save_menu_0608 = false;
	cmd = "EX00608;"; sendCommand(cmd);
	if (wait_char(';', 12, 20, "Read menu 0608", ASC) >= 12) {
		size_t p = replystr.rfind("EX00608");
		if (p != string::npos)
			save_menu_0608 = (replystr[p+10] == '1');
	}
}

void RIG_TS990::set_menu_0607(int val)
{
	menu_0607 = val;
	cmd = "EX00607 00";
	cmd += menu_0607 ? "1" : "0";
	cmd += ";";
	sendCommand(cmd);
}

void RIG_TS990::set_menu_0608(int val)
{
	menu_0608 = val;
	cmd = "EX00608 00";
	cmd += menu_0608 ? "1" : "0";
	cmd += ";";
	sendCommand(cmd);
}

//==============================================================================
// Bandpass filter commands
//==============================================================================

int RIG_TS990::set_widths(int val)
{
	int bw = 0;
	if (useB) bw = B.iBW;
	else bw = A.iBW;
	switch (val) {
	case LSB: case USB:
		if (menu_0607) {
			bandwidths_ = TS990_filt_width;
			bw_vals_    = TS990_WIDTH_bw_vals;
			dsp_SL      = TS990_filt_width;
			SL_tooltip  = TS990_filt_width_tooltip;
			SL_label    = TS990_filt_width_label;
			dsp_SH      = TS990_filt_shift;
			SH_tooltip  = TS990_filt_shift_tooltip;
			SH_label    = TS990_filt_shift_label;
		} else {
			bandwidths_ = TS990_filt_SH;
			bw_vals_    = TS990_HI_bw_vals;
			dsp_SL      = TS990_filt_SL;
			SL_tooltip  = TS990_filt_SL_tooltip;
			SL_label    = TS990_filt_SL_label;
			dsp_SH      = TS990_filt_SH;
			SH_tooltip  = TS990_filt_SH_tooltip;
			SH_label    = TS990_filt_SH_label;
		}
		break;
	case LSBD1: case LSBD2: case LSBD3:
	case USBD1: case USBD2: case USBD3:
		if (menu_0608) {
			bandwidths_ = TS990_filt_width;
			bw_vals_ = TS990_WIDTH_bw_vals;
			dsp_SL     = TS990_filt_width;
			SL_tooltip = TS990_filt_width_tooltip;
			SL_label   = TS990_filt_width_label;
			dsp_SH     = TS990_filt_shift;
			SH_tooltip = TS990_filt_shift_tooltip;
			SH_label   = TS990_filt_shift_label;
		} else {
			bandwidths_ = TS990_filt_SH;
			bw_vals_ = TS990_HI_bw_vals;
			dsp_SL = TS990_filt_SL;
			SL_tooltip = TS990_filt_SL_tooltip;
			SL_label   = TS990_filt_SL_label;
			dsp_SH = TS990_filt_SH;
			SH_tooltip = TS990_filt_SH_tooltip;
			SH_label   = TS990_filt_SH_label;
		}
		break;
	case CW: case CWR:
		bandwidths_ = TS990_CW_width;
		bw_vals_ = TS990_CW_bw_vals;
		dsp_SL = TS990_CW_width;
		SL_tooltip = TS990_CW_W_tooltip;
		SL_label   = TS990_CW_W_btn_label;
		dsp_SH = TS990_CW_shift;
		SH_tooltip = TS990_CW_S_tooltip;
		SH_label   = TS990_CW_S_btn_label;
		break;
	case FSK: case FSKR:
		bandwidths_ = TS990_FSK_filt;
		bw_vals_ = TS990_FSK_bw_vals;
		dsp_SL = TS990_empty;
		dsp_SH = TS990_empty;
		break;
	case PSK: case PSKR:
		bandwidths_ = TS990_PSK_filt;
		bw_vals_ = TS990_PSK_bw_vals;
		dsp_SL = TS990_empty;
		dsp_SH = TS990_empty;
		break;
	case AM: case AMD1: case AMD2: case AMD3:
		bandwidths_ = TS990_AM_SH;
		bw_vals_ = TS990_AM_HI_bw_vals;
		dsp_SL = TS990_AM_SL;
		dsp_SH = TS990_AM_SH;
		SL_tooltip = TS990_AM_SL_tooltip;
		SL_label   = TS990_AM_btn_SL_label;
		SH_tooltip = TS990_AM_SH_tooltip;
		SH_label   = TS990_AM_btn_SH_label;
		break;
	case FM: case FMD1: case FMD2: case FMD3:
		bandwidths_ = TS990_filt_SH;
		bw_vals_ = TS990_HI_bw_vals;
		dsp_SL = TS990_filt_SL;
		dsp_SH = TS990_filt_SH;
		SL_tooltip = TS990_filt_SL_tooltip;
		SL_label   = TS990_filt_SL_label;
		SH_tooltip = TS990_filt_SH_tooltip;
		SH_label   = TS990_filt_SH_label;
		break;
	}
	return bw;
}


const char **RIG_TS990::bwtable(int m)
{
	const char **filter = TS990_filt_SH;
	switch (m) {
		case LSB: case USB:
			if (menu_0607)
				filter = TS990_filt_width;
			else
				filter = TS990_filt_SH;
			break;
		 case FM: case FMD1: case FMD2: case FMD3:
			filter = TS990_filt_SH;
			break;
		case LSBD1: case LSBD2: case LSBD3:
		case USBD1: case USBD2: case USBD3:
			if (menu_0608)
				filter = TS990_filt_width;
			else
				filter = TS990_filt_SH;
			break;
		case AM: case AMD1: case AMD2: case AMD3:
			filter = TS990_AM_SH;
			break;
		case CW: case CWR:
			filter = TS990_CW_width;
			break;
		case FSK: case FSKR:
			filter = TS990_FSK_filt;
			break;
		case PSK: case PSKR:
			filter = TS990_PSK_filt;
			break;
	}
	return filter;
}

const char **RIG_TS990::lotable(int m)
{
	const char **filter = TS990_filt_SL;
	switch (m) {
		case LSB: case USB:
			if (menu_0607)
				filter = TS990_filt_width;
			else
				filter = TS990_filt_SL;
			break;
		case FM: case FMD1: case FMD2: case FMD3:
			filter = TS990_filt_SL;
			break;
		case LSBD1: case LSBD2: case LSBD3:
		case USBD1: case USBD2: case USBD3:
			if (menu_0608)
				filter = TS990_filt_width;
			else
				filter = TS990_filt_SL;
			break;
		case CW: case CWR:
			filter = TS990_CW_width;
			break;
		case FSK: case FSKR:
			filter = NULL;//TS990_FSK_filt;
			break;
		case PSK: case PSKR:
			filter = NULL;//TS990_PSK_filt;
			break;
		case AM: case AMD1: case AMD2: case AMD3:
			filter = TS990_AM_SL;
			break;
	}
	return filter;
}

const char **RIG_TS990::hitable(int m)
{
	const char **filter = TS990_filt_SH;
	switch (m) {
		case LSB: case USB:
			if (menu_0607)
				filter = TS990_filt_shift;
			else
				filter = TS990_filt_SH;
			break;
		case LSBD1: case LSBD2: case LSBD3:
		case USBD1: case USBD2: case USBD3:
			if (menu_0608)
				filter = TS990_filt_shift;
			else
				filter = TS990_filt_SH;
			break;
		case CW: case CWR:
			filter = TS990_CW_shift;
			break;
		case FM: case FMD1: case FMD2: case FMD3:
			filter = TS990_filt_SH;
			break;
		case FSK: case FSKR:
			filter = NULL;
			break;
		case PSK: case PSKR:
			filter = NULL;
			break;
		case AM: case AMD1: case AMD2: case AMD3:
			filter = TS990_AM_SH;
			break;
	}
	return filter;
}

int RIG_TS990::adjust_bandwidth(int val)
{
	int retval = 0;
	switch (val) {
		case LSB: case USB:
			if (useB)
				retval = menu_0607 ? B_default_SH_WI : B_default_HI_LO;
			else
				retval = menu_0607 ? A_default_SH_WI : A_default_HI_LO;
			break;
		case LSBD1: case USBD1:
			if (useB)
				retval = menu_0608 ? B_default_SH_WI_D1 : B_default_HI_LO;
			else
				retval = menu_0608 ? A_default_SH_WI_D1 : A_default_HI_LO;
			break;
		case LSBD2: case USBD2:
			if (useB)
				retval = menu_0608 ? B_default_SH_WI_D2 : B_default_HI_LO;
			else
				retval = menu_0608 ? A_default_SH_WI_D2 : A_default_HI_LO;
			break;
		case LSBD3: case USBD3:
			if (useB)
				retval = menu_0608 ? B_default_SH_WI_D3 : B_default_HI_LO;
			else
				retval = menu_0608 ? A_default_SH_WI_D3 : A_default_HI_LO;
			break;
		case FM:
			retval = (useB ? B_default_FM : A_default_FM);
			break;
		case FMD1:
			retval = (useB ? B_default_FM_D1 : A_default_FM_D1);
			break;
		case FMD2:
			retval = (useB ? B_default_FM_D2 : A_default_FM_D2);;
			break;
		case FMD3:
			retval = (useB ? B_default_FM_D3 : A_default_FM_D3);
			break;
		case AM:
			retval = (useB ? B_default_AM : A_default_AM);
			break;
		case AMD1:
			retval = (useB ? B_default_AM_D1 : A_default_AM_D1);
			break;
		case AMD2:
			retval = (useB ? B_default_AM_D2 : A_default_AM_D2);
			break;
		case AMD3:
			retval = (useB ? B_default_AM_D3 : A_default_AM_D3);
			break;
		case CW: case CWR:
			retval = (useB ? B_default_CW : A_default_CW);
			break;
		case FSK: case FSKR:
			retval = (useB ? B_default_FSK : A_default_FSK);
			break;
		case PSK: case PSKR:
			retval = (useB ? B_default_PSK : A_default_PSK);
			break;
		}
	return retval;
}

int RIG_TS990::def_bandwidth(int val)
{
	return adjust_bandwidth(val);
}

void RIG_TS990::set_bwA(int val)
{
	size_t SL = 0, SH = 0;
	SL = val & 0x7F;
	SH = (val >> 8) & 0x7F;

	switch (A.imode) {
	case LSB: case USB:
		if (val < 256) break;
		if (menu_0607) {
			if (SL >= sizeof(TS990_CAT_filt_width)/sizeof(*TS990_CAT_filt_width) ||
				 SH >= sizeof(TS990_CAT_filt_shift)/sizeof(*TS990_CAT_filt_shift) )
				A.iBW = A_default_SH_WI;
			else
				A.iBW = val;
			cmd = "SL0";
			cmd += TS990_CAT_filt_width[A.iBW & 0x7F];
			sendCommand(cmd);
			showresp(INFO, ASC, "set width", cmd, "");
			cmd = "SH0";
			cmd += TS990_CAT_filt_shift[(A.iBW >> 8) & 0x7F];
			sendCommand(cmd);
			showresp(INFO, ASC, "set filter shift", cmd, "");
		} else {
			if (SH >= sizeof(TS990_CAT_filt_SH)/sizeof(*TS990_CAT_filt_SH) ||
				 SL >= sizeof(TS990_CAT_filt_SL)/sizeof(*TS990_CAT_filt_SL) )
				A.iBW = A_default_HI_LO;
			else
				A.iBW = val;
			cmd = "SL0";
			cmd += TS990_CAT_filt_SL[A.iBW & 0x7F];
			sendCommand(cmd);
			showresp(INFO, ASC, "set filter lower cutoff", cmd, "");
			cmd = "SH0";
			cmd += TS990_CAT_filt_SH[(A.iBW >> 8) & 0x7F];
			sendCommand(cmd);
			showresp(INFO, ASC, "set filter upper cutoff", cmd, "");
		}
		break;

	case LSBD1: case LSBD2: case LSBD3:
	case USBD1: case USBD2: case USBD3:
		if (val < 256) break;
		if (menu_0608) {
			if (SL >= sizeof(TS990_CAT_filt_width)/sizeof(*TS990_CAT_filt_width) ||
				 SH >= sizeof(TS990_CAT_filt_shift)/sizeof(*TS990_CAT_filt_shift) )
				A.iBW = A_default_SH_WI;
			else
				A.iBW = val;
			cmd = "SL0";
			cmd += TS990_CAT_filt_width[A.iBW & 0x7F];
			sendCommand(cmd);
			showresp(INFO, ASC, "set data width", cmd, "");
			cmd = "SH0";
			cmd += TS990_CAT_filt_shift[(A.iBW >> 8) & 0x7F];
			sendCommand(cmd);
			showresp(INFO, ASC, "set data shift", cmd, "");
		} else {
			if (SH >= sizeof(TS990_CAT_filt_SH)/sizeof(*TS990_CAT_filt_SH) ||
				 SL >= sizeof(TS990_CAT_filt_SL)/sizeof(*TS990_CAT_filt_SL) )
				A.iBW = A_default_HI_LO;
			else
				A.iBW = val;
			cmd = "SL0";
			cmd += TS990_CAT_filt_SL[A.iBW & 0x7F];
			sendCommand(cmd);
			showresp(INFO, ASC, "set data lower cutoff", cmd, "");
			cmd = "SH0";
			cmd += TS990_CAT_filt_SH[(A.iBW >> 8) & 0x7F];
			sendCommand(cmd);
			showresp(INFO, ASC, "set data upper cutoff", cmd, "");
		}
		break;

	case AM: case AMD1: case AMD2: case AMD3:
		if (val < 256) break;
		if (SH >= sizeof(TS990_CAT_AM_SH) / sizeof(*TS990_CAT_AM_SH) ||
			SL >= sizeof(TS990_CAT_AM_SL) / sizeof(*TS990_CAT_AM_SL) )
			A.iBW = A_default_AM;
		else
			A.iBW = val;
		cmd = "SL0";
		cmd += TS990_CAT_AM_SL[A.iBW & 0x7F];
		sendCommand(cmd);
		showresp(INFO, ASC, "set AM lower", cmd, "");
		cmd = "SH0";
		cmd += TS990_CAT_AM_SH[(A.iBW >> 8) & 0x7F];
		sendCommand(cmd);
		showresp(INFO, ASC, "set AM upper", cmd, "");
		break;

	case FM: case FMD1: case FMD2: case FMD3:
		if (val < 256) break;
		if (SH >= sizeof(TS990_CAT_filt_SH) / sizeof(*TS990_CAT_filt_SH) ||
			SL >= sizeof(TS990_CAT_filt_SL) / sizeof(*TS990_CAT_filt_SL) )
			A.iBW = A_default_FM;
		else
			A.iBW = val;
		cmd = "SL0";
		cmd += TS990_CAT_filt_SL[A.iBW & 0x7F];
		sendCommand(cmd);
		showresp(INFO, ASC, "set FM lower", cmd, "");
		cmd = "SH0";
		cmd += TS990_CAT_filt_SH[(A.iBW >> 8) & 0x7F];
		sendCommand(cmd);
		showresp(INFO, ASC, "set FM upper", cmd, "");
		break;

	case CW: case CWR:
		if (val < 256) break;
		if (SL >= sizeof(TS990_CAT_CW_width) / sizeof(*TS990_CAT_CW_width) ||
			SH >= sizeof(TS990_CAT_CW_shift) / sizeof(*TS990_CAT_CW_shift) )
			A.iBW = A_default_CW;
		else
			A.iBW = val;
		cmd = "SL0";
		cmd += TS990_CAT_CW_width[A.iBW & 0x7F];
		sendCommand(cmd);
		showresp(INFO, ASC, "set cw width", cmd, "");
		cmd = "SH0";
		cmd += TS990_CAT_CW_shift[(A.iBW >> 8) & 0x7F];
		sendCommand(cmd);
		showresp(INFO, ASC, "set cw shift", cmd, "");
		break;

	case FSK: case FSKR:
		if (val > 256) break;
		if (SL >= sizeof(TS990_CAT_FSK_filt) / sizeof(*TS990_CAT_FSK_filt))
			A.iBW = A_default_FSK;
		else
			A.iBW = val;
		cmd = "SL0";
		cmd += TS990_CAT_FSK_filt[A.iBW & 0x7F];
		sendCommand(cmd);
		showresp(INFO, ASC, "set FSK bw", cmd, "");
		break;

	case PSK: case PSKR:
		if (val > 256) break;
		if (SL >= sizeof(TS990_CAT_PSK_filt) / sizeof(*TS990_CAT_PSK_filt))
			A.iBW = A_default_PSK;
		else
			A.iBW = val;
		cmd = "SL0";
		cmd += TS990_CAT_PSK_filt[A.iBW & 0x7F];
		sendCommand(cmd);
		showresp(INFO, ASC, "set PSK bw", cmd, "");
		break;
	}
}

void RIG_TS990::set_bwB(int val)
{
	size_t SL = 0, SH = 0;
	SL = val & 0x7F;
	SH = (val >> 8) & 0x7F;

	switch (B.imode) {
	case LSB: case USB:
		if (val < 256) break;
		if (menu_0607) {
			if (SL >= sizeof(TS990_CAT_filt_width)/sizeof(*TS990_CAT_filt_width) ||
				 SH >= sizeof(TS990_CAT_filt_shift)/sizeof(*TS990_CAT_filt_shift) )
				B.iBW = B_default_SH_WI;
			else
				B.iBW = val;
			cmd = "SL0";
			cmd += TS990_CAT_filt_width[A.iBW & 0x7F];
			sendCommand(cmd);
			showresp(INFO, ASC, "set width", cmd, "");
			cmd = "SH0";
			cmd += TS990_CAT_filt_shift[(A.iBW >> 8) & 0x7F];
			sendCommand(cmd);
			showresp(INFO, ASC, "set filter shift", cmd, "");
		} else {
			if (SH >= sizeof(TS990_CAT_filt_SH)/sizeof(*TS990_CAT_filt_SH) ||
				SL >= sizeof(TS990_CAT_filt_SL)/sizeof(*TS990_CAT_filt_SL) )
				B.iBW = B_default_HI_LO;
			else
				B.iBW = val;
			cmd = "SL0";
			cmd += TS990_CAT_filt_SL[A.iBW & 0x7F];
			sendCommand(cmd);
			showresp(INFO, ASC, "set filter lower cutoff", cmd, "");
			cmd = "SH0";
			cmd += TS990_CAT_filt_SH[(A.iBW >> 8) & 0x7F];
			sendCommand(cmd);
			showresp(INFO, ASC, "set filter upper cutoff", cmd, "");
		}
		break;

	case LSBD1: case LSBD2: case LSBD3:
	case USBD1: case USBD2: case USBD3:
		if (val < 256) break;
		if (menu_0608) {
			if (SL >= sizeof(TS990_CAT_filt_width)/sizeof(*TS990_CAT_filt_width) ||
				SH >= sizeof(TS990_CAT_filt_shift)/sizeof(*TS990_CAT_filt_shift) )
				B.iBW = B_default_SH_WI;
			else
				B.iBW = val;
			cmd = "SL0";
			cmd += TS990_CAT_filt_width[A.iBW & 0x7F];
			sendCommand(cmd);
			showresp(INFO, ASC, "set data width", cmd, "");
			cmd = "SH0";
			cmd += TS990_CAT_filt_shift[(A.iBW >> 8) & 0x7F];
			sendCommand(cmd);
			showresp(INFO, ASC, "set data shift", cmd, "");
		} else {
			if (SH >= sizeof(TS990_CAT_filt_SH)/sizeof(*TS990_CAT_filt_SH) ||
				 SL >= sizeof(TS990_CAT_filt_SL)/sizeof(*TS990_CAT_filt_SL) )
				B.iBW = B_default_HI_LO;
			else
				B.iBW = val;
			cmd = "SL0";
			cmd += TS990_CAT_filt_SL[A.iBW & 0x7F];
			sendCommand(cmd);
			showresp(INFO, ASC, "set data lower cutoff", cmd, "");
			cmd = "SH0";
			cmd += TS990_CAT_filt_SH[(A.iBW >> 8) & 0x7F];
			sendCommand(cmd);
			showresp(INFO, ASC, "set data upper cutoff", cmd, "");
		}
		break;

	case AM: case AMD1: case AMD2: case AMD3:
		if (val < 256) break;
		if (SH >= sizeof(TS990_CAT_AM_SH) / sizeof(*TS990_CAT_AM_SH) ||
			SL >= sizeof(TS990_CAT_AM_SL) / sizeof(*TS990_CAT_AM_SL) )
			B.iBW = B_default_AM;
		else
			B.iBW = val;
		cmd = "SL0";
		cmd += TS990_CAT_AM_SL[A.iBW & 0x7F];
		sendCommand(cmd);
		showresp(INFO, ASC, "set AM lower", cmd, "");
		cmd = "SH0";
		cmd += TS990_CAT_AM_SH[(A.iBW >> 8) & 0x7F];
		sendCommand(cmd);
		showresp(INFO, ASC, "set AM upper", cmd, "");
		break;

	case FM: case FMD1: case FMD2: case FMD3:
		if (val < 256) break;
		if (SH >= sizeof(TS990_CAT_filt_SH) / sizeof(*TS990_CAT_filt_SH) ||
			SL >= sizeof(TS990_CAT_filt_SL) / sizeof(*TS990_CAT_filt_SL) )
			B.iBW = B_default_FM;
		else
			B.iBW = val;
		cmd = "SL0";
		cmd += TS990_CAT_filt_SL[A.iBW & 0x7F];
		sendCommand(cmd);
		showresp(INFO, ASC, "set FM lower", cmd, "");
		cmd = "SH0";
		cmd += TS990_CAT_filt_SH[(A.iBW >> 8) & 0x7F];
		sendCommand(cmd);
		showresp(INFO, ASC, "set FM upper", cmd, "");
		break;

	case CW: case CWR:
		if (val < 256) break;
		if (SL >= sizeof(TS990_CAT_CW_width) / sizeof(*TS990_CAT_CW_width) ||
			SH >= sizeof(TS990_CAT_CW_shift) / sizeof(*TS990_CAT_CW_shift) )
			B.iBW = B_default_CW;
		else
			B.iBW = val;
		cmd = "SL0";
		cmd += TS990_CAT_CW_width[A.iBW & 0x7F];
		sendCommand(cmd);
		showresp(INFO, ASC, "set cw width", cmd, "");
		cmd = "SH0";
		cmd += TS990_CAT_CW_shift[(A.iBW >> 8) & 0x7F];
		sendCommand(cmd);
		showresp(INFO, ASC, "set cw shift", cmd, "");
		break;

	case FSK: case FSKR:
		if (val > 256) break;
		if (SL >= sizeof(TS990_CAT_FSK_filt) / sizeof(*TS990_CAT_FSK_filt))
			B.iBW = B_default_FSK;
		else
			B.iBW = val;
		cmd = "SL0";
		cmd += TS990_CAT_FSK_filt[A.iBW & 0x7F];
		sendCommand(cmd);
		showresp(INFO, ASC, "set FSK bw", cmd, "");
		break;

	case PSK: case PSKR:
		if (val > 256) break;
		if (SL >= sizeof(TS990_CAT_PSK_filt) / sizeof(*TS990_CAT_PSK_filt))
			B.iBW = B_default_PSK;
		else
			B.iBW = val;
		cmd = "SL0";
		cmd += TS990_CAT_PSK_filt[A.iBW & 0x7F];
		sendCommand(cmd);
		showresp(INFO, ASC, "set PSK bw", cmd, "");
		break;
	}
}

int RIG_TS990::get_bwA()
{
	size_t p;
	int SL = 0, SH = 0;

	switch (A.imode) {
	case CW: case CWR:
		A.iBW = A_default_CW;
		SL = A.iBW & 0x7F;
		SH = (A.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get CW width", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get CW shift", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				A_default_CW = A.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
			}
		}
		break;

	case FSK: case FSKR:
		A.iBW = A_default_FSK;
		SL = A.iBW & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get FSK Width", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			A_default_FSK = A.iBW = (SL & 0x7F);
		}
		break;

	case PSK: case PSKR:
		A.iBW = A_default_PSK;
		SL = A.iBW & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get PSK Width", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			A_default_PSK = A.iBW = (SL & 0x7F);
		}
		break;

	case LSB: case USB:
		SL = A.iBW & 0x7F;
		SH = (A.iBW >> 8) & 0x7F;
		if (menu_0607) {
			A.iBW = A_default_SH_WI;
			cmd = "SL0;";
			if (wait_char(';', 6, 20, "get width", ASC) == 6) {
				p = replystr.rfind("SL");
				if (p == string::npos) break;
				SL = fm_decimal(&replystr[3], 2); //2 2
				cmd = "SH0;";
				if (wait_char(';', 6, 20, "get filter shift", ASC) == 6) {
					p = replystr.rfind("SH");
					if (p == string::npos) break;
					SH = fm_decimal(&replystr[3], 2); //2 2
					A_default_SH_WI = A.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
				}
			}
		} else {
			A.iBW = A_default_HI_LO;
			cmd = "SL0;";
			if (wait_char(';', 6, 20, "get filter lower cutoff", ASC) == 6) {
				p = replystr.rfind("SL");
				if (p == string::npos) break;
				SL = fm_decimal(&replystr[3], 2); //2 2
				cmd = "SH0;";
				if (wait_char(';', 6, 20, "get filter upper cutoff", ASC) == 6) {
					p = replystr.rfind("SH");
					if (p == string::npos) break;
					SH = fm_decimal(&replystr[3], 2); //2 2
					A_default_HI_LO = A.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
				}
			}
		}
		break;

	case LSBD1: case USBD1:
			SL = A.iBW & 0x7F;
			SH = (A.iBW >> 8) & 0x7F;
		if (menu_0608) {
			A.iBW = A_default_SH_WI_D1;
			cmd = "SL0;";
			if (wait_char(';', 6, 20, "get width", ASC) == 6) {
				p = replystr.rfind("SL");
				if (p == string::npos) break;
				SL = fm_decimal(&replystr[3], 2); //2 2
				cmd = "SH0;";
				if (wait_char(';', 6, 20, "get filter shift", ASC) == 6) {
					p = replystr.rfind("SH");
					if (p == string::npos) break;
						SH = fm_decimal(&replystr[3], 2); //2
					A_default_SH_WI_D1 = A.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
				}
			}
		} else {

			A.iBW = A_default_HI_LO;
			cmd = "SL0;";
			if (wait_char(';', 6, 20, "get filter lower cutoff", ASC) == 6) {
				p = replystr.rfind("SL");
				if (p == string::npos) break;
				SL = fm_decimal(&replystr[3], 2); //2 2
				cmd = "SH0;";
				if (wait_char(';', 6, 20, "get filter upper cutoff", ASC) == 6) {
					p = replystr.rfind("SH");
					if (p == string::npos) break;
					SH = fm_decimal(&replystr[3], 2); //2 2
					A_default_HI_LO = A.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
				}
			}
		}
		break;

	case LSBD2: case USBD2:
			SL = A.iBW & 0x7F;
			SH = (A.iBW >> 8) & 0x7F;
		if (menu_0608) {
			A.iBW = A_default_SH_WI_D2;
			cmd = "SL0;";
			if (wait_char(';', 6, 20, "get width", ASC) == 6) {
				p = replystr.rfind("SL");
				if (p == string::npos) break;
				SL = fm_decimal(&replystr[3], 2); //2 2
				cmd = "SH0;";
				if (wait_char(';', 6, 20, "get filter shift", ASC) == 6) {
					p = replystr.rfind("SH");
					if (p == string::npos) break;
					SH = fm_decimal(&replystr[3], 2); //2
					A_default_SH_WI_D2 = A.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
				}
			}
		} else {

			A.iBW = A_default_HI_LO;
			cmd = "SL0;";
			if (wait_char(';', 6, 20, "get filter lower cutoff", ASC) == 6) {
				p = replystr.rfind("SL");
				if (p == string::npos) break;
				SL = fm_decimal(&replystr[3], 2); //2 2
				cmd = "SH0;";
				if (wait_char(';', 6, 20, "get filter upper cutoff", ASC) == 6) {
					p = replystr.rfind("SH");
					if (p == string::npos) break;
					SH = fm_decimal(&replystr[3], 2); //2 2
					A_default_HI_LO = A.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
				}
			}
		}
		break;

	case LSBD3: case USBD3:
			SL = A.iBW & 0x7F;
			SH = (A.iBW >> 8) & 0x7F;
		if (menu_0608) {
			A.iBW = A_default_SH_WI_D3;
			cmd = "SL0;";
			if (wait_char(';', 6, 20, "get width", ASC) == 6) {
				p = replystr.rfind("SL");
				if (p == string::npos) break;
				SL = fm_decimal(&replystr[3], 2); //2 2
				cmd = "SH0;";
				if (wait_char(';', 6, 20, "get filter shift", ASC) == 6) {
					p = replystr.rfind("SH");
					if (p == string::npos) break;
					SH = fm_decimal(&replystr[3], 2); //2
					A_default_SH_WI_D3 = A.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
				}
			}
		} else {
			A.iBW = A_default_HI_LO;
			cmd = "SL0;";
			if (wait_char(';', 6, 20, "get filter lower cutoff", ASC) == 6) {
				p = replystr.rfind("SL");
				if (p == string::npos) break;
				SL = fm_decimal(&replystr[3], 2); //2 2
				cmd = "SH0;";
				if (wait_char(';', 6, 20, "get filter upper cutoff", ASC) == 6) {
					p = replystr.rfind("SH");
					if (p == string::npos) break;
					SH = fm_decimal(&replystr[3], 2); //2 2
					A_default_HI_LO = A.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
				}
			}
		}
		break;

	case AM:
		A.iBW = A_default_AM;
		SL = A.iBW & 0x7F;
		SH = (A.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get AM lo", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get AM hi", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				A_default_AM = A.iBW = ((SH << 8) | (SL & 0x7F)) | 0x8000;
			}
		}
		break;

	case AMD1:
		A.iBW = A_default_AM_D1;
		SL = A.iBW & 0x7F;
		SH = (A.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get AM lo", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get AM hi", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				A_default_AM_D1 = A.iBW = ((SH << 8) | (SL & 0x7F)) | 0x8000;
			}
		}
		break;

	case AMD2:
		A.iBW = A_default_AM_D2;
		SL = A.iBW & 0x7F;
		SH = (A.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get AM lo", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get AM hi", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				A_default_AM_D2 = A.iBW = ((SH << 8) | (SL & 0x7F)) | 0x8000;
			}
		}
		break;

	case AMD3:
		A.iBW = A_default_AM_D3;
		SL = A.iBW & 0x7F;
		SH = (A.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get AM lo", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get AM hi", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				A_default_AM_D3 = A.iBW = ((SH << 8) | (SL & 0x7F)) | 0x8000;
			}
		}
		break;

	case FM:
		A.iBW = A_default_FM;
		SL = A.iBW & 0x7F;
		SH = (A.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get FM lo", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get FM hi", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				A_default_FM = A.iBW = ((SH << 8) | (SL & 0x7F)) | 0x8000;
			}
		}
		break;

	case FMD1:
		A.iBW = A_default_FM_D1;
		SL = A.iBW & 0x7F;
		SH = (A.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get FM lo", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get FM hi", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				A_default_FM_D1 = A.iBW = ((SH << 8) | (SL & 0x7F)) | 0x8000;
			}
		}
		break;

	case FMD2 :
		A.iBW = A_default_FM_D2;
		SL = A.iBW & 0x7F;
		SH = (A.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get FM lo", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get FM hi", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				A_default_FM_D2 = A.iBW = ((SH << 8) | (SL & 0x7F)) | 0x8000;
			}
		}
		break;

	case FMD3:
		A.iBW = A_default_FM_D3;
		SL = A.iBW & 0x7F;
		SH = (A.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get FM lo", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get FM hi", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				A_default_FM_D3 = A.iBW = ((SH << 8) | (SL & 0x7F)) | 0x8000;
			}
		}
		return A.iBW;
	}
	return A.iBW;
}

int RIG_TS990::get_bwB()
{
	size_t p;
	int SL = 0, SH = 0;

	switch (B.imode) {
	case CW: case CWR:
		B.iBW = B_default_CW;
		SL = B.iBW & 0x7F;
		SH = (B.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get CW width", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get CW shift", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				B_default_CW = B.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
			}
		}
		break;

	case FSK: case FSKR:
		B.iBW = B_default_FSK;
		SL = B.iBW & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get FSK Width", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			B_default_FSK = B.iBW = (SL & 0x7F);
		}
		break;

	case PSK: case PSKR:
		B.iBW = B_default_PSK;
		SL = B.iBW & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get PSK Width", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			B_default_PSK = B.iBW = (SL & 0x7F);
		}
		break;

	case LSB: case USB:
		SL = B.iBW & 0x7F;
		SH = (B.iBW >> 8) & 0x7F;
		if (menu_0607) {
			B.iBW = B_default_SH_WI;
			cmd = "SL0;";
			if (wait_char(';', 6, 20, "get width", ASC) == 6) {
				p = replystr.rfind("SL");
				if (p == string::npos) break;
				SL = fm_decimal(&replystr[3], 2); //2 2
				cmd = "SH0;";
				if (wait_char(';', 6, 20, "get filter shift", ASC) == 6) {
					p = replystr.rfind("SH");
					if (p == string::npos) break;
					SH = fm_decimal(&replystr[3], 2); //2 2
					B_default_SH_WI = B.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
				}
			}
		} else {
			B.iBW = B_default_HI_LO;
			cmd = "SL0;";
			if (wait_char(';', 6, 20, "get filter lower cutoff", ASC) == 6) {
				p = replystr.rfind("SL");
				if (p == string::npos) break;
				SL = fm_decimal(&replystr[3], 2); //2 2
				cmd = "SH0;";
				if (wait_char(';', 6, 20, "get filter upper cutoff", ASC) == 6) {
					p = replystr.rfind("SH");
					if (p == string::npos) break;
					SH = fm_decimal(&replystr[3], 2); //2 2
					B_default_HI_LO = B.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
				}
			}
		}
		break;

	case LSBD1: case USBD1:
			SL = B.iBW & 0x7F;
			SH = (B.iBW >> 8) & 0x7F;
		if (menu_0608) {
			B.iBW = B_default_SH_WI_D1;
			cmd = "SL0;";
			if (wait_char(';', 6, 20, "get width", ASC) == 6) {
				p = replystr.rfind("SL");
				if (p == string::npos) break;
				SL = fm_decimal(&replystr[3], 2); //2 2
				cmd = "SH0;";
				if (wait_char(';', 6, 20, "get filter shift", ASC) == 6) {
					p = replystr.rfind("SH");
					if (p == string::npos) break;
						SH = fm_decimal(&replystr[3], 2); //2
					B_default_SH_WI_D1 = B.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
				}
			}
		} else {

			B.iBW = B_default_HI_LO;
			cmd = "SL0;";
			if (wait_char(';', 6, 20, "get filter lower cutoff", ASC) == 6) {
				p = replystr.rfind("SL");
				if (p == string::npos) break;
				SL = fm_decimal(&replystr[3], 2); //2 2
				cmd = "SH0;";
				if (wait_char(';', 6, 20, "get filter upper cutoff", ASC) == 6) {
					p = replystr.rfind("SH");
					if (p == string::npos) break;
					SH = fm_decimal(&replystr[3], 2); //2 2
					B_default_HI_LO = B.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
				}
			}
		}
		break;

	case LSBD2: case USBD2:
			SL = B.iBW & 0x7F;
			SH = (B.iBW >> 8) & 0x7F;
		if (menu_0608) {
			B.iBW = B_default_SH_WI_D2;
			cmd = "SL0;";
			if (wait_char(';', 6, 20, "get width", ASC) == 6) {
				p = replystr.rfind("SL");
				if (p == string::npos) break;
				SL = fm_decimal(&replystr[3], 2); //2 2
				cmd = "SH0;";
				if (wait_char(';', 6, 20, "get filter shift", ASC) == 6) {
					p = replystr.rfind("SH");
					if (p == string::npos) break;
					SH = fm_decimal(&replystr[3], 2); //2
					B_default_SH_WI_D2 = B.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
				}
			}
		} else {

			B.iBW = B_default_HI_LO;
			cmd = "SL0;";
			if (wait_char(';', 6, 20, "get filter lower cutoff", ASC) == 6) {
				p = replystr.rfind("SL");
				if (p == string::npos) break;
				SL = fm_decimal(&replystr[3], 2); //2 2
				cmd = "SH0;";
				if (wait_char(';', 6, 20, "get filter upper cutoff", ASC) == 6) {
					p = replystr.rfind("SH");
					if (p == string::npos) break;
					SH = fm_decimal(&replystr[3], 2); //2 2
					B_default_HI_LO = B.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
				}
			}
		}
		break;

	case LSBD3: case USBD3:
			SL = B.iBW & 0x7F;
			SH = (B.iBW >> 8) & 0x7F;
		if (menu_0608) {
			B.iBW = B_default_SH_WI_D3;
			cmd = "SL0;";
			if (wait_char(';', 6, 20, "get width", ASC) == 6) {
				p = replystr.rfind("SL");
				if (p == string::npos) break;
				SL = fm_decimal(&replystr[3], 2); //2 2
				cmd = "SH0;";
				if (wait_char(';', 6, 20, "get filter shift", ASC) == 6) {
					p = replystr.rfind("SH");
					if (p == string::npos) break;
					SH = fm_decimal(&replystr[3], 2); //2
					B_default_SH_WI_D3 = B.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
				}
			}
		} else {
			B.iBW = B_default_HI_LO;
			cmd = "SL0;";
			if (wait_char(';', 6, 20, "get filter lower cutoff", ASC) == 6) {
				p = replystr.rfind("SL");
				if (p == string::npos) break;
				SL = fm_decimal(&replystr[3], 2); //2 2
				cmd = "SH0;";
				if (wait_char(';', 6, 20, "get filter upper cutoff", ASC) == 6) {
					p = replystr.rfind("SH");
					if (p == string::npos) break;
					SH = fm_decimal(&replystr[3], 2); //2 2
					B_default_HI_LO = B.iBW = ((SH << 8) | (SL & 0x7F )) | 0x8000;
				}
			}
		}
		break;

	case AM:
		B.iBW = B_default_AM;
		SL = B.iBW & 0x7F;
		SH = (B.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get AM lo", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get AM hi", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				B_default_AM = B.iBW = ((SH << 8) | (SL & 0x7F)) | 0x8000;
			}
		}
		break;

	case AMD1:
		B.iBW = B_default_AM_D1;
		SL = B.iBW & 0x7F;
		SH = (B.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get AM lo", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get AM hi", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				B_default_AM_D1 = B.iBW = ((SH << 8) | (SL & 0x7F)) | 0x8000;
			}
		}
		break;

	case AMD2:
		B.iBW = B_default_AM_D2;
		SL = B.iBW & 0x7F;
		SH = (B.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get AM lo", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get AM hi", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				B_default_AM_D2 = B.iBW = ((SH << 8) | (SL & 0x7F)) | 0x8000;
			}
		}
		break;

	case AMD3:
		B.iBW = B_default_AM_D3;
		SL = B.iBW & 0x7F;
		SH = (B.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get AM lo", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get AM hi", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				B_default_AM_D3 = B.iBW = ((SH << 8) | (SL & 0x7F)) | 0x8000;
			}
		}
		break;

	case FM:
		B.iBW = B_default_FM;
		SL = B.iBW & 0x7F;
		SH = (B.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get FM lo", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get FM hi", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				B_default_FM = B.iBW = ((SH << 8) | (SL & 0x7F)) | 0x8000;
			}
		}
		break;

	case FMD1:
		B.iBW = B_default_FM_D1;
		SL = B.iBW & 0x7F;
		SH = (B.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get FM lo", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get FM hi", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				B_default_FM_D1 = B.iBW = ((SH << 8) | (SL & 0x7F)) | 0x8000;
			}
		}
		break;

	case FMD2 :
		B.iBW = B_default_FM_D2;
		SL = B.iBW & 0x7F;
		SH = (B.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get FM lo", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get FM hi", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				B_default_FM_D2 = B.iBW = ((SH << 8) | (SL & 0x7F)) | 0x8000;
			}
		}
		break;

	case FMD3:
		B.iBW = B_default_FM_D3;
		SL = B.iBW & 0x7F;
		SH = (B.iBW >> 8) & 0x7F;
		cmd = "SL0;";
		if (wait_char(';', 6, 20, "get FM lo", ASC) == 6) {
			p = replystr.rfind("SL");
			if (p == string::npos) break;
			SL = fm_decimal(&replystr[3], 2); //2 2
			cmd = "SH0;";
			if (wait_char(';', 6, 20, "get FM hi", ASC) == 6) {
				p = replystr.rfind("SH");
				if (p == string::npos) break;
				SH = fm_decimal(&replystr[3], 2); //2 2
				B_default_FM_D3 = B.iBW = ((SH << 8) | (SL & 0x7F)) | 0x8000;
			}
		}
		return B.iBW;
	}
	return B.iBW;
}

int RIG_TS990::get_modetype(int n)
{
	return TS990_mode_type[n];
}

void RIG_TS990::set_noise(bool val) //Now Setting AGC
{
	if (useB) {
		if (nb_level == 2) {
			nb_level = 3;
			nb_label("AGC F", false);
			cmd = "GC13;";
			sendCommand(cmd);
		} else if (nb_level == 3) {
			nb_level = 1;
			nb_label("AGC S", false);
			cmd = "GC11;";
			sendCommand(cmd);
		} else if (nb_level == 1) {
			nb_level = 2;
			nb_label("AGC M", false);
			cmd = "GC12;";
			sendCommand(cmd);
		}
	} else {
		if (nb_level == 2) {
			nb_level = 3;
			nb_label("AGC F", false);
			cmd = "GC03;";
			sendCommand(cmd);
		} else if (nb_level == 3) {
			nb_level = 1;
			nb_label("AGC S", false);
			cmd = "GC01;";
			sendCommand(cmd);
		} else if (nb_level == 1) {
			nb_level = 2;
			nb_label("AGC M", false);
			cmd = "GC02;";
			sendCommand(cmd);
		}
	}
}

//----------------------------------------------------------------------
int  RIG_TS990::get_agc()
{
	int val = 0;
	if (useB) {
		cmd = "GC1;";
		if (wait_char(';', 5, 100, "get AGC", ASC) < 5) return val;

		size_t p = replystr.rfind("GC");
		if (p == string::npos) return val;
		if (replystr[p + 3] == '1' ) {
			nb_label("AGC S", false);
		} else if (replystr[p + 3] == '2' ) {
			nb_label("AGC M", false);
		} else if (replystr[p + 3] == '3' ) {
			nb_label("AGC F", false);
		}
	} else {
		cmd = "GC0;";
		if (wait_char(';', 5, 100, "get AGC", ASC) < 5) return val;

		size_t p = replystr.rfind("GC");
		if (p == string::npos) return val;
		if (replystr[p + 3] == '1' ) {
			nb_label("AGC S", false);
		} else if (replystr[p + 3] == '2' ) {
			nb_label("AGC M", false);
		} else if (replystr[p + 3] == '3' ) {
			nb_label("AGC F", false);
		}
	}
	return val;
}

//==============================================================================

void RIG_TS990::set_squelch(int val)
{
	if (useB) {
		cmd = "SQ1";
		cmd.append(to_decimal(abs(val),3)).append(";");
		sendCommand(cmd);
		showresp(INFO, ASC, "set squelch", cmd, "");
	} else {
		cmd = "SQ0";
		cmd.append(to_decimal(abs(val),3)).append(";");
		sendCommand(cmd);
		showresp(INFO, ASC, "set squelch", cmd, "");
	}
}

int  RIG_TS990::get_squelch()
{
	int val = 0;
	if (useB) {
		cmd = "SQ1;";
		if (wait_char(';', 7, 20, "get squelch", ASC) >= 7) {
			size_t p = replystr.rfind("SQ1");
			if (p == string::npos) return val;
			replystr[p + 6] = 0;
			val = atoi(&replystr[p + 3]);
		}
	} else {
		cmd = "SQ0;";
		if (wait_char(';', 7, 20, "get squelch", ASC) >= 7) {
			size_t p = replystr.rfind("SQ0");
			if (p == string::npos) return val;
			replystr[p + 6] = 0;
			val = atoi(&replystr[p + 3]);
		}
	}
	return val;
}

void RIG_TS990::get_squelch_min_max_step(int &min, int &max, int &step)
{
	min = 0; max = 255; step = 1;
}

void RIG_TS990::set_rf_gain(int val)
{
	if (useB) {
		cmd = "RG1";
		cmd.append(to_decimal(val,3)).append(";");
		sendCommand(cmd);
		showresp(INFO, ASC, "set rf gain", cmd, "");
	} else {
		cmd = "RG0";
		cmd.append(to_decimal(val,3)).append(";");
		sendCommand(cmd);
		showresp(INFO, ASC, "set rf gain", cmd, "");
	}
}

int  RIG_TS990::get_rf_gain()
{
	int val = progStatus.rfgain;
	if (useB) {
		cmd = "RG1;";
		if (wait_char(';', 7, 20, "get rf gain", ASC) < 7) return val;

		size_t p = replystr.rfind("RG");
		if (p != string::npos)
			val = fm_decimal(&replystr[p+3], 3);
	} else {
		cmd = "RG0;";
		if (wait_char(';', 7, 20, "get rf gain", ASC) < 7) return val;

		size_t p = replystr.rfind("RG");
		if (p != string::npos)
			val = fm_decimal(&replystr[p+3], 3);
	}
	return val;
}

void RIG_TS990::get_rf_min_max_step(int &min, int &max, int &step)
{
	min = 0;
	max = 255;
	step = 1;
}

static bool nr_on = false;

void RIG_TS990::set_noise_reduction(int val)
{
	if (useB) {
		cmd.assign("NR1").append(val ? "1" : "0" ).append(";");
		sendCommand(cmd);
		showresp(INFO, ASC, "SET noise reduction", cmd, "");
		if (val) nr_on = true;
		else nr_on = false;
	} else {
		cmd.assign("NR0").append(val ? "1" : "0" ).append(";");
		sendCommand(cmd);
		showresp(INFO, ASC, "SET noise reduction", cmd, "");
		if (val) nr_on = true;
		else nr_on = false;
	}
}

int  RIG_TS990::get_noise_reduction()
{
	int val = 0;
	if (useB) {
		cmd = rsp = "NR1";
		cmd.append(";");
		if (wait_char(';', 5, 20, "GET noise reduction", ASC) < 5) return val;

		size_t p = replystr.rfind(rsp);
		if (p == string::npos) return val;
		val = replystr[p+3] - '0';
		if (val == 1) nr_on = true;
		else nr_on = false;
		return val;
	} else {
		cmd = rsp = "NR0";
		cmd.append(";");
		if (wait_char(';', 5, 20, "GET noise reduction", ASC) < 5) return val;

		size_t p = replystr.rfind(rsp);
		if (p == string::npos) return val;
		val = replystr[p+3] - '0';
		if (val == 1) nr_on = true;
		else nr_on = false;
	}
	return val;
}

void RIG_TS990::set_noise_reduction_val(int val)
{
	if (useB) {
		cmd.assign("RL11").append(to_decimal(val, 2)).append(";");
		sendCommand(cmd);
		showresp(INFO, ASC, "SET_noise_reduction_val", cmd, "");
	} else {
		cmd.assign("RL10").append(to_decimal(val, 2)).append(";");
		sendCommand(cmd);
		showresp(INFO, ASC, "SET_noise_reduction_val", cmd, "");
	}
}

int  RIG_TS990::get_noise_reduction_val()
{
	int val = 0;
	if (useB) {
		if (!nr_on) return val;
		cmd = rsp = "RL11";
		cmd.append(";");
		if (wait_char(';', 7, 20, "GET noise reduction val", ASC) < 7) return val;
		size_t p = replystr.rfind(rsp);
		if (p == string::npos) return val;
		val = fm_decimal(&replystr[p+4], 2);
	} else {
		if (!nr_on) return val;
		cmd = rsp = "RL10";
		cmd.append(";");
		if (wait_char(';', 7, 20, "GET noise reduction val", ASC) < 7) return val;
		size_t p = replystr.rfind(rsp);
		if (p == string::npos) return val;
		val = fm_decimal(&replystr[p+4], 2);
	}
	return val;
}

void RIG_TS990::set_auto_notch(int v)
{
	if (useB) {
		cmd.assign("NT1").append(v ? "1" : "0" ).append(";");
		sendCommand(cmd);
		showresp(INFO, ASC, "SET Auto Notch", cmd, "");
	} else {
		cmd.assign("NT0").append(v ? "1" : "0" ).append(";");
		sendCommand(cmd);
		showresp(INFO, ASC, "SET Auto Notch", cmd, "");
	}
}

int  RIG_TS990::get_auto_notch()
{
	int val = 0;
	if (useB) {
		cmd = "NT1;";
		if (wait_char(';', 5, 20, "get auto notch", ASC) < 5) return val;
		size_t p = replystr.rfind("NT");
		if (p == string::npos) return val;
		if (replystr[p+3] == '1') val = 1;
	} else {
		cmd = "NT0;";
		if (wait_char(';', 5, 20, "get auto notch", ASC) < 5) return val;
		size_t p = replystr.rfind("NT");
		if (p == string::npos) return val;
		if (replystr[p+3] == '1') val = 1;
	}
	return val;
}

void RIG_TS990::set_notch(bool on, int val)
{
	if (useB) {
		if (on) {
			cmd.assign("NT12;");
			sendCommand(cmd);
			showresp(INFO, ASC, "Set notch ON", cmd, "");
			int bp = (int)(val * 127.0 / 3000);
			if (bp == 0) bp = 1;
			cmd.assign("BP1").append(to_decimal(bp, 3)).append(";");
			sendCommand(cmd);
			showresp(INFO, ASC, "set notch freq", cmd, "");
		} else {
			cmd.assign("NT10;");
			sendCommand(cmd);
			showresp(INFO, ASC, "Set notch OFF", cmd, "");
		}
	} else {
		if (on) {
			cmd.assign("NT02;");
			sendCommand(cmd);
			showresp(INFO, ASC, "Set notch ON", cmd, "");
			int bp = (int)(val * 127.0 / 3000);
			if (bp == 0) bp = 1;
			cmd.assign("BP0").append(to_decimal(bp, 3)).append(";");
			sendCommand(cmd);
			showresp(INFO, ASC, "set notch freq", cmd, "");
		} else {
			cmd.assign("NT00;");
			sendCommand(cmd);
			showresp(INFO, ASC, "Set notch OFF", cmd, "");
		}
	}
}

bool  RIG_TS990::get_notch(int &val)
{
	if (useB) {
		val = 1500;
		cmd = "NT1;";
		if (wait_char(';', 5, 20, "get notch state", ASC) < 5) return 0;
		size_t p = replystr.rfind("NT");
		if (p == string::npos)
			return 0;
		if (replystr[p+3] == '2') {
			cmd.assign("BP1;");
			if (wait_char(';', 7, 20, "get notch freq", ASC) < 7) return 0;
			size_t p = replystr.rfind("BP1");
			if (p != string::npos)
				val = (int)(atoi(&replystr[p+3]) * 3000 / 127.0);
			return 1;
		}
		return 0;
	} else {
		val = 1500;
		cmd = "NT0;";
		if (wait_char(';', 5, 20, "get notch state", ASC) < 5) return 0;
		size_t p = replystr.rfind("NT");
		if (p == string::npos)
			return 0;
		if (replystr[p+3] == '2') {
			cmd.assign("BP0;");
			if (wait_char(';', 7, 20, "get notch freq", ASC) < 7) return 0;
			size_t p = replystr.rfind("BP0");
			if (p != string::npos)
				val = (int)(atoi(&replystr[p+3]) * 3000.0 / 127.0);
			return 1;
		}
		return 0;
	}
	return 0;
}

void RIG_TS990::get_notch_min_max_step(int &min, int &max, int &step)
{
	min = 20;
	max = 3000;
	step = 10;
}


//==============================================================================

void RIG_TS990::set_if_shift(int val)  //Now doing Monitor
{
	progStatus.shift_val = val;
	cmd = "ML";
	cmd.append(to_decimal(val,3)).append(";");
	sendCommand(cmd);
	showresp(INFO, ASC, "set Mon Level", cmd, "");
}

bool RIG_TS990::get_if_shift(int &val)
{
	cmd = "ML;";
	if (wait_char(';', 6, 20, "get Mon Level", ASC) == 6) {
		size_t p = replystr.rfind("ML");
		if (p != string::npos) {
			val = fm_decimal(&replystr[p+2], 3);
			return true;
		}
	}
	val = progStatus.shift_val;
	return false;
}


void RIG_TS990::get_if_min_max_step(int &min, int &max, int &step)
{
	if_shift_min = min = 0;
	if_shift_max = max = 255;
	if_shift_step = step = 1;
	if_shift_mid = 10;
}

