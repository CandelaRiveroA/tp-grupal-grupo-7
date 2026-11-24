#!/usr/bin/env python3
"""Genera atlas PNG compuestos por categoria y editor/prefabs.toml a partir de JSONs de prefabs."""

import argparse
import colorsys
import json
from dataclasses import dataclass
from pathlib import Path

from PIL import Image

TILE_SIZE = 16
GID_MASK = 0x1FFFFFFF
FLIP_H = 0x80000000
FLIP_V = 0x40000000
FLIP_D = 0x20000000

LAYERS_ORDER = ("Back", "Back2", "Buildings", "Front", "AlwaysFront")
MAX_ATLAS_COLS = 32

CATEGORIES: list[tuple[str, str]] = [
    ("arboles_y_vegetacion", "Arboles y vegetacion"),
    ("caminos", "Caminos"),
    ("cesped_y_flores", "Cesped y flores"),
    ("estructuras_granja", "Estructuras de granja"),
    ("vallas", "Vallas"),
    ("casas_y_comercios", "Casas y comercios"),
    ("mobiliario_urbano", "Mobiliario urbano"),
    ("parque_y_ocio", "Parque y ocio"),
    ("granero", "Granero"),
    ("mazmorra", "Mazmorra"),
]

# Tilesets alojados en subcarpetas de tilesheets/
TILESET_REL_PATHS = {
    "decoraciones_mazmorra": "Mines/decoraciones_mazmorra.png",
}

EXCLUDED_PREFAB_IDS = {"camioneta_estacionada"}

# Rol y celda de puerta (relativa al origen del prefab) para edificios especiales.
BUILDING_ROLES: dict[str, tuple[str, tuple[int, int]]] = {
    "casa_granja_1": ("cabin", (5, 7)),
    "casa_granja_2": ("cabin", (5, 7)),
    "casa_granja_3": ("cabin", (5, 7)),
    "casa_granja_4": ("cabin", (5, 7)),
    "centro_comunitario": ("community_center", (5, 9)),
}

CABIN_VARIANTS: list[tuple[str, str, float, float, float]] = [
    ("casa_granja_2", "Casa de Granja (Azul)", 0.58, 0.90, 1.05),
    ("casa_granja_3", "Casa de Granja (Verde)", 0.33, 0.80, 0.95),
    ("casa_granja_4", "Casa de Granja (Violeta)", 0.77, 0.85, 1.05),
]

DEFAULT_CONTENT_ROOT = Path("/home/fernandezz/Descargas/Content/Content (unpacked)/proyecto")
REPO_ROOT = Path(__file__).resolve().parents[2]


@dataclass
class TilesetSheet:
    firstgid: int
    name: str
    image: Image.Image
    columns: int


@dataclass
class PlacedPrefab:
    id: str
    name: str
    category: str
    category_label: str
    col: int
    row: int
    width: int
    height: int
    collision_grid: list[bool]
    sprite: Image.Image
    role: str | None = None
    door: tuple[int, int] | None = None


def toml_escape(value: str) -> str:
    escaped = (
        value.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\n", "\\n")
        .replace("\r", "\\r")
        .replace("\t", "\\t")
    )
    return f'"{escaped}"'


