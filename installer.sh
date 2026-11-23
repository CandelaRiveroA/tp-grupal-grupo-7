#!/usr/bin/env bash
# Instalador / desinstalador del TP Final - Grupo 7.
# Uso:
#   ./installer.sh            -> instala
#   ./installer.sh --uninstall -> desinstala

set -euo pipefail

# ---------- Colores ----------
GREEN='\033[0;32m'
BLUE='\033[94m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

# ---------- Configuracion ----------
APP_NAME="taller"
PREFIX="/opt/${APP_NAME}"
BIN_DIR="/usr/local/bin"
REPO_URL="https://github.com/CandelaRiveroA/tp-grupal-grupo-7"
SRC_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CLONE=0

# Flags se procesan mas abajo, despues de asegurar root.

# Usuario real que invoco sudo (para los accesos directos del Desktop).
if [[ -n "${SUDO_USER:-}" ]]; then
    TARGET_USER="$SUDO_USER"
elif command -v logname >/dev/null 2>&1; then
    TARGET_USER="$(logname 2>/dev/null || echo root)"
else
    TARGET_USER="${USER:-root}"
fi
TARGET_HOME="$(getent passwd "$TARGET_USER" | cut -d: -f6)"
if [[ -z "$TARGET_HOME" || ! -d "$TARGET_HOME" ]]; then
    TARGET_HOME="/root"
    TARGET_USER="root"
fi

# Directorio del Escritorio (respeta idioma: Desktop, Escritorio, etc.).
if command -v xdg-user-dir >/dev/null 2>&1 && [[ $EUID -eq 0 ]]; then
    # xdg-user-dir debe correr como el usuario real.
    DESKTOP_DIR="$(sudo -u "$TARGET_USER" xdg-user-dir DESKTOP 2>/dev/null || true)"
fi
if [[ -z "${DESKTOP_DIR:-}" || ! -d "$DESKTOP_DIR" ]]; then
    for cand in "$TARGET_HOME/Desktop" "$TARGET_HOME/Escritorio" "$TARGET_HOME/Desktop"; do
        if [[ -d "$cand" ]]; then
            DESKTOP_DIR="$cand"
            break
        fi
    done
fi
if [[ -z "${DESKTOP_DIR:-}" ]]; then
    DESKTOP_DIR="$TARGET_HOME/Desktop"
fi

# ---------- Helpers ----------
info()  { echo -e "${BLUE}$*${NC}"; }
ok()    { echo -e "${GREEN}$*${NC}"; }
warn()  { echo -e "${YELLOW}$*${NC}"; }
fail()  { echo -e "${RED}$*${NC}"; exit 1; }

# Re-ejecutar con sudo si no somos root.
if [[ $EUID -ne 0 ]]; then
    exec sudo "$0" "$@"
fi

# Procesar flags.
UNINSTALL=0
KEEP_DEPS=0
for arg in "$@"; do
    case "$arg" in
        --uninstall|uninstall) UNINSTALL=1 ;;
        --keep-deps|--no-remove-deps) KEEP_DEPS=1 ;;
        --clone|--from-github) CLONE=1 ;;
    esac
done

