#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include "core/StartupScreens.h"
#include "core/ActionSounds.h"
#include "core/ToolUseSounds.h"
#include "core/WindowConfig.h"
#include "game/ActionResolver.h"
#include "game/ChatCue.h"
#include "game/DayPhase.h"
#include "game/DialogAnimator.h"
#include "game/EnterSelection.h"
#include "game/StructureSelection.h"
#include "game/FarmSelection.h"
#include "game/FootstepCue.h"
#include "game/FractionStat.h"
#include "game/InputCommand.h"
#include "game/ObjectPicker.h"
#include "game/NoticeQueue.h"
#include "game/ObtainedBannerQueue.h"
#include "game/PlayerController.h"
#include "game/PropAnimator.h"
#include "game/PopupState.h"
#include "game/ResourceAudioCue.h"
#include "game/SettingsState.h"
#include "game/SceneTransition.h"
#include "game/ViewMotion.h"
#include "sdl/audio/AudioCatalog.h"
#include "sdl/audio/AudioMixer.h"
#include "sdl/audio/AudioPlayer.h"
#include "sdl/context/SdlContext.h"
#include "sdl/input/InputManager.h"
#include "sdl/render/BuildPlacePainter.h"
#include "sdl/render/ButtonPainter.h"
#include "sdl/render/Camera.h"
#include "sdl/render/ChatPainter.h"
#include "sdl/render/CursorPainter.h"
#include "sdl/render/DialogPainter.h"
#include "sdl/render/DigitPainter.h"
#include "sdl/render/HudOverlayPainter.h"
#include "sdl/render/InventoryHud.h"
#include "sdl/render/MarketPopupPainter.h"
#include "sdl/render/NameplatePainter.h"
#include "sdl/render/NoticePainter.h"
#include "sdl/render/NpcPopupPainter.h"
#include "sdl/render/ObtainedBannerHud.h"
#include "sdl/render/PromptPainter.h"
#include "sdl/render/Renderer.h"
#include "sdl/render/SettingsPainter.h"
#include "sdl/render/TextRenderer.h"
#include "world/CursorCatalog.h"
#include "world/CreatureCatalog.h"
#include "world/DialogCatalog.h"
#include "world/FarmCatalogs.h"
#include "world/InventoryLayout.h"
#include "world/MarketCatalog.h"
#include "game/MarketPopupState.h"
#include "game/MarketSetCycle.h"
#include "game/NpcPlacementResolver.h"
#include "world/NameplateLayout.h"
#include "world/NoticeLayout.h"
#include "world/NpcCatalog.h"
#include "world/PromptLayout.h"
#include "world/SceneDescription.h"
#include "world/SceneSwitcher.h"
#include "world/SettingsLayout.h"
#include "world/SceneSources.h"

namespace game {
class GameClient;
class EnemyTestTrigger;
struct LocalWorldView;
}  // namespace game

namespace world {
class Scenario;
class ScenePixelMapper;
class CharacterCatalog;
struct CharacterPalettes;
struct DigitCatalog;
struct InventoryLayout;
struct ObtainedBannerCatalog;
struct ResourceCatalog;
struct ToolOverlayCatalog;
}  // namespace world

namespace core {

// Una partida en curso: posee los objetos de la sesión de juego y
// orquesta cada frame (estado -> cámara -> pantallas de arranque ->
// intención del jugador -> audio -> dibujo). Application solo la
// construye y la corre; las decisiones viven en las clases que posee.
class PlaySession {
public:
    PlaySession(const WindowConfig& config, const world::Scenario& scenario,
                game::GameClient& gameClient, const world::CharacterCatalog& characters,
                const world::NpcCatalog& npcs, const world::MarketCatalog& market,
                const world::InventoryLayout& inventory,
                const world::ToolOverlayCatalog& toolOverlay,
                const world::ResourceCatalog& resources, const world::ObtainedBannerCatalog& banner,
                const world::DigitCatalog& digits, const world::CursorCatalog& cursors,
                const world::DialogCatalog& dialogs, const world::FontCatalog& font,
                const world::PromptLayout& prompt, const world::ChatLayout& chat,
                const world::NoticeLayout& notice,
                const world::NameplateLayout& nameplate,
                const world::SettingsLayout& settings,
                const world::HudOverlayCatalog& hudOverlays,
                const world::HudOverlayCatalog& lighting,
                const world::CharacterPalettes& palettes,
                game::EnemyTestTrigger* enemyTestTrigger = nullptr);

