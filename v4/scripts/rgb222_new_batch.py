# 导入必要的库
from PIL import Image, ImageOps  
import sys

# 定义总帧数 N
N = 7  # 您可以根据需要修改帧数

try:  
    resample_filter = Image.Resampling.LANCZOS  # Pillow 9.1.0 及以上版本  
except AttributeError:  
    resample_filter = Image.LANCZOS  # Pillow 9.0.0 及以下版本  

output_header_path = f'anim_data_{N}_frames.h'

anim_data = []  # 用于存储所有帧的 RGB222 数据

for frame_num in range(1, N + 1):
    input_image_path = f'{frame_num}.jpg'
    preview_image_path = f'preview_{frame_num}.png'
    
    # 打开图像并转换为 RGB 模式
    try:
        img = Image.open(input_image_path).convert('RGB')
    except FileNotFoundError:
        print(f"未找到文件 {input_image_path}")
        continue
    
    # 调整大小到 240x240
    if img.size != (240, 240):
        img = ImageOps.fit(img, (240, 240), method=resample_filter, centering=(0.5, 0.5))  
    
    pixels = img.load()
    
    # 创建当前帧的 RGB222 数据列表
    frame_data = []
    
    for y in range(240):
        row_data = []
        for x in range(240):
            r, g, b = pixels[x, y]
            # 缩小颜色深度到 2 位
            r2 = (r >> 6) & 0x03  # 取最高 2 位
            g2 = (g >> 6) & 0x03
            b2 = (b >> 6) & 0x03
            # 合并为 0b00RRGGBB 格式
            rgb222 = (r2 << 4) | (g2 << 2) | b2
            row_data.append(rgb222)
        frame_data.append(row_data)
    anim_data.append(frame_data)
    
    # 将 RGB222 数据转换回 RGB888，用于预览
    preview_img = Image.new('RGB', (240, 240))
    preview_pixels = preview_img.load()
    
    for y in range(240):
        for x in range(240):
            rgb222 = frame_data[y][x]
            # 提取 R、G、B 两位数据
            r2 = (rgb222 >> 4) & 0x03
            g2 = (rgb222 >> 2) & 0x03
            b2 = rgb222 & 0x03
            # 扩展到 8 位
            r8 = (r2 << 6) | (r2 << 4) | (r2 << 2) | r2
            g8 = (g2 << 6) | (g2 << 4) | (g2 << 2) | g2
            b8 = (b2 << 6) | (b2 << 4) | (b2 << 2) | b2
            preview_pixels[x, y] = (r8, g8, b8)
    
    # 保存预览图像
    preview_img.save(preview_image_path)
    print(f"已处理帧 {frame_num}")

# 生成 C 头文件
with open(output_header_path, 'w') as f:
    f.write('#pragma once\n')
    f.write('#include <stdint.h>\n\n')
    f.write(f'uint8_t anim_data[{N}][240][240] = {{\n')
    for frame_idx, frame_data in enumerate(anim_data):
        f.write('    {\n')
        for y in range(240):
            f.write('        {')
            row_data = frame_data[y]
            f.write(', '.join(f'0x{byte:02X}' for byte in row_data))
            f.write('}')
            if y != 239:
                f.write(',\n')
            else:
                f.write('\n')
        f.write('    }')
        if frame_idx != N - 1:
            f.write(',\n')
        else:
            f.write('\n')
    f.write('};\n')

print(f"已生成 {output_header_path}")
