#pragma once

// Coordenadas de recorte del atlas TitleButtons.es-ES..png (400x655).

// Cartel "STARDEW VALLEY": franja superior completa del atlas.
#define ATLAS_LOGO_X 0
#define ATLAS_LOGO_Y 0
#define ATLAS_LOGO_W 400
#define ATLAS_LOGO_H 186

// Cartel en pantalla: el atlas se escala x2.
#define TITLE_LOGO_W (ATLAS_LOGO_W * 2)
#define TITLE_LOGO_H (ATLAS_LOGO_H * 2)

// Fila de botones: normal y hover, de izquierda a derecha
#define ATLAS_BUTTON_W 74
#define ATLAS_BUTTON_H 58
#define ATLAS_BUTTON_NORMAL_Y 186
#define ATLAS_BUTTON_HOVER_Y 244
#define ATLAS_BUTTON_COUNT 4

// Boton "Volver": franja derecha de la segunda fila (usado por JoinCodeScreen)
#define ATLAS_BACK_X 296
#define ATLAS_BACK_Y_NORMAL 252
#define ATLAS_BACK_Y_HOVER 279
#define ATLAS_BACK_W 66
#define ATLAS_BACK_H 27

// Tamano fijo de la ventana del lobby
#define TITLE_SCREEN_W 1280
#define TITLE_SCREEN_H 720

// Placa del username
#define TITLE_PLATE_W 288
#define TITLE_PLATE_H 72
#define TITLE_PLATE_MARGIN 12
#define TITLE_PLATE_TOP_Y 0
#define TITLE_PLATE_TEXT_INSET_X 32
#define TITLE_PLATE_TEXT_INSET_TOP 29
#define TITLE_PLATE_TEXT_INSET_BOTTOM 16
#define TITLE_PLATE_FONT_PX 22
#define TITLE_PLATE_FONT_FAMILY "StardewValley"
#define TITLE_PLATE_TEXT_COLOR "#7a5230"

// Geometria de la pantalla de inicio
#define TITLE_LOGO_TOP_Y 86                // margen superior del cartel
#define TITLE_BUTTON_ROW_GAP 148           // hueco cartel -> fila de botones
#define TITLE_BUTTON_ROW_BOTTOM_MARGIN 14  // margen inferior de la fila
#define TITLE_BUTTON_SPACING 16            // hueco entre botones vecinos
