from PIL import Image

# 加载图片
image = Image.open("2.png").convert("L")

# 获取图片尺寸
width, height = image.size

# 创建输出文件
output_file = open("output_array.h", "w")

# 写入数组声明
output_file.write("#ifndef IMAGE_ARRAY\n")
output_file.write("#define IMAGE_ARRAY\n")
output_file.write("const unsigned char image_array[%d][%d] = {\n" % (height, width))

# 遍历图片像素
for y in range(height):
    output_file.write("{")
    for x in range(width):
        # 获取像素值
        pixel = image.getpixel((x, y))
        # 将像素值转换为C语言数组的格式
        output_file.write("0x%x, " % pixel)
    output_file.write("},\n")

# 写入数组结束标记
output_file.write("};\n")
output_file.write("#endif // IMAGE_ARRAY\n")

# 关闭输出文件
output_file.close()