/*
 * Без имени.c
 * 
 * Copyright 2012 Voldemar Khramtsov <harestomper@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <string.h>
#include <wand/magick_wand.h>

// gcc -Wall `pkg-config --cflags --libs MagickWand` -o testtext testtext.c 
// The pattern_name MUST have a leading #
/*
void set_tile_pattern (DrawingWand *d_wand, char *pattern_name, char *pattern_file)
{
   MagickWand *t_wand;
   long w,h;

   t_wand = NewMagickWand ();  
   MagickReadImage (t_wand,pattern_file);
   // Read the tile's width and height
   w = MagickGetImageWidth (t_wand);
   h = MagickGetImageHeight (t_wand);
   DrawPushPattern (d_wand, pattern_name+1, 0, 0, w, h);
   DrawComposite (d_wand, SrcOverCompositeOp, 0, 0, 0, 0, t_wand);
   DrawPopPattern (d_wand);
   DrawSetFillPatternURL (d_wand, pattern_name);
}

void test_wand (LPTSTR lpCmdLine)
{
   MagickWand *magick_wand = NULL;
   MagickWand *c_wand = NULL;
   DrawingWand *d_wand = NULL;
   PixelWand *p_wand = NULL;


   MagickWandGenesis ();


// Text effect 1 - shadow effect using MagickShadowImage
   magick_wand = NewMagickWand ();
   d_wand = NewDrawingWand ();
   p_wand = NewPixelWand ();
   PixelSetColor (p_wand, "none");
   // Create a new transparent image
   MagickNewImage (magick_wand, 640, 480, p_wand);

   // Set up a 72 point white font 
   PixelSetColor (p_wand, "white");
   DrawSetFillColor (d_wand, p_wand);
   DrawSetFont (d_wand, "Verdana-Bold-Italic") ;
   DrawSetFontSize (d_wand, 72);
   // Add a black outline to the text
   PixelSetColor (p_wand, "black");
   DrawSetStrokeColor (d_wand, p_wand);
   // Turn antialias on - not sure this makes a difference
   DrawSetTextAntialias (d_wand, MagickTrue);
   // Now draw the text
   DrawAnnotation (d_wand, 200, 140, "Magick");
   // Draw the image on to the magick_wand
   MagickDrawImage (magick_wand, d_wand);

   // Trim the image down to include only the text
   MagickTrimImage (magick_wand, 0);
   // equivalent to the command line +repage
   MagickResetImagePage (magick_wand, "");

   // Make a copy of the text image
   c_wand = CloneMagickWand (magick_wand);

   // Set the background colour to blue for the shadow
   PixelSetColor (p_wand, "blue");
   MagickSetImageBackgroundColor (magick_wand, p_wand);

   // Opacity is a real number indicating (apparently) percentage
   MagickShadowImage (magick_wand, 79, 4, 5, 5);
   
   // Composite the text on top of the shadow
   MagickCompositeImage (magick_wand, c_wand, OverCompositeOp, 5, 5);

   // Create a new image the same size as the text image and put a solid colour
   // as its background
   PixelSetColor (p_wand, "rgb(125,215,255)");

   if (c_wand)
      c_wand = DestroyMagickWand(c_wand);

   c_wand = NewMagickWand ();
   MagickNewImage (c_wand, MagickGetImageWidth (magick_wand), MagickGetImageHeight (magick_wand), p_wand);
   // Now composite the shadowed text over the plain background
   MagickCompositeImage (c_wand, magick_wand, OverCompositeOp, 0, 0);
   // and write the result
   MagickWriteImage (c_wand, "text_shadow.png"); 
*/
   // Clean up
