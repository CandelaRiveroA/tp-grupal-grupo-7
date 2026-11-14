#pragma once

#include <QList>
#include <QMargins>
#include <QSize>
#include <string>

// Constantes del editor, agrupadas por archivo que las usa.
// Sin inline: por ser const, cada unidad de traduccion que las usa se queda
// con su propia copia (linkage interno). Hoy cada una la usa un solo archivo.
namespace editor {

// WorkspaceView.cpp
// Mapa que se muestra al abrir un mundo.
const std::string kHomeMapId = "farm";
// Ancho inicial de los paneles del cuerpo: arbol, lienzo y paleta.
const QList<int> kBodySplitterSizes{220, 600, 240};
const QSize kValidationDialogSize{480, 320};
constexpr int kHeadingIndent = 4;
constexpr int kTopBarSpacing = 8;

// MapTreeWidget.cpp
// Ancho minimo del boton de alta, para que no se corte su texto.
constexpr int kAddMapButtonMinWidth = 130;

// PaletteView.cpp
// Margen interno del panel y de cada grupo de la paleta.
const QMargins kPaletteMargins{4, 4, 4, 4};

}  // namespace editor