    // Corre el game loop hasta que se pida salir. Devuelve true si el
    // jugador abandonó la partida y hay que volver al lobby.
    bool run();

private:
    void tick(int iteration);

    // La escena actual (la que resuelve el switcher).
    const world::SceneDescription& scene() const;

    // Vista de presentación del tick: la de "meterse" (EnterSelection)
    // más el agradecimiento LOCAL del regalo. La consumen el movimiento,
    // el renderer y los diálogos; el estado de juego queda intacto.
    game::LocalWorldView presentationView(const game::LocalWorldView& view) const;

    // Marcador del lugar de plantado sobre la celda que apunta el puntero.
    void drawPlantingMarker(const world::ScenePixelMapper& cells);

    // Capa frontal del elemento donde está metido el jugador (lo que lo
    // tapa). QUÉ dibujar lo decidió EnterSelection; esto solo lo pinta,
    // sobre las entidades y bajo el HUD.
    void drawEnteredFront(const game::LocalWorldView& view, const world::ScenePixelMapper& cells);

    // El popup del HUD con la figura vacía del inventario y su
    // contenido (la pregunta o el avance). El cuándo lo dice
    // PopupState, el dónde InventoryLayout y el cómo los painters;
    // esto solo compone la llamada.
    void drawPopup(const world::PixelBox& panel);

    // Los nombres bajo los pies de las entidades que traen displayName.
    void drawNameplates(const game::LocalWorldView& view, int iteration);

    // El popup de un NPC: mismo panel que el resto y, adentro, el
    // retrato y el texto que dicen sus datos.
    void drawNpcPopup();

    // Popup del catálogo del mercado. Quien lo pide (hoy la tecla demo,
    // mañana el mensaje del server) solo llama a open/close; el dibujo
    // y los datos no cambian.
    void openMarketOfSpokenNpc();
    // DEMO (tecla B): abre el catálogo del comerciante de prueba, o lo
    // cierra si ya está abierto. No depende de la escena.
    void toggleDemoCatalog();
    void openMarketFromView(const game::LocalWorldView& view);
    void drawMarketCatalog();

    // Popup de configuración (ESC): se abre y se cierra con el mismo
    // comando, salvo que otro popup ya tenga el mundo bloqueado. QUÉ
    // contiene lo dicen SettingsLayout/SettingsState; esto solo compone.
    void toggleSettings();
    // La caja del panel (la grilla que digan sus datos, centrada).
    world::PixelBox settingsPanel() const;
    void drawSettings();
    bool overSettingsControl(const world::PixelPoint& point) const;
    // Click con el popup abierto: el botón del QR o empezar a arrastrar
    // un deslizador. El resto no hace nada (el mundo está bloqueado).
    void handleSettingsClick(const game::PointerClick& click);
    // Mientras el botón sigue apretado la perilla sigue al mouse; al
    // soltarlo termina el arrastre y se aplica el volumen al mixer.
    void dragSettingsSlider();
    // Fija el volumen de la categoría desde la x del mouse y lo aplica.
    void setVolumeFromPointer(world::SoundCategory category, int x);

    // Cierra el chat (ESC): descarta lo tipeado y suelta el teclado.
    void closeChat();

    // Borra el último caracter UTF-8 escrito
    void eraseLastCodepoint(std::string& text);

    // Corre la acción de un edificio tocado (tabla structureActions_).
    void runStructureAction(const world::SceneStructure& structure);

    // Abre el popup de un NPC de npcs.toml (su contenido) por id.
    void openStructurePopup(const std::string& npcId);

    // Pasa a otro mapa. PROVISORIO: hoy el cambio es local (como la
    // tecla M); cuando el server mande el mapa, esto solo manda la
    // intención. Un destino que ninguna fuente conoce se ignora.
    void enterStructureMap(const std::string& mapId);

    // Cambia la escena actual (en el negro de la transición): pide la
    // escena al switcher y reconstruye el índice del renderer, las
    // selecciones y la cámara. El dibujo no cambia: solo QUÉ escena se
    // pinta.
    void changeScene(const std::string& room);

    // La cámara sigue el rectángulo dibujado del jugador local, no su
    // celda lógica: así también se centra durante la interpolación del paso.
    void updateCamera(const game::LocalWorldView& view, const game::ViewMotion& motion,
                      int iteration);

