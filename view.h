#pragma once
#include <windows.h>

const int MNP_PADDING_CLIENT = 20;		// padding 20px
LPCTSTR   MNP_APPNAME = L"MyNotePad";
LPCTSTR	  MNP_FONTFACE = L"Microsoft Yahei UI Light";	// L"Lucida Console";
const int MNP_FONTSIZE = 28;
const int MNP_LINEHEIGHT = MNP_FONTSIZE;

const int MNP_BGCOLOR_EDIT = 0x00EEEEEE;
const int MNP_BGCOLOR_PREVIEW = 0x00F7F7F7;
const int MNP_BGCOLOR_SEL = 0x00CCCCCC;
const int MNP_FONTCOLOR = 0x00444444;

const int MNP_SCROLLBAR_BGCOLOR = 0x00E5E5E5;
const int MNP_SCROLLBAR_COLOR = 0x00D1D1D1;
const int MNP_SCROLLBAR_WIDTH = 14;

#include "model.h"
#include "controller.h"
