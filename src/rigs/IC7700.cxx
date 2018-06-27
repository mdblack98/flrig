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

#include "IC7700.h"
#include "debug.h"
#include "support.h"

//=============================================================================
// IC-7700

const char IC7700name_[] = "IC-7700";

const char *IC7700modes_[] = {
	"LSB", "USB", "AM", "CW", "RTTY", 
	"FM",  "CW-R", "RTTY-R", "PSK", "PSK-R", 
	"LSB-D", "USB-D", "AM-D", "FM-D", NULL};

enum {
	LSB7700, USB7700, AM7700, CW7700, RTTY7700,
	FM7700,  CWR7700, RTTYR7700, PSK7700, PSKR7700,
	LSBD7700, USBD7700, AMD7700, FMD7700 };

const char IC7700_mode_type[] = {
	'L', 'U', 'U', 'L', 'L', 'U', 'L', 'U', 'U', 'L',
	'L', 'U', 'U', 'U' };

const char IC7700_mode_nbr[] = {
	0x00, // Select the LSB mode
	0x01, // Select the USB mode
	0x02, // Select the AM mode
	0x03, // Select the CW mode
	0x04, // Select the RTTY mode
	0x05, // Select the FM mode
	0x07, // Select the CW-R mode
	0x08, // Select the RTTY-R mode
	0x12, // select PSK mode
	0x13, // select PSK-R mode
	0x00, // LSBD
	0x01, // USBD
	0x02, // AMD
	0x05 // FMD
};

const char *IC7700_ssb_bws[] = {
"50",    "100",  "150",  "200",  "250",  "300",  "350",  "400",  "450",  "500",
"600",   "700",  "800",  "900", "1000", "1100", "1200", "1300", "1400", "1500",
"1600", "1700", "1800", "1900", "2000", "2100", "2200", "2300", "2400", "2500",
"2600", "2700", "2800", "2900", "3000", "3100", "3200", "3300", "3400", "3500",
"3600",
NULL };

const char *IC7700_rtty_bws[] = {
"50",    "100",  "150",  "200",  "250",  "300",  "350",  "400",  "450",  "500",
"600",   "700",  "800",  "900", "1000", "1100", "1200", "1300", "1400", "1500",
"1600", "1700", "1800", "1900", "2000", "2100", "2200", "2300", "2400", "2500",
"2600", "2700",
NULL };

const char *IC7700_am_bws[] = {
"200",   "400",  "600",  "800", "1000", "1200", "1400", "1600", "1800", "2000",
"2200", "2400", "2600", "2800", "3000", "3200", "3400", "3600", "3800", "4000",
"4200", "4400", "4600", "4800", "5000", "5200", "5400", "5600", "5800", "6000",
"6200", "6400", "6600", "6800", "7000", "7200", "7400", "7700", "7800", "8000",
"8200", "8400", "8600", "8800", "9000", "9200", "9400", "9600", "9800", "10000", 
NULL };

const char *IC7700_fm_bws[] = {
"FIXED", NULL };

static GUI IC7700_widgets[]= {
	{ (Fl_Widget *)btnVol,        2, 125,  50 },	//0
	{ (Fl_Widget *)sldrVOLUME,   54, 125, 156 },	//1
	{ (Fl_Widget *)btnAGC,        2, 145,  50 },	//2
	{ (Fl_Widget *)sldrRFGAIN,   54, 145, 156 },	//3
	{ (Fl_Widget *)sldrSQUELCH,  54, 165, 156 },	//4
	{ (Fl_Widget *)btnNR,         2, 185,  50 },	//5
	{ (Fl_Widget *)sldrNR,       54, 185, 156 },	//6
	{ (Fl_Widget *)btnLOCK,     214, 105,  50 },	//7
	{ (Fl_Widget *)sldrINNER,   266, 105, 156 },	//8
	{ (Fl_Widget *)btnCLRPBT,   214, 125,  50 },	//9
	{ (Fl_Widget *)sldrOUTER,   266, 125, 156 },	//10
	{ (Fl_Widget *)btnNotch,    214, 145,  50 },	//11
	{ (Fl_Widget *)sldrNOTCH,   266, 145, 156 },	//12
	{ (Fl_Widget *)sldrMICGAIN, 266, 165, 156 },	//13
	{ (Fl_Widget *)sldrPOWER,   266, 185, 156 },	//14
	{ (Fl_Widget *)NULL, 0, 0, 0 }
};

