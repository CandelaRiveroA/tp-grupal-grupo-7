#pragma once

namespace world {

// Caja en píxeles (misma forma que world::PixelBox pero local a esta capa
// pura para no arrastrar dependencias).
struct BannerBox {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

// Geometría PURA del cartel: dónde va el panel de un stackIndex dado y
// dónde el icono dentro del panel. Sin SDL. Permite testear el layout.
//
// La pila está anclada a la IZQUIERDA y centrada en vertical respecto del
// primer cartel; crece hacia ABAJO (stackIndex 0 arriba).
class ObtainedBannerLayout {
public:
    // panelSide: lado del panel en px (cuadrado). marginX: separación del
    // borde izquierdo de la ventana. stackGap: separación vertical entre
    // paneles. iconMargin: margen interior del icono. iconOffsetX/Y: ajuste
    // fino del icono para compensar el aire de su celda.
    ObtainedBannerLayout(int windowWidth, int windowHeight, int panelSide, int marginX,
                         int stackGap, int iconMargin, int iconOffsetX = 0,
                         int iconOffsetY = 0);

    // Caja del panel para un cartel en la posición stackIndex de la pila.
    BannerBox panelBox(int stackIndex) const;

    // Caja del icono (cuadrado) centrado dentro del panel dado, con el
    // margen interior configurado.
    BannerBox iconBox(const BannerBox& panel) const;

private:
    int windowWidth_;
    int windowHeight_;
    int panelSide_;
    int marginX_;
    int stackGap_;
    int iconMargin_;
    int iconOffsetX_;
    int iconOffsetY_;
};

}  // namespace world
