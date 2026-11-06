#pragma once

// Medidas de las hojas de Stardew y del lienzo, en pixeles del arte. La pose
// (cuerpo y brazos) sale de characters.toml.

#define SHEET_CANVAS_W 32
#define SHEET_CANVAS_H 48
// Escala del sizeHint de CharacterPreview (pantalla de personaje).
#define SHEET_PREVIEW_HINT_SCALE 6
#define SHEET_BODY_X 8
// Sube el personaje para que los zapatos queden sobre la sombra del fondo.
#define SHEET_BODY_Y 9

#define SHEET_PUPIL_RGB 0x682b0f

// Peinados: se usan las 2 primeras filas de la hoja.
#define SHEET_HAIR_W 16
#define SHEET_HAIR_H 32
#define SHEET_HAIR_COLUMNS 16
#define SHEET_HAIR_STRIDE_Y 96
#define SHEET_HAIR_COUNT 32
#define SHEET_HAIR_OFFSET_Y 0

// Camisas: se usan las 3 primeras filas de la hoja.
#define SHEET_SHIRT_SIZE 8
#define SHEET_SHIRT_COLUMNS 16
#define SHEET_SHIRT_STRIDE_Y 32
#define SHEET_SHIRT_COUNT 48
#define SHEET_SHIRT_X 4
#define SHEET_SHIRT_Y 15

// Los brazos de la hoja traen 3 colores marcador que la camisa reemplaza.
#define SHEET_SLEEVE_DARK_RGB 0x4a0c06
#define SHEET_SLEEVE_MID_RGB 0x701718
#define SHEET_SLEEVE_LIGHT_RGB 0x8e1f0c
#define SHEET_SLEEVE_SAMPLE_X 0
#define SHEET_SLEEVE_DARK_Y 0
#define SHEET_SLEEVE_MID_Y 3
#define SHEET_SLEEVE_LIGHT_Y 1

#define SHEET_PANTS_W 16
#define SHEET_PANTS_H 32
#define SHEET_PANTS_STRIDE_X 192
#define SHEET_PANTS_CELLS 10
// Solo pantalones casi grises: el tinte multiplica y ensucia los de color propio.
#define SHEET_PANTS_DYEABLE_PERCENT 70
#define SHEET_PANTS_GREY_SPREAD 18

// Accesorios: indice 0 = ninguno. Con offset 2 el vello facial cae bajo los ojos.
#define SHEET_ACC_SIZE 16
#define SHEET_ACC_COLUMNS 8
#define SHEET_ACC_STRIDE_Y 32
#define SHEET_ACC_COUNT 20
#define SHEET_ACC_OFFSET_Y 2
