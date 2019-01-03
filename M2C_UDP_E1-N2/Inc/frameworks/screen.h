#ifndef SCREEN_H__
#define SCREEN_H__
#include "stdint.h"
#include "frameworks/list.h"
//
// typedef struct screen_point_t {
//   uint16_t x;
//   uint16_t y;
// } screen_point_t;

#define SCREEN_MESH_FILL ((const uint8_t*)1)
#define SCREEN_MESH_NULL ((const uint8_t*)0)

enum screen_align_t {
  SCREEN_ALIGN_TOPLEFT = 0x22,
  SCREEN_ALIGN_TOP = 0x20,
  SCREEN_ALIGN_TOPRIGHT = 0x21,
  SCREEN_ALIGN_LEFT = 0x02,
  SCREEN_ALIGN_CENTER = 0x00,
  SCREEN_ALIGN_RIGHT = 0x01,
  SCREEN_ALIGN_BOTTOMLEFT = 0x12,
  SCREEN_ALIGN_BOTTOM = 0x10,
  SCREEN_ALIGN_BOTTOMRIGHT = 0x11,
};

typedef uint16_t (*screen_xy2cFunction_t) (int16_t, int16_t);

typedef struct screen_rect_t {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
} screen_rect_t;

typedef struct screen_mesh_t {
  const uint8_t* buffer;
  int16_t w;
  int16_t h; // 0: fullsize
} screen_mesh_t;

typedef struct screen_meshGroup_t {
  uint8_t** bufferPointer;
  int16_t w;
  int16_t h;
} screen_meshGroup_t;

// align 00TB00LR
// offset syyysxxx
// color rrrrrggggggbbbbb
typedef struct screen_sequence_t {
  const screen_rect_t* rect;
  uint8_t align;
  const screen_mesh_t*const* meshs;
  int8_t* xoffsets; // nullable
  int8_t* yoffsets; // nullable
  uint8_t size;
  uint16_t frontColor;
  uint16_t backColor;
} screen_sequence_t;

typedef struct screen_dec_t {
  int32_t data;
  const screen_mesh_t*const* meshs;
  int8_t dotPosition;
  int8_t minSize;
  int8_t maxSize;
} screen_dec_t;


typedef struct screen_area_t {
  const screen_rect_t* rect;
  uint8_t align;
  int8_t xoffset;
  int8_t yoffset;
  int16_t width;
  int16_t height;
} screen_area_t;

typedef struct screen_statubar_t {
  const screen_rect_t* rect;
  uint8_t align;
  int8_t xoffset;
  int8_t yoffset;
  int16_t width;
  int16_t height;
  int16_t percent;
  int8_t borderSize;
  uint16_t borderColor;
  uint16_t leftColor;
  uint16_t rightColor;
  uint16_t backColor;
} screen_statubar_t;

typedef struct screen_colorbar_t {
  const screen_rect_t* rect;
  uint8_t align;
  int8_t xoffset;
  int8_t yoffset;
  int16_t width;
  int16_t height;
  const uint16_t* colors;
  int8_t size;
  uint16_t backColor;
} screen_colorbar_t;

typedef struct screen_slider_t {
  const screen_rect_t* rect;
  uint8_t align;
  int8_t xoffset;
  int8_t yoffset;
  int16_t width;
  int16_t height;
  int16_t percent;
  const screen_mesh_t* mesh;
  uint16_t color;
  uint16_t backColor;
} screen_slider_t;

typedef struct screen_grid_t {
  const screen_rect_t* rect;
  uint8_t align;
  int8_t xoffset;
  int8_t yoffset;
  int16_t width;
  int16_t height;
  int16_t * rowSpaces;
  int16_t rowCount;
  int16_t * colSpaces;
  int16_t colCount;
  uint16_t outterLineWidth;
  uint16_t innerLineWidth;
  uint16_t outterLineColor;
  uint16_t innerLineColor;
  uint16_t backColor;
} screen_grid_t;


enum screen_lineType {
  SCREEN_LINETPYE_line,
  SCREEN_LINETPYE_column,
  SCREEN_LINETPYE_SIZE,
};

typedef struct screen_lineData_t {
  int16_t* dataUpper;
  int16_t* dataLowwer;
} screen_lineData_t;

typedef struct screen_figure_t {
  const screen_rect_t* rect;
  uint8_t align;
  int8_t xoffset;
  int8_t yoffset;
  int16_t width;
  int16_t height;
  screen_lineData_t** lines;
  int8_t lineCount;
  int16_t gridXWidth;
  int16_t gridXSpace;
  int16_t gridYWidth;
  int16_t gridYSpace;
  int16_t gridDash;
  int16_t gridDashSpace;
  uint16_t* lineColors;
  uint16_t gridColor;
  uint16_t spaceColor;
  uint16_t backColor;
} screen_figure_t;