/*
   if (magick_wand)
      magick_wand = DestroyMagickWand (magick_wand);

   if (c_wand)
      c_wand = DestroyMagickWand (c_wand);

   if (d_wand)
      d_wand = DestroyDrawingWand (d_wand);

   if (p_wand)
      p_wand = DestroyPixelWand (p_wand);

// Text effect 2 - patterned text
   magick_wand = NewMagickWand ();
   d_wand = NewDrawingWand ();
   p_wand = NewPixelWand ();

   set_tile_pattern (d_wand, "#check", "pattern:checkerboard");

   PixelSetColor (p_wand, "none");
   // Create a new transparent image
   MagickNewImage (magick_wand, 640, 480, p_wand);

   // Set up a 72 point font 
   DrawSetFont (d_wand, "Verdana-Bold-Italic" ) ;
   DrawSetFontSize (d_wand, 72);
   // Now draw the text
   DrawAnnotation (d_wand, 200, 140, "Magick");
   // Draw the image on to the magick_wand
   MagickDrawImage (magick_wand, d_wand);
   // Trim the image
   MagickTrimImage (magick_wand, 0);
   // Add a white border
   PixelSetColor (p_wand, "white");
   MagickBorderImage (magick_wand, p_wand, 5, 5);
   // and write it
   MagickWriteImage (magick_wand, "text_pattern.png");

   //Clean up
   if (magick_wand)
      magick_wand = DestroyMagickWand (magick_wand);

   if (d_wand)
      d_wand = DestroyDrawingWand (d_wand);
      
   if (p_wand)
      p_wand = DestroyPixelWand (p_wand);

   MagickWandTerminus ();
}
*/
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
/*
  MagickWand *magick_wand;
  DrawingWand *d_wand;
  PixelWand *p_wand;
  
   magick_wand = NewMagickWand ();
   d_wand = NewDrawingWand ();
   p_wand = NewPixelWand ();

   // Create a 320x100 lightblue canvas
   PixelSetColor (p_wand, "lightblue");
   MagickNewImage (magick_wand, 320, 100, p_wand);

   // Set up a 72 point font 
   DrawSetFont (d_wand, "Verdana-Bold-Italic");
   DrawSetFontSize (d_wand, 72);
   // Now draw the text

  const char *text = "Magick";
   DrawAnnotation (d_wand, 25, 65, text);
   // Draw the image on to the magick_wand
   MagickDrawImage (magick_wand, d_wand);

//   MagickDistortImage (magick_wand, ArcDistortion, 1, dargs, MagickFalse);

   // Trim the image
   MagickTrimImage (magick_wand, 0);
   // Add the border
   PixelSetColor (p_wand, "lightblue");
   MagickBorderImage (magick_wand, p_wand, 10, 10);

   // and write it
   MagickWriteImage (magick_wand, "text_arc.png");

   //Clean up
   if (magick_wand)
      magick_wand = DestroyMagickWand (magick_wand);

   if (d_wand)
      d_wand = DestroyDrawingWand (d_wand);

   if (p_wand)
      p_wand = DestroyPixelWand (p_wand);

*/
int main (int argc, char **argv)
{
    MagickWand *result_wand;
    PixelWand *foreground;
    PixelWand *background;
    DrawingWand *draw_wand;
    double *textinfo;

    MagickWandGenesis ();
    foreground = NewPixelWand ();
    background = NewPixelWand ();
    draw_wand = NewDrawingWand ();


    // 'rgba' Не читает нормаьлно
    PixelSetColor (foreground, "#00000077");
    DrawSetFillColor (draw_wand, foreground);

    // Создать Wand любого размера, только чтобы было что подсунуть для DrawingWand
    result_wand = NewMagickWand ();
    PixelSetColor (background, "#00000000");
    MagickNewImage (result_wand, 1, 1, background);

    // Полученное название шрифта с размером нужно разделить на шрифт и размер
    // и имя шрифта привести к такому виду
//    DrawSetFont (draw_wand, "Ubuntu-Bold-Italic");
    DrawSetFont (draw_wand, "Bitstream-Vera-Sans-Mono-Bold-Oblique");
    DrawSetFontSize (draw_wand, 100);
    DrawSetTextAntialias (draw_wand, MagickTrue);

    // Получить размеры текста массивом
    //  0 character width
    //  1 character height
    //  2 ascender
    //  3 descender
    //  4 text width
    //  5 text height
    //  6 maximum horizontal advance
    textinfo = MagickQueryFontMetrics (result_wand, draw_wand, argv [1]);

    // Убить старую маленькую палочку, чтобы создать новую нужного размера
    result_wand = DestroyMagickWand (result_wand);
    result_wand = NewMagickWand ();
    MagickNewImage (result_wand, textinfo [4], textinfo [5], background);

    // Задравать на старом драве нужный текст, разместив его в позиции
    // X = 0, Y = ascender
    DrawAnnotation (draw_wand, 0, textinfo [2], argv [1]);

    // Задравать этот текст на большую палочку
    MagickDrawImage (result_wand, draw_wand);

    // Обрезать лишнее прозрачное
    MagickTrimImage (result_wand, 0);

    // Готово
    MagickWriteImage (result_wand, "text-draw.png");

    draw_wand = DestroyDrawingWand (draw_wand);
    background = DestroyPixelWand (background);
    foreground = DestroyPixelWand (foreground);

        MagickWandTerminus ();

    return 0;
}