uninstall() {
    info "Desinstalando ${APP_NAME}..."

    rm -f "${BIN_DIR}/taller_client" \
          "${BIN_DIR}/taller_server" \
          "${BIN_DIR}/taller_editor" \
          "${BIN_DIR}/taller_uninstall"

    # Accesos directos del Desktop del usuario invocante.
    if [[ -d "$DESKTOP_DIR" ]]; then
        rm -f "${DESKTOP_DIR}/taller_client.sh" \
              "${DESKTOP_DIR}/taller_server.sh" \
              "${DESKTOP_DIR}/taller_editor.sh" \
              "${DESKTOP_DIR}/taller_client.desktop" \
              "${DESKTOP_DIR}/taller_server.desktop" \
              "${DESKTOP_DIR}/taller_editor.desktop"
    fi

    # Desinstalar dependencias instaladas por este script (si se registraron),
    # salvo que se pida conservarlas (--keep-deps).
    if [[ $KEEP_DEPS -eq 0 && -f "${PREFIX}/installed-packages.txt" ]]; then
        info "Desinstalando dependencias instaladas por el script..."
        to_remove=""
        while IFS= read -r pkg; do
            [[ -z "$pkg" ]] && continue
            if dpkg-query -W -f='${Status}' "$pkg" 2>/dev/null | grep -q "install ok installed"; then
                to_remove="$to_remove $pkg"
            fi
        done < "${PREFIX}/installed-packages.txt"
        if [[ -n "$to_remove" ]]; then
            # shellcheck disable=SC2086
            apt-get remove --purge -y $to_remove || true
            apt-get autoremove -y || true
        fi
    elif [[ $KEEP_DEPS -eq 1 ]]; then
        warn "Se conservan las dependencias instaladas (--keep-deps)."
    fi

    rm -rf "$PREFIX"

    ok "Desinstalacion completada."
    exit 0
}

if [[ $UNINSTALL -eq 1 ]]; then
    uninstall
fi

# Clonar el repo desde GitHub si se pidio (--clone / --from-github).
if [[ $CLONE -eq 1 ]]; then
    TMP_SRC="$(mktemp -d /tmp/taller-clone-XXXXXX)"
    trap 'rm -rf "$TMP_SRC"' EXIT
    info "Clonando repositorio desde GitHub..."
    git clone --depth 1 --branch main "$REPO_URL" "$TMP_SRC"
    SRC_DIR="$TMP_SRC"
    ok "Repositorio clonado"
fi

# ---------- Instalacion ----------
info "Instalando ${APP_NAME} en ${PREFIX}..."

# Dependencias instaladas por este script (se guardan para poder desinstalarlas).
DEPS="build-essential cmake git pkg-config \
    qt6-base-dev qt6-base-dev-tools \
    libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev \
    libopus-dev libopusfile-dev libxmp-dev libfluidsynth-dev fluidsynth \
    libwavpack1 libwavpack-dev wavpack \
    libgl1-mesa-dev libfreetype-dev"

# Paso 0 - Dependencias
info "Paso 0 - Actualizando repositorios e instalando dependencias..."
export DEBIAN_FRONTEND=noninteractive
apt-get update -y
# ca-certificates se instala aparte: no se desinstala automaticamente.
apt-get install -y --no-install-recommends ca-certificates
apt-get install -y --no-install-recommends $DEPS
ok "Paso 0 - Completado"

# Limpieza de instalacion previa
info "Eliminando instalacion previa de ${APP_NAME} (si existe)..."
rm -f "${BIN_DIR}/taller_client" "${BIN_DIR}/taller_server" \
      "${BIN_DIR}/taller_editor" "${BIN_DIR}/taller_uninstall"
rm -rf "$PREFIX"
ok "Archivos previos eliminados"

# Crear directorio de instalacion
mkdir -p "$PREFIX"
# Guardar la lista de dependencias para el desinstalador.
printf '%s\n' "$DEPS" > "${PREFIX}/installed-packages.txt"

# Paso 1..4 - (dependencias ya instaladas en el Paso 0)
info "Paso 1 - CMake listo."
info "Paso 2 - SDL2pp se descarga via CMake (FetchContent)."
info "Paso 3 - Qt6 instalado."
info "Paso 4 - Resto de dependencias instaladas."

# Paso 5 - Compilar el juego
info "Paso 5 - Compilando el juego (puede tardar varios minutos)..."
cmake -S "$SRC_DIR" -B "$PREFIX" \
    -DCMAKE_BUILD_TYPE=Release \
    -DTALLER_TESTS=OFF \
    -DTALLER_TILE_INSPECTOR=OFF \
    -DTALLER_QT_HARNESS=OFF \
    -DTALLER_INSTALL_SCENARIOS_DIR="${PREFIX}/editor/scenarios" \
    -DTALLER_INSTALL_WORLDS_DIR="${PREFIX}/worlds" \
    -DTALLER_INSTALL_PREFABS_TOML="${PREFIX}/editor/prefabs.toml" \
    -DTALLER_INSTALL_ASSETS_DIR="${PREFIX}/assets"