void RIG_IC7700::initialize()
{
	IC7700_widgets[0].W = btnVol;
	IC7700_widgets[1].W = sldrVOLUME;
	IC7700_widgets[2].W = btnAGC;
	IC7700_widgets[3].W = sldrRFGAIN;
	IC7700_widgets[4].W = sldrSQUELCH;
	IC7700_widgets[5].W = btnNR;
	IC7700_widgets[6].W = sldrNR;
	IC7700_widgets[7].W = btnLOCK;
	IC7700_widgets[8].W = sldrINNER;
	IC7700_widgets[9].W = btnCLRPBT;
	IC7700_widgets[10].W = sldrOUTER;
	IC7700_widgets[11].W = btnNotch;
	IC7700_widgets[12].W = sldrNOTCH;
	IC7700_widgets[13].W = sldrMICGAIN;
	IC7700_widgets[14].W = sldrPOWER;
}

RIG_IC7700::RIG_IC7700() {
	defaultCIV = 0x74;
	name_ = IC7700name_;
	modes_ = IC7700modes_;
	bandwidths_ = IC7700_ssb_bws;
	_mode_type = IC7700_mode_type;
	adjustCIV(defaultCIV);

	comm_retries = 2;
	comm_wait = 20;
	comm_timeout = 50;
	comm_echo = true;
	comm_rtscts = false;
	comm_rtsplus = true;
	comm_dtrplus = true;
	comm_catptt = true;
	comm_rtsptt = false;
	comm_dtrptt = false;

	widgets = IC7700_widgets;

	has_pbt_controls = true;
	has_FILTER = true;

	has_extras =

	has_cw_wpm =
	has_cw_spot_tone =
	has_cw_qsk =
	has_cw_vol =

	has_vox_onoff =
	has_vox_gain =
	has_vox_anti =
	has_vox_hang =

	has_compON =
	has_compression =

	has_micgain_control =
	has_bandwidth_control = true;

	precision = 1;
	ndigits = 9;

	filA = filB = 1;

	def_freq = A.freq = 14070000;
	def_mode = A.imode = 1;
	def_bw = A.iBW = 34;

	B.freq = 7070000;
	B.imode = 1;
	B.iBW = 34;

};

//======================================================================
// IC7700 unique commands
//======================================================================

void RIG_IC7700::selectA()
{
	cmd = pre_to;
	cmd += '\x07';
	cmd += '\x00';
	cmd.append(post);
	waitFB("select A");
}

void RIG_IC7700::selectB()
{
	cmd = pre_to;
	cmd += '\x07';
	cmd += '\x01';
	cmd.append(post);
	waitFB("select B");
}

void RIG_IC7700::set_modeA(int val)
{
	A.imode = val;
	cmd = pre_to;
	cmd += '\x06';
	cmd += IC7700_mode_nbr[val];
	cmd += filA;
	cmd.append( post );
	waitFB("set mode A");
// digital set / clear
	if (val == LSBD7700 || val == USBD7700 || val == AMD7700 || val == FMD7700) {
		cmd = pre_to;
		cmd += '\x1A'; cmd += '\x06';
		cmd += '\x01'; cmd += '\x01';
		cmd += filA;
		cmd.append( post);
		waitFB("set digital");
	}
}

static const char *szfilter[] = {"1", "2", "3"};