    // ¿El objetivo del pick es interactivo? Lo dice el estado (la fuente
    // decide); esto solo lo lee para elegir la variante del cursor.
    bool isInteractive(const game::PickTarget& target, const game::LocalWorldView& view) const;

    // ¿El objetivo es OTRO jugador (no el local)? Lo decide la vista:
    // mismo tipo Player y un índice distinto de kLocalPlayerIndex.
    // Además exige algo transferible en el casillero activo: con una
    // herramienta el server no arma el pedido (ver isGiftTarget).
    bool isGiftTarget(const game::PickTarget& target, const game::LocalWorldView& view) const;

    // La escena actual: intercambiable al cambiar de escenario (la
    // ventana la sigue dando el Scenario). De dónde sale cada escena
    // lo resuelve el switcher a partir de sus fuentes (las salas de
    // scenes.toml y, si hay, los mapas del editor); las fuentes se
    // declaran ANTES porque el switcher las guarda prestadas.
    world::SceneSources sources_;
    world::SceneSwitcher sceneSwitcher_;
    game::GameClient& gameClient_;
    // ANDAMIAJE TEMPORAL (prueba): disparador de acciones de enemigos
    // del mundo simulado (null si no hay). Se va con las teclas de
    // prueba (G/F/H/K/J/L/V).
    game::EnemyTestTrigger* enemyTestTrigger_;
    // ANDAMIAJE TEMPORAL (prueba): despacho de esas teclas a la acción
    // que disparan (una fila por tecla). Se va con el disparador.
    std::map<game::InputCommand::Kind, world::ActionKind> testActions_;
    // Las dos teclas del proyectil (avanzar / desaparecer) también por
    // tabla: Kind -> método del disparador. Se va con el andamiaje.
    std::map<game::InputCommand::Kind, void (game::EnemyTestTrigger::*)()> projectileTests_;
    // Tecla de ataque con un proyectil concreto (hoy el fuego verde V):
    // Kind -> tipo de proyectil. Se va con el andamiaje.
    std::map<game::InputCommand::Kind, world::CreatureKind> attackProjectiles_;
    const world::CharacterCatalog& characters_;
    const world::NpcCatalog& npcCatalog_;
    game::NpcPlacementResolver npcPlacements_;
    const world::MarketCatalog& marketCatalog_;

    // El campo (cultivos, terreno, lugar de plantado) y el inventario
    // con los sacos y la cosecha ya registrados. Van ANTES de todo lo
    // que los consume: el HUD y el renderer guardan lo que reciben.
    world::FarmCatalogs farm_;
    world::InventoryLayout inventory_;

    sdl::SdlContext sdlContext_;
    sdl::Camera camera_;
    sdl::Renderer renderer_;
    sdl::DigitPainter digitPainter_;
    sdl::InventoryHud inventoryHud_;
    sdl::ObtainedBannerHud bannerHud_;
    // La herramienta única de texto: la comparten el popup y los HUD
    // (etiquetas de hud.toml), por eso se declara antes que ellos.
    sdl::TextRenderer textRenderer_;
    sdl::HudOverlayPainter hudOverlayPainter_;
    // Luz nocturna: mismo painter que el HUD fijo, con otro catálogo
    // (ui/lighting.toml) y dibujado antes de los HUD, modulado por la
    // intensidad de DayPhase.
    sdl::HudOverlayPainter lightingPainter_;
    game::ObtainedBannerQueue bannerQueue_;
    sdl::InputManager input_;
    sdl::AudioPlayer audio_;
    sdl::AudioMixer audioMixer_;

