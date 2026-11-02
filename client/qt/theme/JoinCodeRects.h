// Constantes de la pantalla de unirse con codigo (JoinCodeScreen).

// Velo negro sobre el paisaje del titulo: 1 - 184/255 ~= 0.28 deja el cielo
// y las nubes al mismo brillo que la referencia (ui/Join screen.png).
#define JOIN_BACKDROP_DIM_ALPHA 184

// Pergamino (:/ui/join/scroll.png, 25x18): rollo izquierdo 12px, columna
// central estirable de 1px, rollo derecho 12px. Se ensambla a ancho logico
// y se escala x2.
#define JOIN_SCROLL_PIECE_W 12
#define JOIN_SCROLL_PIECE_H 18
#define JOIN_SCROLL_MID_X 12
#define JOIN_SCROLL_RIGHT_X 13
#define JOIN_SCROLL_LOGICAL_W 168

// Posicion del pergamino y hueco hasta la fila de ingreso (1280x720).
#define JOIN_SCROLL_TOP_Y 294
#define JOIN_SCROLL_ROW_GAP 22

// Marco de la caja de texto (:/ui/join/input_box.png, 18x18, Marco A):
// 9-slice con borde de 6px ensamblado a tamano logico y escalado x2.
#define JOIN_INPUT_SLICE 6
#define JOIN_INPUT_LOGICAL_W 139
#define JOIN_INPUT_LOGICAL_H 24
#define JOIN_INPUT_SCALE 2

// Margenes internos del texto dentro de la caja (px logicos).
#define JOIN_INPUT_PAD_LEFT 14
#define JOIN_INPUT_PAD_TOP 8
#define JOIN_INPUT_PAD_RIGHT 14
#define JOIN_INPUT_PAD_BOTTOM 8

// Boton OK (:/ui/join/ok.png: 16x16 logico guardado a x4 = 64x64).
// En pantalla va a 32x32 (16 x2): NO atender al asset de x4.
#define JOIN_OK_LOGICAL 16
#define JOIN_OK_SIZE 32
#define JOIN_HOVER_BRIGHT 115

// Hueco entre la caja de texto y el estado ("Conectando...").
#define JOIN_STATUS_ROW_GAP 8
