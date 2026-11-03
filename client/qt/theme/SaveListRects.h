#pragma once

#include "CharacterSheetRects.h"
#include "TitleSpriteRects.h"

// Ventana exterior de madera (join_input_box.png a x2). Aloja exactamente
// 4 ranuras de partida sin separacion extra.
#define SAVE_WINDOW_SCALE 2
#define SAVE_WINDOW_SLICE 6
#define SAVE_WINDOW_LOGICAL_W 416
#define SAVE_WINDOW_LOGICAL_H 248
#define SAVE_WINDOW_W (SAVE_WINDOW_LOGICAL_W * SAVE_WINDOW_SCALE)
#define SAVE_WINDOW_H (SAVE_WINDOW_LOGICAL_H * SAVE_WINDOW_SCALE)
#define SAVE_WINDOW_PAD 12

// Ranura de partida (char_recessed_box.png a x2)
#define SAVE_ROW_SCALE 2
#define SAVE_ROW_SLICE 5
#define SAVE_ROW_LOGICAL_W 404
#define SAVE_ROW_LOGICAL_H 59
#define SAVE_ROW_W (SAVE_ROW_LOGICAL_W * SAVE_ROW_SCALE)
#define SAVE_ROW_H (SAVE_ROW_LOGICAL_H * SAVE_ROW_SCALE)
#define SAVE_VISIBLE_ROWS 4

// Flechas de scroll (ui/save/arrow_{up,down}.png a x2), pegadas al borde
// derecho del panel
#define SAVE_SCROLL_ARROW_W 22
#define SAVE_SCROLL_ARROW_H 24
#define SAVE_SCROLL_ARROW_GAP_TOP 12
#define SAVE_SCROLL_ARROW_GAP_BOTTOM 16

// Iconos de la ranura (ui/save/{coin,clock,delete}.png a x2)
#define SAVE_COIN_W 18
#define SAVE_COIN_H 18
#define SAVE_CLOCK_W 18
#define SAVE_CLOCK_H 30

// Badge "Cooperativa" (placa de madera): el PNG de 320x86 se reduce a la mitad.
#define SAVE_COOP_BADGE_W 160
#define SAVE_COOP_BADGE_H 43

#define SAVE_DELETE_SIZE 24

// Layout raiz de la pantalla. El panel va centrado en la ventana del lobby y
// "Volver" queda anclado abajo a la derecha; los stretch salen de esa geometria.
#define SAVE_ROOT_MARGIN_LEFT 48
#define SAVE_ROOT_MARGIN_TOP 24
#define SAVE_ROOT_MARGIN_RIGHT 28
#define SAVE_ROOT_MARGIN_BOTTOM 12

#define SAVE_WINDOW_X ((TITLE_SCREEN_W - SAVE_WINDOW_W) / 2)
#define SAVE_WINDOW_Y ((TITLE_SCREEN_H - SAVE_WINDOW_H) / 2)

// Separacion entre el panel y las flechas, y alto del sprite de "Volver",
// que TitleSprites escala x2.
#define SAVE_ARROWS_GAP 12
#define SAVE_BACK_SCALE 2
#define SAVE_BACK_H (ATLAS_BACK_H * SAVE_BACK_SCALE)

// Factores de stretch que dejan el panel en (SAVE_WINDOW_X, SAVE_WINDOW_Y).
#define SAVE_STRETCH_TOP (SAVE_WINDOW_Y - SAVE_ROOT_MARGIN_TOP)
#define SAVE_STRETCH_LEFT (SAVE_WINDOW_X - SAVE_ROOT_MARGIN_LEFT)
#define SAVE_STRETCH_RIGHT                                                   \
    (TITLE_SCREEN_W - SAVE_ROOT_MARGIN_LEFT - SAVE_ROOT_MARGIN_RIGHT -       \
     SAVE_STRETCH_LEFT - SAVE_WINDOW_W - SAVE_ARROWS_GAP - SAVE_SCROLL_ARROW_W)
#define SAVE_STRETCH_BOTTOM                                                  \
    (TITLE_SCREEN_H - SAVE_WINDOW_Y - SAVE_WINDOW_H - SAVE_BACK_H -          \
     SAVE_ROOT_MARGIN_BOTTOM)

static_assert(SAVE_STRETCH_TOP > 0 && SAVE_STRETCH_LEFT > 0 && SAVE_STRETCH_RIGHT > 0 &&
                      SAVE_STRETCH_BOTTOM > 0,
              "el panel de partidas no entra en la ventana del lobby");

// Tinte del hover: multiplica el relleno crema por (0.98, 0.92, 0.65) hacia
// el trigo (224,180,81) de la referencia; los bordes oscuros quedan intactos.
#define SAVE_HOVER_TINT_R 98
#define SAVE_HOVER_TINT_G 92
#define SAVE_HOVER_TINT_B 65
#define SAVE_HOVER_KEEP_MAX 100
#define SAVE_HOVER_BRIGHT 115
// Los tintes son porcentajes y el promedio es sobre los 3 canales RGB.
#define SAVE_PERCENT 100
#define SAVE_RGB_CHANNELS 3

// Interior de la ranura (SaveRowButton): margenes y separaciones.
#define SAVE_ROW_MARGIN_LEFT 18
#define SAVE_ROW_MARGIN_TOP 10
#define SAVE_ROW_MARGIN_RIGHT 14
#define SAVE_ROW_MARGIN_BOTTOM 10
#define SAVE_ROW_SPACING 8      // entre indice, retrato y columnas
#define SAVE_ROW_TEXT_GAP 4     // entre lineas de una misma columna
#define SAVE_ROW_ICON_GAP 4     // entre un icono (moneda, reloj) y su texto
#define SAVE_ROW_STATS_GAP 16   // entre el oro y el tiempo jugado

// Retrato de la ranura: el lienzo del catalogo a x2 (64x96).
#define SAVE_PORTRAIT_SCALE 2
#define SAVE_PORTRAIT_W (SHEET_CANVAS_W * SAVE_PORTRAIT_SCALE)
#define SAVE_PORTRAIT_H (SHEET_CANVAS_H * SAVE_PORTRAIT_SCALE)

// Ranura deshabilitada: misma opacidad que TitleButton.
#define SAVE_DISABLED_OPACITY 0.5

// Formato del oro ("404.316o") y del tiempo jugado ("54:40").
#define SAVE_GOLD_GROUP_DIGITS 3
#define SAVE_SECONDS_PER_HOUR 3600
#define SAVE_SECONDS_PER_MINUTE 60
#define SAVE_MINUTES_WIDTH 2
#define SAVE_DECIMAL_BASE 10