int RIG_IC7700::get_modeA()
{
	int md = 0;
	cmd = pre_to;
	cmd += '\x04';
	cmd.append(post);
	string resp = pre_fm;
	resp += '\x04';
	if (waitFOR(8, "get mode A")) {
		size_t p = replystr.rfind(resp);
		for (md = LSB7700; md <= PSKR7700; md++)
			if (replystr[p+5] == IC7700_mode_nbr[md]) break;
		filA = replystr[p+6];

		if (md == LSB7700 || md == USB7700 || md == AM7700 || md == FM7700) {
			cmd = pre_to;
			cmd.append("\x1a\x06");
			cmd.append(post);
			resp = pre_fm;
			resp.append("\x1a\x06");
			if (waitFOR(9, "get digital setting")) {
				size_t p = replystr.rfind(resp);
				if (replystr[p+6] == 0x01) {
					if (md == LSB7700) md = LSBD7700;
					else if (md == USB7700) md = USBD7700;
					else if (md == AM7700) md = AMD7700;
					else if (md == FM7700) md = FMD7700;
				}
				if (replystr[p+7]) filA = replystr[p+7];
			}
		}
		A.imode = md;
	}
	return A.imode;
}

void RIG_IC7700::set_modeB(int val)
{
	B.imode = val;
	cmd = pre_to;
	cmd += '\x06';
	cmd += IC7700_mode_nbr[val];
	cmd += filB;
	cmd.append( post );
	waitFB("set mode B");
	if (val == LSBD7700 || val == USBD7700 || val == AMD7700 || val == FMD7700) {
		cmd = pre_to;
		cmd += '\x1A'; cmd += '\x06';
		cmd += '\x01'; cmd += '\x01';
		cmd += filB;
		cmd.append( post);
		waitFB("set digital");
	}
}

int RIG_IC7700::get_modeB()
{
	int md = 0;
	cmd = pre_to;
	cmd += '\x04';
	cmd.append(post);
	string resp = pre_fm;
	resp += '\x04';
	if (waitFOR(8, "get mode B")) {
		size_t p = replystr.rfind(resp);
		for (md = LSB7700; md <= PSKR7700; md++)
			if (replystr[p+5] == IC7700_mode_nbr[md]) break;
		filA = replystr[p+6];

		if (md == LSB7700 || md == USB7700 || md == AM7700 || md == FM7700) {
			cmd = pre_to;
			cmd.append("\x1a\x06");
			cmd.append(post);
			resp = pre_fm;
			resp.append("\x1a\x06");
			if (waitFOR(9, "get digital")) {
				size_t p = replystr.rfind(resp);
				if (replystr[p+6] == 0x01) {
					if (md == LSB7700) md = LSBD7700;
					else if (md == USB7700) md = USBD7700;
					else if (md == AM7700) md = AMD7700;
					else if (md == FM7700) md = FMD7700;
				}
				if (replystr[p+7]) filB = replystr[p+7];
			}
		}
		B.imode = md;
	}
	return B.imode;
}

int RIG_IC7700::get_bwA()
{
	cmd = pre_to;
	cmd.append("\x1a\x03");
	cmd.append(post);
	string resp = pre_fm;
	resp.append("\x1a\x03");
	if (waitFOR(8, "get bw A")) {
		size_t p = replystr.rfind(resp);
		A.iBW = fm_bcd(replystr.substr(p+6), 2);
	}
	return A.iBW;
}

void RIG_IC7700::set_bwA(int val)
{
	if (A.imode == FM7700 || A.imode == FMD7700) return;
	A.iBW = val;
	cmd = pre_to;
	cmd.append("\x1a\x03");
	cmd.append(to_bcd(A.iBW, 2));
	cmd.append(post);
	waitFB("set bw A");
}

int RIG_IC7700::get_bwB()
{
	cmd = pre_to;
	cmd.append("\x1a\x03");
	cmd.append(post);
	string resp = pre_fm;
	resp.append("\x1a\x03");
	if (waitFOR(8, "get bw B")) {
		size_t p = replystr.rfind(resp);
		B.iBW = fm_bcd(replystr.substr(p+6), 2);
	}
	return B.iBW;
}

void RIG_IC7700::set_bwB(int val)
{
	if (B.imode == FM7700 || B.imode == FMD7700) return;
	B.iBW = val;
	cmd = pre_to;
	cmd.append("\x1a\x03");
	cmd.append(to_bcd(B.iBW, 2));
	cmd.append(post);
	waitFB("set bw A");
}