typedef struct screen_menu_t {
  const screen_rect_t* rect;
  uint8_t align;
  int8_t xoffset;
  int8_t yoffset;
  int16_t width;
  int16_t height;
  int16_t itemCount;
  int16_t itemSpace;
  int16_t itemHeight;
  uint8_t itemAlign;
  int8_t itemXOffset;
  int8_t itemYOffset;
  const screen_mesh_t* const* meshs;
  int16_t rowCount;
  uint16_t* meshColors;
  uint16_t* backColors;
  uint16_t backColor;
  uint8_t* updateBits;
  uint8_t updateBackground;
} screen_menu_t;

typedef struct screen_dynamic_t {
  const screen_rect_t* rect;
  uint8_t align;
  int8_t xoffset;
  int8_t yoffset;
  int16_t width;
  int16_t height;
  screen_xy2cFunction_t function;
  uint16_t backColor;
} screen_dynamic_t;



// static inline uint16_t rgb(uint16_t r, uint16_t g, uint16_t b) {
//   return ((r&0xf8)<<8) | ((g&0xfc)<<3) | (b>>3);
// }

#define rgb(r,g,b) (((r&0xf8)<<8) | ((g&0xfc)<<3) | (b>>3))
#define rgba(r,g,b,a) rgb((r*(a+1))>>8,(g*(a+1))>>8,(b*(a+1))>>8)

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))
#define abs(a) ((a)<0?(-(a)):(a))


static inline int16_t overlay(int16_t v1, int16_t v2, int16_t base) {
  return v1 + v2 - base;
}
static inline void parseRBG(uint16_t color, uint8_t* buffer) {
  buffer[0] = (color&0xf800)>>8;
  buffer[1] = (color&0x07e0)>>3;
  buffer[2] = (color&0x001f)<<3;
}
uint16_t static inline color_mix(uint16_t frontColor, uint16_t backColor, uint8_t alpha) { // color background alpha
  uint8_t frontColorBuffer[3];
  uint8_t backColorBuffer[3];
  uint8_t colorBuffer[3];
  parseRBG(frontColor,frontColorBuffer);
  parseRBG(backColor,backColorBuffer);
  fors(3) {
    colorBuffer[i] = ((((int16_t)frontColorBuffer[i]) - ((int16_t)backColorBuffer[i])) * (int16_t) alpha)/256 + (int16_t)backColorBuffer[i];
  }
  uint16_t color = rgb(colorBuffer[0],colorBuffer[1],colorBuffer[2]);
  return color;
}


static inline uint16_t overlayRGB(uint16_t color1, uint16_t color2, uint16_t backColor) {
  uint8_t buffer[12];
  parseRBG(color1, buffer);
  parseRBG(color2, buffer+3);
  parseRBG(backColor, buffer+6);
  for (uint8_t i = 0; i < 3; i++) {
    int16_t value = overlay(buffer[i], buffer[i+3], buffer[i+6]);
    buffer[9+i] = min(max(0,value),255);
  }
  return rgb(buffer[9],buffer[10],buffer[11]);
}

static inline void bit_set(uint8_t* buffer, int16_t i) {
  int16_t index = i>>3;
  uint8_t mask = 1<<(i&0x07);
  buffer[index] |= mask;
}

static inline uint8_t bit_check(uint8_t* buffer, int16_t i) {
  int16_t index = i>>3;
  uint8_t mask = 1<<(i&0x07);
  return (buffer[index] & mask) != 0;
}

void screen_init(void);

void screen_powerOff(void);
void screen_fill(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void screen_draw(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t frontColor, uint16_t backColor, const uint8_t* buffer);
void screen_drawD(int16_t x, int16_t y, int16_t w, int16_t h, screen_xy2cFunction_t fcn);
void screen_draw4DH(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t frontColor, uint16_t backColor ,uint16_t backColor2, uint16_t boundryH,const uint8_t* buffer);
void screen_drawSequence(screen_sequence_t* sequence);
void screen_drawDec(screen_dec_t* dec);
void screen_drawStatubar(screen_statubar_t* bar);
void screen_drawColorbar(screen_colorbar_t* bar);
void screen_drawSlider(screen_slider_t* bar);
void screen_drawGrid(screen_grid_t* grid);
void screen_drawFigure(screen_figure_t * figure);
void screen_drawMenu(screen_menu_t* menu); //??
void screen_drawDynamicMesh(screen_dynamic_t* dynamic);
uint8_t screen_meshAt(const screen_mesh_t* mesh, int16_t x, int16_t y);

uint16_t screen_fetchDec(screen_dec_t* dec, const screen_mesh_t** outMeshs);
int16_t screen_fetchSequenceWidth(screen_sequence_t* sequence);
void screen_fetchThickLine(int16_t* upper, int16_t* lowwer, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t width);
int32_t screen_fetchGridInterval(int32_t minValue, int32_t maxValue, int16_t maxGridCount, int16_t* outGridCount);
void screen_fetchColumn(int16_t* upper, int16_t* lowwer, int16_t x, int16_t y, int16_t width, int16_t base);
void screen_setDir(uint16_t degree);

void screen_drawSequenceDynamicBackground(screen_sequence_t* sequence, uint16_t backColor2, uint16_t boundryH);
void screen_fill_circle(int16_t x, int16_t y,int16_t r, uint16_t color);

#endif