def load_tilesets(
    firstgids: dict[str, int], tilesheets_dir: Path, cache: dict[str, tuple[Image.Image, int]]
) -> list[TilesetSheet]:
    sheets: list[TilesetSheet] = []
    for name, firstgid in firstgids.items():
        if name not in cache:
            rel = TILESET_REL_PATHS.get(name, f"{name}.png")
            img_path = tilesheets_dir / rel
            if not img_path.is_file():
                raise FileNotFoundError(f"Tilesheet no encontrado: {img_path}")
            img = Image.open(img_path).convert("RGBA")
            cache[name] = (img, img.width // TILE_SIZE)
        img, cols = cache[name]
        sheets.append(TilesetSheet(firstgid=int(firstgid), name=name, image=img, columns=cols))
    sheets.sort(key=lambda s: s.firstgid)
    return sheets


def find_tileset(gid: int, sheets: list[TilesetSheet]) -> TilesetSheet:
    chosen = None
    for sheet in sheets:
        if sheet.firstgid <= gid:
            chosen = sheet
        else:
            break
    if chosen is None:
        raise ValueError(f"GID {gid} fuera de rango para tilesets disponibles")
    return chosen


def extract_tile(raw_gid: int, sheets: list[TilesetSheet]) -> Image.Image | None:
    gid = raw_gid & GID_MASK
    if gid == 0:
        return None

    sheet = find_tileset(gid, sheets)
    local_id = gid - sheet.firstgid
    src_col = local_id % sheet.columns
    src_row = local_id // sheet.columns
    x0 = src_col * TILE_SIZE
    y0 = src_row * TILE_SIZE
    if x0 + TILE_SIZE > sheet.image.width or y0 + TILE_SIZE > sheet.image.height:
        raise ValueError(
            f"GID {gid} (local {local_id}) excede dimensiones de '{sheet.name}'"
        )

    tile = sheet.image.crop((x0, y0, x0 + TILE_SIZE, y0 + TILE_SIZE))
    if raw_gid & FLIP_D:
        tile = tile.transpose(Image.Transpose.TRANSPOSE)
    if raw_gid & FLIP_H:
        tile = tile.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
    if raw_gid & FLIP_V:
        tile = tile.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
    return tile


def patch_prefab(prefab: dict) -> None:
    """Completa recortes de origen detectados en los JSONs de prefabs."""
    pid = prefab["id"]
    layers = prefab["layers"]
    if pid == "casa_gris_alex":
        layers["AlwaysFront"][0] = [0, 0, 2090, 2091, 2092, 2093, 0, 0, 0]
    elif pid == "casa_azul_jodi":
        w = int(prefab["size"]["width"])
        prefab["size"]["height"] = int(prefab["size"]["height"]) + 1
        for lname, grid in layers.items():
            top = [0, 0, 2050, 2051, 0, 0, 0, 0] if lname == "AlwaysFront" else [0] * w
            grid.insert(0, top)
        prefab["collision_grid"].insert(0, [0] * w)


def compose_prefab(prefab: dict, sheets: list[TilesetSheet]) -> Image.Image:
    width = int(prefab["size"]["width"])
    height = int(prefab["size"]["height"])
    canvas = Image.new("RGBA", (width * TILE_SIZE, height * TILE_SIZE), (0, 0, 0, 0))
    layers = prefab["layers"]

    for layer_name in LAYERS_ORDER:
        grid = layers.get(layer_name)
        if not grid:
            continue
        layer_img = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
        for r in range(height):
            for c in range(width):
                tile = extract_tile(int(grid[r][c]), sheets)
                if tile is not None:
                    layer_img.paste(tile, (c * TILE_SIZE, r * TILE_SIZE))
        canvas = Image.alpha_composite(canvas, layer_img)

    return canvas


def recolor_roof(
    src: Image.Image, target_hue: float, sat_scale: float, val_scale: float
) -> Image.Image:
    mask_path = Path(__file__).resolve().with_name("casa_granja_roof_mask.png")
    mask = Image.open(mask_path).convert("RGBA")
    out = src.copy()
    px = out.load()
    px_m = mask.load()
    w, h = out.size
    for y in range(h):
        for x in range(w):
            if px_m[x, y][3] == 0 or px_m[x, y][1] < 128:
                continue
            r, g, b, a = px[x, y]
            if a == 0:
                continue
            hue, sat, val = colorsys.rgb_to_hsv(r / 255.0, g / 255.0, b / 255.0)
            delta = hue if hue <= 0.5 else hue - 1.0
            nh = (target_hue + delta * 0.5) % 1.0
            ns = min(1.0, sat * sat_scale)
            nv = min(1.0, val * val_scale)
            nr, ng, nb = colorsys.hsv_to_rgb(nh, ns, nv)
            px[x, y] = (int(round(nr * 255)), int(round(ng * 255)), int(round(nb * 255)), a)
    return out


def pack_atlas(
    items: list[tuple[dict, Image.Image, list[bool]]],
    category: str,
    category_label: str,
) -> tuple[Image.Image, list[PlacedPrefab]]:
    col = 0
    row = 0
    row_h = 0
    atlas_cols = 0
    placed: list[PlacedPrefab] = []

    for prefab, sprite, collision_grid in items:
        w = int(prefab["size"]["width"])
        h = int(prefab["size"]["height"])
        if col > 0 and col + w > MAX_ATLAS_COLS:
            row += row_h
            col = 0
            row_h = 0

        pid = prefab["id"]
        role, door = BUILDING_ROLES.get(pid, (None, None))
        placed.append(
            PlacedPrefab(
                id=pid,
                name=prefab["nombre"],
                category=category,
                category_label=category_label,
                col=col,
                row=row,
                width=w,
                height=h,
                collision_grid=collision_grid,
                sprite=sprite,
                role=role,
                door=door,
            )
        )
        col += w
        row_h = max(row_h, h)
        atlas_cols = max(atlas_cols, col)

    atlas_rows = row + row_h
    atlas = Image.new("RGBA", (atlas_cols * TILE_SIZE, atlas_rows * TILE_SIZE), (0, 0, 0, 0))
    for item in placed:
        atlas.paste(item.sprite, (item.col * TILE_SIZE, item.row * TILE_SIZE))
    return atlas, placed


def format_prefabs_toml(
    categories: list[tuple[str, str]], placed_all: list[PlacedPrefab]
) -> str:
    lines = ["format_version = 1"]

    for category, _ in categories:
        lines.extend(
            [
                "",
                f"[tilesets.prefab_{category}]",
                f'image = "World/prefabs/{category}.png"',
                f"cell = [{TILE_SIZE}, {TILE_SIZE}]",
            ]
        )

    for item in placed_all:
        lines.extend(
            [
                "",
                "[[prefabs]]",
                f"id = {toml_escape(item.id)}",
                f"name = {toml_escape(item.name)}",
                f"category = {toml_escape(item.category)}",
                f"category_label = {toml_escape(item.category_label)}",
            ]
        )
        if item.role is not None:
            lines.append(f"role = {toml_escape(item.role)}")
        if item.door is not None:
            lines.append(f"door = [{item.door[0]}, {item.door[1]}]")
        lines.extend(
            [
                f'tile = "prefab_{item.category}:{item.col},{item.row}"',
                f"size = [{item.width}, {item.height}]",
                "collision_grid = [",
            ]
        )
        for r in range(item.height):
            start = r * item.width
            row_vals = item.collision_grid[start : start + item.width]
            formatted = ", ".join("true " if v else "false" for v in row_vals).rstrip()
            lines.append(f"  {formatted},")
        lines.append("]")

    lines.append("")
    return "\n".join(lines)


def build_all(
    prefabs_dir: Path, tilesheets_dir: Path, assets_dir: Path, output_toml: Path
) -> dict[str, int]:
    atlas_dir = assets_dir / "World" / "prefabs"
    atlas_dir.mkdir(parents=True, exist_ok=True)

    sheet_cache: dict[str, tuple[Image.Image, int]] = {}
    seen_ids: dict[str, str] = {}
    all_placed: list[PlacedPrefab] = []
    counts: dict[str, int] = {}

    for category, category_label in CATEGORIES:
        json_path = prefabs_dir / f"{category}.json"
        data = json.loads(json_path.read_text(encoding="utf-8"))
        sheets = load_tilesets(data["tileset_firstgids"], tilesheets_dir, sheet_cache)

        items: list[tuple[dict, Image.Image, list[bool]]] = []
        for prefab in data["prefabs"]:
            pid = prefab["id"]
            if pid in EXCLUDED_PREFAB_IDS:
                continue
            patch_prefab(prefab)
            if pid in seen_ids:
                raise ValueError(
                    f"ID duplicado '{pid}' entre '{seen_ids[pid]}' y '{category}'"
                )
            seen_ids[pid] = category

            w = int(prefab["size"]["width"])
            h = int(prefab["size"]["height"])
            raw_grid = prefab["collision_grid"]
            flat_grid = [bool(cell) for row in raw_grid for cell in row]
            if len(flat_grid) != w * h:
                raise ValueError(
                    f"collision_grid invalido en '{pid}': {len(flat_grid)} != {w * h}"
                )

            sprite = compose_prefab(prefab, sheets)
            items.append((prefab, sprite, flat_grid))
            if pid == "casa_granja_1":
                for vid, vname, vhue, vsat, vval in CABIN_VARIANTS:
                    seen_ids[vid] = category
                    vprefab = {**prefab, "id": vid, "nombre": vname}
                    items.append((vprefab, recolor_roof(sprite, vhue, vsat, vval), list(flat_grid)))

        atlas, placed = pack_atlas(items, category, category_label)
        atlas.save(atlas_dir / f"{category}.png")
        all_placed.extend(placed)
        counts[category] = len(placed)

    output_toml.parent.mkdir(parents=True, exist_ok=True)
    output_toml.write_text(
        format_prefabs_toml(CATEGORIES, all_placed), encoding="utf-8"
    )
    return counts


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Genera atlas PNG de prefabs por categoria y editor/prefabs.toml."
    )
    parser.add_argument(
        "--prefabs-dir",
        type=Path,
        default=DEFAULT_CONTENT_ROOT / "prefabs",
        help="Directorio con los JSON de prefabs por categoria",
    )
    parser.add_argument(
        "--tilesheets-dir",
        type=Path,
        default=DEFAULT_CONTENT_ROOT / "tilesheets",
        help="Directorio con los PNG de tilesheets",
    )
    parser.add_argument(
        "--assets-dir",
        type=Path,
        default=REPO_ROOT / "client/sdl/assets",
        help="Raiz de assets del cliente/editor (genera World/prefabs/<cat>.png)",
    )
    parser.add_argument(
        "--output-toml",
        type=Path,
        default=REPO_ROOT / "editor/prefabs.toml",
        help="Archivo TOML de salida",
    )
    args = parser.parse_args()

    counts = build_all(
        args.prefabs_dir, args.tilesheets_dir, args.assets_dir, args.output_toml
    )
    total = sum(counts.values())
    print(f"Generados {len(counts)} atlas PNG y {args.output_toml} ({total} prefabs):")
    for category, count in counts.items():
        print(f"  - {category}: {count}")


if __name__ == "__main__":
    main()