int RIG_IC7700::adjust_bandwidth(int m)
{
	switch (m) {
		case USB7700: case LSB7700: case LSBD7700 : case USBD7700:
			bandwidths_ = IC7700_ssb_bws;
			return 34;
		case AM7700:
			bandwidths_ = IC7700_am_bws;
			return 29;
			break;
		case FM7700: case FMD7700:
			bandwidths_ = IC7700_fm_bws;
			return 0;
			break;
		case RTTY7700: case RTTYR7700:
			bandwidths_ = IC7700_rtty_bws;
			return 12;
			break;
		case CW7700: case CWR7700:
			bandwidths_ = IC7700_ssb_bws;
			return 10;
			break;
		default:
			bandwidths_ = IC7700_ssb_bws;
			return 34;
	}
	return 0;
}

const char ** RIG_IC7700::bwtable(int m)
{
	const char **table;
	switch (m) {
		case AM7700:
			table = IC7700_am_bws;
			break;
		case FM7700: case FMD7700:
			table = IC7700_fm_bws;
			break;
		case RTTY7700: case RTTYR7700:
			table = IC7700_rtty_bws;
			break;
		case CW7700: case CWR7700:
		case USB7700: case LSB7700: case LSBD7700 : case USBD7700:
		default:
			table = IC7700_ssb_bws;
	}
	return table;
}

int RIG_IC7700::def_bandwidth(int m)
{
	return adjust_bandwidth(m);
}

void RIG_IC7700::set_mic_gain(int v)
{
	ICvol = (int)(v * 255 / 100);
	cmd = pre_to;
	cmd.append("\x14\x0B");
	cmd.append(to_bcd(ICvol, 3));
	cmd.append( post );
	waitFB("set mic gain");
}

void RIG_IC7700::set_attenuator(int val)
{
	int cmdval = 0;
	if (atten_level == 0) {
		atten_level = 1;
		atten_label("20 dB", true);
		cmdval = 0x20;
	} else if (atten_level == 1) {
		atten_level = 0;
		atten_label("Att", false);
		cmdval = 0x00;
	}
	cmd = pre_to;
	cmd += '\x11';
	cmd += cmdval;
	cmd.append( post );
	waitFB("set attenuator");
}

int RIG_IC7700::get_attenuator()
{
	cmd = pre_to;
	cmd += '\x11';
	cmd.append( post );
	string resp = pre_fm;
	resp += '\x11';
	if (waitFOR(7, "get attenuator")) {
		size_t p = replystr.rfind(resp);
		if (replystr[p+6] == 0x20) {
			atten_level = 1;
			atten_label("20 dB", true);
		} else {
			atten_level = 0;
			atten_label("Att", false);
		}
	}
	return atten_level;
}

void RIG_IC7700::set_compression(int on, int val)
{
	if (on) {
		cmd.assign(pre_to).append("\x14\x0E");
		cmd.append(to_bcd(val * 255 / 100, 3));
		cmd.append( post );
		waitFB("set comp");

		cmd = pre_to;
		cmd.append("\x16\x44");
		cmd += '\x01';
		cmd.append(post);
		waitFB("set Comp ON");

	} else{
		cmd.assign(pre_to).append("\x16\x44");
		cmd += '\x00';
		cmd.append(post);
		waitFB("set Comp OFF");
	}
}

void RIG_IC7700::set_vox_onoff()
{
	if (progStatus.vox_onoff) {
		cmd.assign(pre_to).append("\x16\x46\x01");
		cmd.append( post );
		waitFB("set vox ON");
	} else {
		cmd.assign(pre_to).append("\x16\x46");
		cmd += '\x00';
		cmd.append( post );
		waitFB("set vox OFF");
	}
}

void RIG_IC7700::set_vox_gain()
{
	cmd.assign(pre_to).append("\x1A\x05"); // values 0-255 = 0 - 100%
	cmd +='\x00';
	cmd +='\x87';
	cmd.append(to_bcd((int)(progStatus.vox_gain * 2.55), 3));
	cmd.append( post );
	waitFB("SET vox gain");
}

void RIG_IC7700::set_vox_anti()
{
	cmd.assign(pre_to).append("\x1A\x05");	// values 0-255 = 0 - 100%
	cmd +='\x00';
	cmd +='\x88';
	cmd.append(to_bcd((int)(progStatus.vox_anti * 2.55), 3));
	cmd.append( post );
	waitFB("SET anti-vox");
}