    StartupScreens screens_;
    game::ViewMotion motion_;
    game::FootstepCue footstepCue_;
    ToolUseSounds toolUseSounds_;
    ActionSounds actionSounds_;
    game::ResourceAudioCue resourceAudioCue_;
    // Avisa con un sonido cuando llega un mensaje de otro jugador.
    game::ChatCue chatCue_;
    game::FarmSelection farmSelection_;
    game::EnterSelection enterSelection_;
    // Edificios clickeables del editor (rol con comportamiento en el
    // overlay): qué hacer con cada acción es esta tabla, no un switch.
    game::StructureSelection structureSelection_;
    std::map<world::StructureAction, void (PlaySession::*)(const std::string&)> structureActions_;
    game::PopupState popupState_;
    // NPC con el popup abierto (nullptr = ninguno). El click lo abre y
    // cualquier click posterior lo cierra.
    const world::NpcDef* npcPopup_ = nullptr;
    // Id (npcs.toml) del último NPC con el que se habló: B abre el catálogo
    // de ESE comerciante. Provisorio hasta que el chat lo pida (validado por
    // el servidor). Vacío = ninguno.
    std::string lastNpcId_;
    // Sueño: último aviso recibido y si el jugador ya votó Sí en esta noche.
    int lastSleeping_ = 0;
    int lastTotal_ = 0;
    bool voted_ = false;
    // Día terminado: ya apretó OK y espera a que despierten los demás.
    bool woke_ = false;
    // Entre ServerDayEnd y ServerDay: el día terminó y se espera el OK de
    // todos. El sueño ya no cuenta (el popup de espera no puede reabrirse).
    bool dayEnding_ = false;
    // Texto tipeado; al Enter sale por gameClient_.sendChat.
    std::string chatInput_;
    // Última versión vista del historial: al cambiar, llegó un mensaje y
    // el preview se muestra hasta chatPreviewUntilTick_.
    int seenChatVersion_ = 0;
    int chatPreviewUntilTick_ = -1;
    game::SceneTransition sceneTransition_;
    // El momento del día (filtra los elementos `when` y gradúa la luz) y
    // el reloj del HUD. Entrada: el reloj del server (setClock en cada
    // tick).
    game::DayPhase dayPhase_;
    // Stats simulados del jugador: hoy los simula el cliente y el server
    // los reemplazará. La traducción a las claves del HUD queda en el
    // borde de dibujo, no en el estado.
    std::map<game::StatKind, game::FractionStat> stats_;
    // Regalo aceptado: el diálogo de agradecimiento es presentación
    // local hasta que el server mande la indicación. El reloj dura lo
    // que la animación (dato del catálogo), así se puede repetir.
    bool giftThanksActive_ = false;
    int giftThanksTicks_ = 0;
    // Pedido de regalo pendiente de respuesta: el emisor (senderId del
    // ServerGiftRequest que abrió el popup), que viaja en el
    // protocol::GiftResponseMessage. Sin pedido = nullopt.
    std::optional<std::uint8_t> pendingGiftSenderId_;
    game::ActionResolver actionResolver_;
    game::PlayerController player_;
    game::ObjectPicker picker_;
    sdl::BuildPlacePainter markerPainter_;
    world::CursorCatalog cursorCatalog_;
    sdl::CursorPainter cursorPainter_;
    game::DialogAnimator dialogAnimator_;
    world::DialogCatalog dialogCatalog_;
    sdl::DialogPainter dialogPainter_;
    world::PromptLayout promptLayout_;
    sdl::ButtonPainter buttonPainter_;
    sdl::PromptPainter promptPainter_;
    sdl::NpcPopupPainter npcPopupPainter_;
    sdl::MarketPopupPainter marketPopupPainter_;
    // Popup del catálogo del comerciante: se declara después de
    // marketCatalog_ y de inventory_ (guarda referencias a ambos).
    game::MarketPopupState marketPopup_;
    // PROVISORIO: estilo de catálogo elegido con N (el servidor lo rotará).
    game::MarketSetCycle marketCycle_;
    // Objetos animados clickeables (cofre): abierto/cerrado por id.
    game::PropAnimator props_;
    sdl::ChatPainter chatPainter_;
    // Aviso en pantalla ("Inventario lleno"): la fuente lo pide, la cola
    // cuenta el tiempo y el painter dibuja.
    game::NoticeQueue noticeQueue_;
    sdl::NoticePainter noticePainter_;
    // Nombre bajo los pies de cada jugador (ViewEntity::displayName).
    sdl::NameplatePainter nameplatePainter_;
    // Popup de configuración (ESC): datos, estado (volúmenes incluidos) y
    // dibujo. Se declaran después de los painters que comparte.
    world::SettingsLayout settingsLayout_;
    game::SettingsState settings_;
    sdl::SettingsPainter settingsPainter_;

    // Sonido por cada evento de recurso confirmado. Es dato: se arma una
    // vez en el constructor, no por frame.
    std::map<game::ResourceAudioEvent, sdl::SoundId> resourceSounds_;

    // Sonido de cada comando de input que lo tiene (hoy ESC: abrir/cerrar
    // la configuración y cerrar el chat). Sumar uno = una fila.
    std::map<game::InputCommand::Kind, sdl::SoundId> commandSounds_;
};

}  // namespace core
