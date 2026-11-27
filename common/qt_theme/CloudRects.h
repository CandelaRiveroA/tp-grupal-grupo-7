#pragma once

// Recortes de las 3 nubes blancas de Clouds.png (512x1002) y sus zonas en
// la pantalla de inicio (1280x720).

// Nube blanca N: rect (x, y, w, h) en Clouds.png.
#define CLOUD_WHITE_1_X 152
#define CLOUD_WHITE_1_Y 447
#define CLOUD_WHITE_1_W 123
#define CLOUD_WHITE_1_H 55

#define CLOUD_WHITE_2_X 410
#define CLOUD_WHITE_2_Y 467
#define CLOUD_WHITE_2_W 64
#define CLOUD_WHITE_2_H 36

#define CLOUD_WHITE_3_X 0
#define CLOUD_WHITE_3_Y 471
#define CLOUD_WHITE_3_W 149
#define CLOUD_WHITE_3_H 66

// Zona N: sprite (1..3) y destino pre-escalado en pantalla (una copia por
// zona; el orden de pintado lo da TitleBackdrop). Las posiciones cortadas
// por el borde son negativas y Qt las recorta; la zona 8 es parte de la
// nube gigante de la zona 4. CLOUD_ZONE_COUNT: total de zonas definidas.

#define CLOUD_ZONE_COUNT 10

#define CLOUD_ZONE_1_SPRITE 3
#define CLOUD_ZONE_1_X (-25)
#define CLOUD_ZONE_1_Y 329
#define CLOUD_ZONE_1_W 328
#define CLOUD_ZONE_1_H 145

#define CLOUD_ZONE_2_SPRITE 3
#define CLOUD_ZONE_2_X 1020
#define CLOUD_ZONE_2_Y 251
#define CLOUD_ZONE_2_W 179
#define CLOUD_ZONE_2_H 79

#define CLOUD_ZONE_3_SPRITE 1
#define CLOUD_ZONE_3_X 827
#define CLOUD_ZONE_3_Y 454
#define CLOUD_ZONE_3_W 234
#define CLOUD_ZONE_3_H 105

#define CLOUD_ZONE_4_SPRITE 3
#define CLOUD_ZONE_4_X 860
#define CLOUD_ZONE_4_Y 575
#define CLOUD_ZONE_4_W 492
#define CLOUD_ZONE_4_H 218

#define CLOUD_ZONE_5_SPRITE 1
#define CLOUD_ZONE_5_X (-150)
#define CLOUD_ZONE_5_Y 600
#define CLOUD_ZONE_5_W 271
#define CLOUD_ZONE_5_H 121

#define CLOUD_ZONE_6_SPRITE 2
#define CLOUD_ZONE_6_X 363
#define CLOUD_ZONE_6_Y 434
#define CLOUD_ZONE_6_W 70
#define CLOUD_ZONE_6_H 40

#define CLOUD_ZONE_7_SPRITE 1
#define CLOUD_ZONE_7_X (-126)
#define CLOUD_ZONE_7_Y 200
#define CLOUD_ZONE_7_W 172
#define CLOUD_ZONE_7_H 77

#define CLOUD_ZONE_8_SPRITE 2
#define CLOUD_ZONE_8_X 65
#define CLOUD_ZONE_8_Y 677
#define CLOUD_ZONE_8_W 77
#define CLOUD_ZONE_8_H 43

#define CLOUD_ZONE_9_SPRITE 1
#define CLOUD_ZONE_9_X 55
#define CLOUD_ZONE_9_Y 45
#define CLOUD_ZONE_9_W 246
#define CLOUD_ZONE_9_H 110

#define CLOUD_ZONE_10_SPRITE 2
#define CLOUD_ZONE_10_X 1090
#define CLOUD_ZONE_10_Y 55
#define CLOUD_ZONE_10_W 96
#define CLOUD_ZONE_10_H 54

//Animacion hacia la izquierda.El tick busca ~30 fps.
#define CLOUD_ANIMATION_TICK_MS 33
#define CLOUD_DRIFT_PX_PER_S_SMALL 4
#define CLOUD_DRIFT_PX_PER_S_MEDIUM 12
#define CLOUD_DRIFT_PX_PER_S_LARGE 24
