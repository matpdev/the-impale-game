#!/usr/bin/env python3
"""
Cria imagens de exemplo para o sistema de anúncios
Requer: pip install pillow
"""

from PIL import Image, ImageDraw, ImageFont
import os

def create_banner_top(output_path):
    """Cria banner superior 300x60"""
    width, height = 300, 60
    img = Image.new('RGBA', (width, height), color=(70, 130, 180, 230))  # SteelBlue
    draw = ImageDraw.Draw(img)
    
    # Borda
    draw.rectangle([0, 0, width-1, height-1], outline=(255, 255, 255, 255), width=2)
    
    # Texto
    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 20)
    except:
        font = ImageFont.load_default()
    
    text = "IMPALE GAME SPONSOR"
    bbox = draw.textbbox((0, 0), text, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    x = (width - text_width) // 2
    y = (height - text_height) // 2
    
    draw.text((x, y), text, fill=(255, 255, 255, 255), font=font)
    
    img.save(output_path)
    print(f"Created: {output_path}")

def create_banner_side(output_path):
    """Cria banner lateral 300x150"""
    width, height = 300, 150
    img = Image.new('RGBA', (width, height), color=(60, 179, 113, 220))  # MediumSeaGreen
    draw = ImageDraw.Draw(img)
    
    # Borda
    draw.rectangle([0, 0, width-1, height-1], outline=(255, 255, 255, 255), width=2)
    
    # Texto principal
    try:
        font_big = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24)
        font_small = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 14)
    except:
        font_big = ImageFont.load_default()
        font_small = ImageFont.load_default()
    
    text1 = "IMPALE"
    bbox = draw.textbbox((0, 0), text1, font=font_big)
    text_width = bbox[2] - bbox[0]
    x = (width - text_width) // 2
    draw.text((x, 30), text1, fill=(255, 255, 255, 255), font=font_big)
    
    text2 = "Physics Game"
    bbox = draw.textbbox((0, 0), text2, font=font_small)
    text_width = bbox[2] - bbox[0]
    x = (width - text_width) // 2
    draw.text((x, 70), text2, fill=(255, 255, 255, 255), font=font_small)
    
    text3 = "Play Now!"
    bbox = draw.textbbox((0, 0), text3, font=font_big)
    text_width = bbox[2] - bbox[0]
    x = (width - text_width) // 2
    draw.text((x, 100), text3, fill=(255, 255, 0, 255), font=font_big)
    
    img.save(output_path)
    print(f"Created: {output_path}")

if __name__ == "__main__":
    # Cria diretório se não existir
    ads_dir = "src/assets/ads"
    os.makedirs(ads_dir, exist_ok=True)
    
    # Cria banners
    create_banner_top(os.path.join(ads_dir, "banner_top.png"))
    create_banner_side(os.path.join(ads_dir, "banner_side.png"))
    
    print("\n✅ Advertisement images created successfully!")
    print(f"📁 Location: {ads_dir}/")
