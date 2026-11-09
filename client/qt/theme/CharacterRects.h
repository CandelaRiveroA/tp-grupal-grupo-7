#pragma once

// Medidas de la pantalla de personalizar personaje

// Ventana de madera (join_input_box.png a x2). Ancho = retrato + hueco +
// formulario + 2 rellenos.
#define CHAR_WINDOW_SCALE 2
#define CHAR_WINDOW_SLICE 6
#define CHAR_WINDOW_LOGICAL_W 278
#define CHAR_WINDOW_LOGICAL_H 280
#define CHAR_WINDOW_W (CHAR_WINDOW_LOGICAL_W * CHAR_WINDOW_SCALE)
#define CHAR_WINDOW_H (CHAR_WINDOW_LOGICAL_H * CHAR_WINDOW_SCALE)
#define CHAR_WINDOW_PAD 24
#define CHAR_PREVIEW_COL_W 216
#define CHAR_COLUMN_GAP 31
#define CHAR_FORM_COL_W 261

// Retrato: canvas 32x48 del catalogo a x4.
#define CHAR_PREVIEW_W 128
#define CHAR_PREVIEW_H 192

#define CHAR_ARROW_W 24
#define CHAR_ARROW_H 22

#define CHAR_OK_LOGICAL 16
#define CHAR_OK_SIZE 32

#define CHAR_HOVER_BRIGHT 112

// textBox de Stardew bajado a 48x12 y ampliado x3; los margenes incluyen la
// sombra (izquierda y abajo), por eso no son simetricos.
#define CHAR_INPUT_SOURCE_DIV 4
#define CHAR_INPUT_MARGIN_LEFT 3
#define CHAR_INPUT_MARGIN_TOP 2
#define CHAR_INPUT_MARGIN_RIGHT 2
#define CHAR_INPUT_MARGIN_BOTTOM 2
#define CHAR_INPUT_LOGICAL_W 87
#define CHAR_INPUT_LOGICAL_H 12
#define CHAR_INPUT_SCALE 3

// Maximo de caracteres de las cajas; pasado el ancho util el campo hace scroll.
#define CHAR_INPUT_MAX_CHARS 24

#define CHAR_INPUT_PAD_LEFT 10
#define CHAR_INPUT_PAD_TOP 6
#define CHAR_INPUT_PAD_RIGHT 6
#define CHAR_INPUT_PAD_BOTTOM 6

#define CHAR_INPUT_LABEL_GAP 4

// Hueco uniforme entre el retrato y las filas de apariencia
#define CHAR_ROW_GAP 4
