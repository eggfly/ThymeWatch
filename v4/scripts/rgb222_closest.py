from PIL import Image  
import numpy as np  

def rgb_to_rgb222(r, g, b):  
    """将RGB颜色转换为RGB222格式"""  
    r = (r >> 6) & 0b11  # 取高2位  
    g = (g >> 6) & 0b11  # 取高2位  
    b = (b >> 6) & 0b11  # 取高2位  
    return (r << 4) | (g << 2) | (b)  # 组合成0b00RRGGBB格式  

def closest_rgb222_color(r, g, b):  
    """找到最接近的RGB222颜色"""  
    # 生成所有可能的RGB222颜色  
    colors = [(r2, g2, b2) for r2 in range(4) for g2 in range(4) for b2 in range(4)]  
    
    # 转换输入颜色为RGB222  
    target_color = rgb_to_rgb222(r, g, b)  
    
    # 找到最接近的颜色  
    closest_color = None  
    min_distance = float('inf')  
    
    for r2, g2, b2 in colors:  
        current_color = rgb_to_rgb222(r2 << 6, g2 << 6, b2 << 6)  
        distance = abs(target_color - current_color)  
        
        if distance < min_distance:  
            min_distance = distance  
            closest_color = (r2, g2, b2)  
    
    return rgb_to_rgb222(closest_color[0] << 6, closest_color[1] << 6, closest_color[2] << 6)  

def convert_image_to_rgb222(image_path):  
    """将PNG图像转换为RGB222格式的C语言数组"""  
    # 打开图像并缩放到240x240  
    img = Image.open(image_path).convert('RGB').resize((240, 240))  
    
    # 创建一个空的数组来存储RGB222数据  
    rgb222_array = np.zeros((240, 240), dtype=np.uint8)  

    # 遍历每个像素并转换颜色  
    for y in range(240):  
        for x in range(240):  
            r, g, b = img.getpixel((x, y))  
            rgb222_array[y, x] = closest_rgb222_color(r, g, b)  # 使用最接近的颜色  

    return rgb222_array  

def save_as_c_array(rgb222_array, output_file):  
    """将RGB222数组保存为C语言数组格式"""  
    with open(output_file, 'w') as f:  
        f.write("uint8_t images[240 * 240] = {\n")  
        for y in range(240):  
            for x in range(240):  
                if x > 0:  
                    f.write(", ")  
                f.write(f"0x{rgb222_array[y, x]:02x}")  
            f.write(",\n")  
        f.write("};\n")  

def save_rgb222_image(rgb222_array, output_image_path):  
    """将RGB222数组保存为PNG图像"""  
    # 创建一个新的图像  
    img = Image.new('RGB', (240, 240))  

    # 将RGB222数据转换为RGB格式并填充图像  
    for y in range(240):  
        for x in range(240):  
            value = rgb222_array[y, x]  
            r = (value >> 6) & 0b11  
            g = (value >> 4) & 0b11  
            b = (value >> 2) & 0b11  
            # 将每个通道扩展到8位  
            img.putpixel((x, y), (r * 64, g * 64, b * 64))  

    # 保存图像  
    img.save(output_image_path)  

if __name__ == "__main__":  
    input_image_path = "mario_round_240.png"  # 输入PNG图像路径  
    output_c_file = "output.c"       # 输出C语言文件路径  
    output_image_file = "output_rgb222.png"  # 输出RGB222图像路径  

    rgb222_array = convert_image_to_rgb222(input_image_path)  
    save_as_c_array(rgb222_array, output_c_file)  
    save_rgb222_image(rgb222_array, output_image_file)  

    print(f"转换完成，C语言数组已保存到 {output_c_file}")  
    print(f"RGB222效果图已保存到 {output_image_file}")