void RIG_IC7700::set_vox_hang()
{
	cmd.assign(pre_to).append("\x1A\x05");	// values 00-20 = 0.0 - 2.0 sec
	cmd +='\x00';
	cmd +='\x89';
	cmd.append(to_bcd((int)(progStatus.vox_hang / 10 ), 2));
	cmd.append( post );
	waitFB("SET vox hang");
}

// CW controls

void RIG_IC7700::set_cw_wpm()
{
	cmd.assign(pre_to).append("\x14\x0C"); // values 0-255 = 6 to 48 WPM
	cmd.append(to_bcd(round((progStatus.cw_wpm - 6) * 255 / (48 - 6)), 3));
	cmd.append( post );
	waitFB("SET cw wpm");
}

void RIG_IC7700::set_cw_qsk()
{
	int n = round(progStatus.cw_qsk * 10); // values 0-255
	cmd.assign(pre_to).append("\x14\x0F");
	cmd.append(to_bcd(n, 3));
	cmd.append(post);
	waitFB("Set cw qsk delay");
}

void RIG_IC7700::set_cw_spot_tone()
{
	cmd.assign(pre_to).append("\x14\x09"); // values 0=300Hz 255=900Hz
	int n = round((progStatus.cw_spot_tone - 300) * 255.0 / 600.0);
	if (n > 255) n = 255;
	if (n < 0) n = 0;
	cmd.append(to_bcd(n, 3));
	cmd.append( post );
	waitFB("SET cw spot tone");
}

void RIG_IC7700::set_cw_vol()
{
	cmd.assign(pre_to);
	cmd.append("\x14\x15");
	cmd.append(to_bcd((int)(progStatus.cw_vol * 2.55), 3));
	cmd.append( post );
	waitFB("SET cw sidetone volume");
}

// Tranceiver PTT on/off
void RIG_IC7700::set_PTT_control(int val)
{
	cmd = pre_to;
	cmd += '\x1c';
	cmd += '\x00';
	cmd += (unsigned char) val;
	cmd.append( post );
	waitFB("set ptt");
	ptt_ = val;
}

int RIG_IC7700::get_PTT()
{
	cmd = pre_to;
	cmd += '\x1c'; cmd += '\x00';
	string resp = pre_fm;
	resp += '\x1c'; resp += '\x00';
	cmd.append(post);
	if (waitFOR(8, "get PTT")) {
		size_t p = replystr.rfind(resp);
		if (p != string::npos)
			ptt_ = replystr[p + 6];
	}
	return ptt_;
}

void RIG_IC7700::set_pbt_inner(int val)
{
	int shift = 128 + val * 128 / 50;
	if (shift < 0) shift = 0;
	if (shift > 255) shift = 255;

	cmd = pre_to;
	cmd.append("\x14\x07");
	cmd.append(to_bcd(shift, 3));
	cmd.append(post);
	waitFB("set PBT inner");
}

void RIG_IC7700::set_pbt_outer(int val)
{
	int shift = 128 + val * 128 / 50;
	if (shift < 0) shift = 0;
	if (shift > 255) shift = 255;

	cmd = pre_to;
	cmd.append("\x14\x08");
	cmd.append(to_bcd(shift, 3));
	cmd.append(post);
	waitFB("set PBT outer");
}

const char *RIG_IC7700::FILT(int &val)
{
	if (useB) {
		if (filB < 0) filB = 0;
		if (filB > 3) filB = 3;
		val = filB;
		return(szfilter[filB - 1]);
	}
	else {
		if (filA < 0) filA = 0;
		if (filA > 3) filA = 3;
		val = filA;
		return (szfilter[filA - 1]);
	}
}

const char *RIG_IC7700::nextFILT()
{
	if (useB) {
		filB++;
		if (filB > 3) filB = 1;
		set_modeB(B.imode);
		return(szfilter[filB - 1]);
	} else {
		filA++;
		if (filA > 3) filA = 1;
		set_modeA(A.imode);
		return(szfilter[filA - 1]);
	}
}