cmake --build "$PREFIX" --parallel "$(nproc)"
ok "Paso 5 - Completado"

# Paso 6 - Preparar carpetas de datos
info "Paso 6 - Moviendo archivos a las carpetas correctas..."
mkdir -p "${PREFIX}/editor" "${PREFIX}/worlds"
if [[ -d "${SRC_DIR}/editor/scenarios" ]]; then
    cp -r "${SRC_DIR}/editor/scenarios" "${PREFIX}/editor/"
fi
if [[ -f "${SRC_DIR}/editor/prefabs.toml" ]]; then
    cp "${SRC_DIR}/editor/prefabs.toml" "${PREFIX}/editor/"
fi
if [[ -d "${SRC_DIR}/worlds" ]]; then
    cp -r "${SRC_DIR}/worlds/." "${PREFIX}/worlds/" 2>/dev/null || true
fi

# Ejecutables en /usr/local/bin. El servidor requiere el servicio (puerto).
ln -sf "${PREFIX}/taller_client"  "${BIN_DIR}/taller_client"
cat > "${BIN_DIR}/taller_server" <<EOF
#!/usr/bin/env bash
exec "${PREFIX}/taller_server" 8080 "\$@"
EOF
chmod +x "${BIN_DIR}/taller_server"
ln -sf "${PREFIX}/taller_editor"  "${BIN_DIR}/taller_editor"
ln -sf "${PREFIX}/uninstall.sh"  "${BIN_DIR}/taller_uninstall"
ok "Paso 6 - Completado"

# Paso 7 - Accesos directos en el Desktop
info "Paso 7 - Creando accesos directos en ${DESKTOP_DIR}..."
mkdir -p "$DESKTOP_DIR"

# Copiar iconos si existen en el repo.
for icon_src in "${SRC_DIR}/assets"/taller_*_icon.png; do
    [[ -f "$icon_src" ]] && cp "$icon_src" "$PREFIX/"
done

make_desktop() {
    local name="$1"
    local bin="$2"
    local label="$3"
    local args="${4:-}"
    local terminal="${5:-false}"
    local icon="${6:-applications-games}"
    local file="${DESKTOP_DIR}/${name}.desktop"
    # Si hay icono copiado, usarlo.
    if [[ -f "${PREFIX}/${name}_icon.png" ]]; then
        icon="${PREFIX}/${name}_icon.png"
    fi
    cat > "$file" <<EOF
[Desktop Entry]
Type=Application
Name=${label}
Exec=${PREFIX}/${bin} ${args}
Path=${PREFIX}
Terminal=${terminal}
Icon=${icon}
Categories=Game;
EOF
    chmod +x "$file"
    chown "${TARGET_USER}:${TARGET_USER}" "$file" 2>/dev/null || true
}

make_desktop "taller_client" "taller_client" "Taller Client"
make_desktop "taller_server" "taller_server" "Taller Server" "8080" "true"
make_desktop "taller_editor" "taller_editor" "Taller Editor"
ok "Paso 7 - Completado"

# Copiar este script como desinstalador
cp "${SRC_DIR}/installer.sh" "${PREFIX}/uninstall.sh"
chmod +x "${PREFIX}/uninstall.sh"

# Paso 8 - Limpieza de dependencias de compilacion (opcional, se conserva el repo fuente)
info "Paso 8 - Instalacion finalizada. El codigo fuente se conserva."
ok "Paso 8 - Completado"

echo ""
ok "El juego fue instalado correctamente en ${PREFIX}."
echo -e "${GREEN}Ejecutables: taller_client, taller_server, taller_editor${NC}"
echo -e "${GREEN}Desinstalar con: ${0} --uninstall${NC}"