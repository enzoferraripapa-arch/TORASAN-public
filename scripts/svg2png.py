"""SVG to PNG converter using cairosvg.

MSYS2 の libcairo-2.dll を使用。cairocffi.__init__.py にフルパスをパッチ済み。
"""
import os
import cairosvg


def convert_svg_to_png(svg_path, png_path, scale=2.0):
    """SVG ファイルを PNG に変換"""
    cairosvg.svg2png(url=svg_path, write_to=png_path, scale=scale)
    return os.path.getsize(png_path)


def convert_all(svg_dir, png_dir, scale=2.0):
    """ディレクトリ内の全 SVG を PNG に変換"""
    os.makedirs(png_dir, exist_ok=True)
    results = {}
    for f in sorted(os.listdir(svg_dir)):
        if f.endswith('.svg'):
            svg_path = os.path.join(svg_dir, f)
            png_name = f.replace('.excalidraw.svg', '.png').replace('.svg', '.png')
            png_path = os.path.join(png_dir, png_name)
            size = convert_svg_to_png(svg_path, png_path, scale)
            results[png_name] = size
            print(f"  {png_name}: {size:,} bytes")
    return results


if __name__ == "__main__":
    base = os.path.dirname(os.path.dirname(__file__))
    svg_dir = os.path.join(base, "docs", "diagrams", "excalidraw")
    png_dir = os.path.join(base, "docs", "diagrams", "png")
    print("Converting SVG -> PNG...")
    convert_all(svg_dir, png_dir)
    print("Done.")
