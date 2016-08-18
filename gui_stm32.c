#include "gui_stm32.h"

extern LTDC_HandleTypeDef hLtdcHandler;

extern void LL_ConvertLineToARGB8888(void *src,
				     void *dst,
				     uint32_t xstride,
				     uint32_t color_mode);

void ctgui_draw_element_label(const CTGUI_Element *e) {
  if (e->label != NULL) {
    CTGUI_SpriteSheet *sprite = e->sprite;
    BSP_LCD_DisplayStringAt(e->x, e->y + sprite->sprite_height + 4,
			    (uint8_t *)e->label, LEFT_MODE);
  }
}

static uint8_t colormode_strides[] = {4, 3, 2, 2, 2};

void ctgui_draw_sprite(const CTGUI_SpriteSheet *sprite,
		       uint16_t x,
		       uint16_t y,
		       uint8_t id) {
  uint32_t lcdWidth = BSP_LCD_GetXSize();
  uint32_t address  = hLtdcHandler.LayerCfg[LTDC_ACTIVE_LAYER].FBStartAdress +
		     (((lcdWidth * y) + x) << 2);
  uint16_t width  = sprite->sprite_width;
  uint16_t height = sprite->sprite_height;
  uint8_t *pixels = (uint8_t *)sprite->pixels;
  uint32_t stride = colormode_strides[sprite->format];
  pixels += (id * width * height) * stride;
  stride *= width;
  lcdWidth <<= 2;
  while (--height) {
    LL_ConvertLineToARGB8888(pixels, (uint32_t *)address, width,
			     sprite->format);
    address += lcdWidth;
    pixels += stride;
  }
}

void ctgui_draw_raw(uint16_t x,
		    uint16_t y,
		    uint16_t width,
		    uint16_t height,
		    uint8_t *pixels,
		    uint32_t color_mode) {
  uint32_t lcdWidth = BSP_LCD_GetXSize();
  uint32_t address  = hLtdcHandler.LayerCfg[LTDC_ACTIVE_LAYER].FBStartAdress +
		     (((lcdWidth * y) + x) << 2);
  uint16_t stride = width * colormode_strides[color_mode];
  lcdWidth <<= 2;
  while (--height) {
    LL_ConvertLineToARGB8888(pixels, (uint32_t *)address, width, color_mode);
    address += lcdWidth;
    pixels += stride;
  }
}

void ctgui_draw(CTGUI *gui) {
  BSP_LCD_SetFont((sFONT *)gui->font);
  BSP_LCD_SetBackColor(gui->col_bg);
  BSP_LCD_SetTextColor(gui->col_text);
  for (uint32_t i = 0; i < gui->num_items; i++) {
    CTGUI_Element *e = &gui->items[i];
    if (e->state & CTGUI_DIRTY) {
      e->render(e);
      e->state &= ~((uint16_t)CTGUI_DIRTY);
    }
  }
  gui->redraw = 0;
}

void ctgui_update_touch(TS_StateTypeDef *raw, CTGUI_TouchState *touch) {
  touch->touch_detected = raw->touch_detected;
  touch->touchx[0]      = raw->touchx[0];
  touch->touchy[0]      = raw->touchy[0];
}
