#!/usr/bin/env python3
from PIL import Image
from PIL import ImageDraw


def main():
    # Define a list of 8 colors in RGB111 format
    palette = [
        (1, 1, 1),  # Black
        (0, 0, 255),  # Blue
        (0, 255, 0),  # Green
        (0, 255, 255),  # Cyan
        (255, 0, 0),  # Red
        (255, 0, 255),  # Magenta
        (255, 255, 0),  # Yellow
        (255, 255, 255)  # White
    ]
    ink_palette = [
        (26, 26, 26),  # Black
        (93, 129, 147),  # Blue
        (122, 140, 118),  # Green
        (121, 169, 158),  # Cyan
        (124, 88, 91),  # Red
        (138, 132, 144),  # Magenta
        (164, 152, 129),  # Yellow
        (174, 174, 174)  # White
    ]
    input_file = "icons/ok/icons8-gear-24.jpg"
    # Open the input image and convert it to RGB mode
    input_image = Image.open(input_file).convert('RGBA')
    input_image_with_background = Image.new("RGBA", (input_image.width, input_image.height))
    draw = ImageDraw.Draw(input_image_with_background)
    # draw.rectangle(((0, 0), input_image.size), fill=(255, 255, 255))
    draw.bitmap((0, 0), input_image)
    # input_image_with_background.save('p.png')
    # Resize the image to the desired size
    output_size = (input_image.width, input_image.height)
    output_image = Image.new("RGB", output_size)
    output_ink_image = Image.new("RGB", output_size)
    # Loop through each pixel in the input image and find the closest color in the palette
    iter_pixels(ink_palette, input_image, output_image, output_ink_image, palette)

    # Save the output image
    output = input_file + "_out.png"
    output_ink = input_file + "_out_ink.png"
    output_image.save(output)
    output_ink_image.save(output_ink)


def iter_pixels(ink_palette, input_image, output_image, output_ink_image, palette):
    for x in range(input_image.width):
        for y in range(input_image.height):
            # Get the RGB color of the pixel in the input image
            r, g, b, a = input_image.getpixel((x, y))
            # Find the closest color in the palette
            color_index = min(range(len(palette)),
                              key=lambda i: sum((a - b) ** 2 for a, b in zip(palette[i], (r, g, b))))
            # Set the pixel color in the output image to the closest color in the palette
            output_image.putpixel((x, y), palette[color_index])
            output_ink_image.putpixel((x, y), ink_palette[color_index])


if __name__ == '__main__':
    main()
