#include "frameworks/number.h"
#include "frameworks/list.h"
#include "frameworks/screen.h"
//#include "lcd_img.h"
//#include "lcd_map.h"


///// util functions

static inline void set(int16_t* upper, int16_t* lowwer, int16_t x, int16_t y) {
  if (upper[x] < y) {
    upper[x] = y;
  }
  if (lowwer[x] > y) {
    lowwer[x] = y;
  }
}

static inline void swap(int16_t* a, int16_t* b) {
  int16_t t = *a;
  *a = *b;
  *b = t;
}

void screen_fetchBresenhamLine(int16_t* upper, int16_t* lowwer, int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  uint8_t steep = 0;
  if (abs(x0-x1)<abs(y0-y1)) {
    swap(&x0, &y0);
    swap(&x1, &y1);
    steep = 1;
  }
  if (x0>x1) {
    swap(&x0, &x1);
    swap(&y0, &y1);
  }
  int16_t dx = x1-x0;
  int16_t dy = y1-y0;
  int16_t derror2 = abs(dy)<<1;
  int16_t error2 = 0;
  int16_t y = y0;
  for (int16_t x=x0; x<=x1; x++) {
    if (steep) {
      set(upper, lowwer, y, x);
    } else {
      set(upper, lowwer, x, y);
    }
    error2 += derror2;
    if (error2 > dx) {
      y += (y1 > y0 ? 1 : -1);
      error2 -= dx<<1;
    }
  }
}

static inline void vector(int16_t* dx, int16_t* dy, int16_t l) {
  int16_t ax = abs(*dx), ay = abs(*dy);
  int16_t al;
  if (ax>2*ay) {
    al = ax<<2;
  } else if (ax*2>ay) {
    al = (max(ax,ay)*5);
  } else {
    al = ay<<2;
  }
  // *dx = (*dx*l + (al>>1))/al;
  // *dy = (*dy*l + (al>>1))/al;
  *dx = (((*dx*l)<<2) + ((*dx>0)?(al>>1):-(al>>1)))/al;
  *dy = (((*dy*l)<<2) + ((*dy>0)?(al>>1):-(al>>1)))/al;
}

void screen_fetchThickLine(int16_t* upper, int16_t* lowwer, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t width) {
  if (width <= 1) {
    screen_fetchBresenhamLine(upper, lowwer, x0, y0, x1, y1);
  } else {
    int16_t dx = x1-x0, dy = y1-y0;
    vector(&dx,&dy,width-1);
    int16_t dxu = dy/2,dyu = -dx/2;
    int16_t dxl = - dy + dxu, dyl = dx + dyu;
    // int16_t dxu = 1, dyu = 1;
    // int16_t dxl = 0, dyl = 0;
    screen_fetchBresenhamLine(upper, lowwer, x0 + dxu, y0 + dyu, x1 + dxu, y1 + dyu);
    screen_fetchBresenhamLine(upper, lowwer, x0 + dxl, y0 + dyl, x1 + dxl, y1 + dyl);
    if (width >= 4) {
      int16_t dx2 = -dyu, dy2 = dxu;
      screen_fetchBresenhamLine(upper, lowwer, x0 + dxu, y0 + dyu, x0 - dx2, y0 - dy2);
      screen_fetchBresenhamLine(upper, lowwer, x0 - dx2, y0 - dy2, x0 + dxl, y0 + dyl);
      screen_fetchBresenhamLine(upper, lowwer, x1 + dxu, y1 + dyu, x1 + dx2, y1 + dy2);
      screen_fetchBresenhamLine(upper, lowwer, x1 + dx2, y1 + dy2, x1 + dxl, y1 + dyl);
    } else {
      screen_fetchBresenhamLine(upper, lowwer, x0 + dxu, y0 + dyu, x0 + dxl, y0 + dyl);
      screen_fetchBresenhamLine(upper, lowwer, x1 + dxu, y1 + dyu, x1 + dxl, y1 + dyl);
    }
  }
}

void screen_fetchColumn(int16_t* upper, int16_t* lowwer, int16_t x, int16_t y, int16_t width, int16_t base) {
  for (int16_t i = 0; i < width; i++) {
    set(upper, lowwer, x+i, max(base,y));
    set(upper, lowwer, x+i, min(base,y));
  }
}

static uint8_t meshAt(const screen_mesh_t* mesh, int16_t x, int16_t y) {
  const uint8_t* buffer = mesh->buffer;
  if (buffer == SCREEN_MESH_NULL) {
    return 0;
  } else if (buffer == SCREEN_MESH_FILL) {
    return 1;
  } else {
    int16_t index = ((mesh->w+7)>>3)*y+(x>>3);
    int16_t bit = 0x80 >> (x&0x07);
    return (buffer[index]&bit)!=0;
  }
}

static uint16_t drawMesh(int16_t x, int16_t y, const screen_mesh_t* mesh, uint16_t frontColor, uint16_t backColor) {
	int16_t w = mesh->w, h = mesh->h;
  if (mesh->buffer == SCREEN_MESH_NULL) {
    screen_fill(x, y, w, h, backColor);
  } else if (mesh->buffer == SCREEN_MESH_FILL) {
    screen_fill(x, y, w, h, frontColor);
  } else {
    screen_draw(x, y, w, h, frontColor, backColor, mesh->buffer);
  }
  return w;
}
//static uint16_t drawMesh4D(int16_t x, int16_t y, const screen_mesh_t* mesh, uint16_t frontColor, uint16_t backColor, uint16_t backColor2,) {
//	int16_t w = mesh->w, h = mesh->h;
//	screen_draw4DH(x, y, w,h, frontColor, backColor ,backColor2, boundryH,mesh->buffer);
//  return w;
//}
int16_t screen_fetchSequenceWidth(screen_sequence_t* sequence) {
  int16_t totalWidth = 0;
  const screen_mesh_t* const* meshs = sequence-> meshs;
  int8_t* xoffsets = sequence-> xoffsets;
  //int8_t* yoffsets = sequence-> yoffsets;
  for (uint8_t i = 0; i < sequence->size; i++) {
    if (xoffsets) {
      totalWidth += *(xoffsets+i);
    }
    totalWidth += (*(meshs+i))->w;
  }
  return totalWidth;
}

void screen_drawSequence(screen_sequence_t* sequence) {
  int16_t totalWidth = 0;
  const screen_mesh_t* const* meshs = sequence-> meshs;
  int8_t* xoffsets = sequence-> xoffsets;
  int8_t* yoffsets = sequence-> yoffsets;
  totalWidth = screen_fetchSequenceWidth(sequence);
  int16_t x;
  uint8_t align = sequence-> align;
  if ((align & 0x03) == 0x01) {
    x = sequence->rect->x + sequence->rect->w - totalWidth;
  } else if ((align & 0x03) == 0x02) {
    x = sequence->rect->x;
  } else {
    x = sequence->rect->x + (sequence->rect->w >> 1) - (totalWidth >> 1);
  }
  if (sequence->rect->x<x) {
    screen_fill(sequence->rect->x,sequence->rect->y,x - sequence->rect->x, sequence->rect->h, sequence->backColor);
  }
  for (uint8_t i = 0; i < sequence->size; i++) {
    int16_t t = sequence->rect->y, b = sequence->rect->y + sequence->rect->h;
    if (xoffsets && *(xoffsets+i) > 0) {
      screen_fill(x,t,*(xoffsets+i),sequence->rect->h,sequence->backColor);
			x += *(xoffsets+i);
    }
    int16_t y;
    if((align & 0x30) == 0x10) {
      y = sequence->rect->y + sequence->rect->h - (*(meshs+i))->h;
    } else if ((align & 0x30) == 0x20) {
      y = sequence->rect->y;
    } else {
      y = sequence->rect->y + (sequence->rect->h >> 1) - ((*(meshs+i))->h >> 1);
    }
    if (yoffsets) {
      y += *(yoffsets+i);
    }
    const screen_mesh_t* mesh = *(meshs+i);
    if (t < y) {
      screen_fill(x,t,mesh->w,y-t, sequence->backColor);
    }
    drawMesh(x, y, mesh, sequence->frontColor, sequence->backColor);
    if (y+mesh->h < b) {
      screen_fill(x, y+mesh->h, mesh->w, b-(y+mesh->h), sequence->backColor);
    }
    x += mesh->w;
  }
  if (x < sequence->rect->x + sequence->rect->w) {
    int16_t right = sequence->rect->x + sequence->rect->w;
    screen_fill(x, sequence->rect->y, right - x, sequence->rect->h, sequence->backColor);
  }
}

static void fill2bg(x,y,w,h,yb,b1,b2) {
	if (yb <= y) {
		screen_fill(x,y,w,h,b2);
	} else if (yb < y+h) {
		screen_fill(x,y,w,yb - y,b1);
		screen_fill(x,yb,w,y+h-yb,b2);
	} else {
		screen_fill(x,y,w,h,b1);
	}
}

void screen_drawSequenceDynamicBackground(screen_sequence_t* sequence,uint16_t backColor2,uint16_t boundryH) {
  int16_t totalWidth = 0;
  const screen_mesh_t* const* meshs = sequence-> meshs;
  int8_t* xoffsets = sequence-> xoffsets;
  int8_t* yoffsets = sequence-> yoffsets;
  totalWidth = screen_fetchSequenceWidth(sequence);
  int16_t x;
  uint8_t align = sequence-> align;
	uint16_t backColor = sequence->backColor;
  if ((align & 0x03) == 0x01) {
    x = sequence->rect->x + sequence->rect->w - totalWidth;
  } else if ((align & 0x03) == 0x02) {
    x = sequence->rect->x;
  } else {
    x = sequence->rect->x + (sequence->rect->w >> 1) - (totalWidth >> 1);
  }
  if (sequence->rect->x<x) {
    fill2bg(sequence->rect->x,sequence->rect->y,x - sequence->rect->x, sequence->rect->h,boundryH,sequence->backColor,backColor2);
  }
  for (uint8_t i = 0; i < sequence->size; i++) {
    int16_t t = sequence->rect->y, b = sequence->rect->y + sequence->rect->h;
		int16_t h = sequence->rect->h;
    if (xoffsets && *(xoffsets+i) > 0) {
      fill2bg(x,t,*(xoffsets+i),sequence->rect->h,boundryH,sequence->backColor,backColor2);
			x += *(xoffsets+i);
    }
    int16_t y;
    if((align & 0x30) == 0x10) {
      y = sequence->rect->y + sequence->rect->h - (*(meshs+i))->h;
    } else if ((align & 0x30) == 0x20) {
      y = sequence->rect->y;
    } else {
      y = sequence->rect->y + (sequence->rect->h >> 1) - ((*(meshs+i))->h >> 1);
    }
    if (yoffsets) {
      y += *(yoffsets+i);
    }
    const screen_mesh_t* mesh = *(meshs+i);
    if (t < y) {
      fill2bg(x,t,mesh->w,y-t,boundryH,sequence->backColor,backColor2);
    }
		screen_draw4DH(x, y, mesh->w,mesh->h, sequence->frontColor, sequence->backColor ,backColor2, boundryH,mesh->buffer);
   // drawMesh(x, y, mesh, sequence->frontColor, sequence->backColor);
    if (y+mesh->h < b) {
      fill2bg(x, y+mesh->h, mesh->w, b-(y+mesh->h),boundryH,sequence->backColor,backColor2);
    }
    x += mesh->w;
  }
  if (x < sequence->rect->x + sequence->rect->w) {
    int16_t right = sequence->rect->x + sequence->rect->w;
    fill2bg(x, sequence->rect->y, right - x, sequence->rect->h,boundryH,sequence->backColor,backColor2);
  }
}
//void screen_drawDynamicSequence(screen_sequence_t* sequence,  screen_xy2cFunction_t fcn) {
//  int16_t totalWidth = 0;
//  const screen_mesh_t* const* meshs = sequence-> meshs;
//  int8_t* xoffsets = sequence-> xoffsets;
//  int8_t* yoffsets = sequence-> yoffsets;
//  totalWidth = screen_fetchSequenceWidth(sequence);
//  int16_t x;
//  uint8_t align = sequence-> align;
//  if ((align & 0x03) == 0x01) {
//    x = sequence->rect->x + sequence->rect->w - totalWidth;
//  } else if ((align & 0x03) == 0x02) {
//    x = sequence->rect->x;
//  } else {
//    x = sequence->rect->x + (sequence->rect->w >> 1) - (totalWidth >> 1);
//  }
//  if (sequence->rect->x<x) {
//    screen_drawD(sequence->rect->x,sequence->rect->y,x - sequence->rect->x, sequence->rect->h, fcn);
//  }
//  for (uint8_t i = 0; i < sequence->size; i++) {
//    int16_t t = sequence->rect->y, b = sequence->rect->y + sequence->rect->h;
//    if (xoffsets && *(xoffsets+i) > 0) {
//      screen_drawD(x,t,*(xoffsets+i),sequence->rect->h,fcn);
//			x += *(xoffsets+i);
//    }
//    int16_t y;
//    if((align & 0x30) == 0x10) {
//      y = sequence->rect->y + sequence->rect->h - (*(meshs+i))->h;
//    } else if ((align & 0x30) == 0x20) {
//      y = sequence->rect->y;
//    } else {
//      y = sequence->rect->y + (sequence->rect->h >> 1) - ((*(meshs+i))->h >> 1);
//    }
//    if (yoffsets) {
//      y += *(yoffsets+i);
//    }
//    const screen_mesh_t* mesh = *(meshs+i);
//    if (t < y) {
//      screen_drawD(x,t,mesh->w,y-t, fcn);//clear by backColor before real mess start
//    }
//   // drawMesh(x, y, mesh, sequence->frontColor, sequence->backColor);
//		screen_drawD(x, y+mesh->h, mesh->w, b-(y+mesh->h), fcn);  //draw real mess with fc bc
//    if (y+mesh->h < b) {
//      screen_drawD(x, y+mesh->h, mesh->w, b-(y+mesh->h), fcn);
//    }
//    x += mesh->w;
//  }
//  if (x < sequence->rect->x + sequence->rect->w) {
//    int16_t right = sequence->rect->x + sequence->rect->w;
//    screen_drawD(x, sequence->rect->y, right - x, sequence->rect->h, fcn);//clear by backColor after real mess end
//  }
//}

static uint16_t fetchDecBytes(uint32_t value, uint8_t* buffer) {
  int8_t length;
  length = 0;
  while (value != 0) {
    int32_t d = value/10;
    buffer[length] = value - d*10;
    length++;
    value = d;
  }
  return length;
}

uint16_t screen_fetchDec(screen_dec_t* dec, const screen_mesh_t** outMeshs) { //TODO use number
  char buffer[20];
  // int16_t number_toDecString(int32_t value, int16_t dotPosition, char* buffer, int16_t bufferSize);
  int16_t size = number_toDecString(dec->data, dec->dotPosition, buffer, sizeof(buffer));
  int16_t mi = 0, bi = 0;
  if (buffer[0] == '-') {
    outMeshs[mi++] = dec->meshs[11];
    bi ++;
  }
  if (size - (dec->dotPosition?1:0) < dec->minSize) {
    fors(dec->minSize - size + (dec->dotPosition?1:0)) {
      outMeshs[mi++] = dec->meshs[0];
    }
  }
  int8_t doted = 0;
  while (mi - doted < dec->maxSize && bi < size) {
    const screen_mesh_t* m;
    if (buffer[bi] == '.') {
      doted = 1;
      m = dec->meshs[10];
    } else {
      m = dec->meshs[buffer[bi] - '0'];
    }
    outMeshs[mi++] = m;
    bi++;
  }
  return mi;
}


uint16_t screen_fetchDec2(screen_dec_t* dec, const screen_mesh_t** outMeshs) { //TODO use number
  uint8_t negative = 0;
  int32_t value = dec->data;
  if (value < 0) {
    value = - value;
    negative = 1;
  }
  uint8_t indexs[10] = {0};
  int8_t length;
  int8_t size;
  length = fetchDecBytes(value, indexs);
  size = length;
  if(negative) {
    size += 1;
  }
  size = max(size, dec->minSize);
  size = max(size, dec->dotPosition+1+negative);
  size = min(size, dec->maxSize);
  uint8_t index = max(length,size - negative) - 1;
  uint8_t dotIndex;
  dotIndex = max(max(length, dec->dotPosition+1),dec->minSize) - dec->dotPosition + negative;
  if (dotIndex < size) {
    size += 1;
  }
  const screen_mesh_t** meshs = outMeshs;
  if (meshs) {
    for (int8_t i = 0; i < size; i++) {
      const screen_mesh_t* mesh;
      if (i == 0 && negative) {
        mesh = *(dec->meshs+11);
      } else if (dotIndex == i) {
        mesh = *(dec->meshs+10);
      } else {
        mesh = *(dec->meshs+indexs[index]);
        index--;
      }
      *(meshs + i) = mesh;
    }
  }
	return size;
}

//TODO use void** params
static screen_statubar_t* g_screen_statubar4D = 0;
static int16_t g_screen_XOffset4D = 0;
static int16_t g_screen_YOffset4D = 0;
uint16_t screen_statubarFunction(int16_t x, int16_t y) {
  screen_statubar_t* bar = g_screen_statubar4D;
  x += g_screen_XOffset4D;
  y += g_screen_YOffset4D;
  if (x < 0 || x >= bar->width || y < 0 || y >= bar->height) {
    return bar->backColor;
  } else {
    if (x < bar->borderSize || x >= bar->width - bar->borderSize) {
      return bar->borderColor;
    } else {
      if (y < bar->borderSize || y >= bar->height - bar->borderSize) {
        return bar->borderColor;
      } else {
        int16_t barWidth = bar->width - bar->borderSize * 2;
        if ((x - bar->borderSize) * 1000 <= bar->percent * barWidth) {
          return bar->leftColor;
        } else {
          return bar->rightColor;
        }
      }
    }
  }
}

static void initOffsets(void* p){
  screen_area_t* area = (screen_area_t*) p;
  uint16_t x = 0, y = 0;
  if ((area->align & 0x30) == 0x20) {

  } else if ((area->align & 0x30) == 0x10) {
    y -= area->rect->h - area->height;
  } else {
    y -= (area->rect->h - area->height)/2;
  }
  y += area->yoffset;
  if ((area->align & 0x3) == 0x2) {

  } else if ((area->align & 0x3) == 0x1) {
    x -= area->rect->w - area->width;
  } else {
    x -= (area->rect->w - area->width)/2;
  }
  x -= area->xoffset;
  g_screen_XOffset4D = x;
  g_screen_YOffset4D = y;
}
void screen_drawStatubar(screen_statubar_t* bar) {
  g_screen_statubar4D = bar;
  initOffsets((void*)bar);
  screen_drawD(bar->rect->x, bar->rect->y, bar->rect->w, bar->rect->h,screen_statubarFunction);
}

static screen_colorbar_t* g_screen_colorbar4D = 0;
static int16_t g_screen_colorWidth = 0;
uint16_t screen_colorbarFunction(int16_t x, int16_t y) {
  screen_colorbar_t* bar = g_screen_colorbar4D;
  x += g_screen_XOffset4D;
  y += g_screen_YOffset4D;
  if (x < 0 || x >= bar->width || y < 0 || y >= bar->height) {
    return bar->backColor;
  } else {
    for (int8_t i = 0; i < bar->size; i++) {
      if (x<(((i+1)*g_screen_colorWidth)>>4)){
        return bar->colors[i];
      }
    }
    return bar->backColor;
  }
}
void screen_drawColorbar(screen_colorbar_t* bar) {
  g_screen_colorbar4D = bar;
  initOffsets((void*)bar);
  g_screen_colorWidth = (bar->width << 4)/bar->size;
  screen_drawD(bar->rect->x, bar->rect->y, bar->rect->w, bar->rect->h,screen_colorbarFunction);
}

static screen_slider_t* g_screen_slider4D = 0;
uint16_t screen_sliderFunction(int16_t x, int16_t y) {
  screen_slider_t* slider = g_screen_slider4D;
  x += g_screen_XOffset4D;
  y += g_screen_YOffset4D;
  if (x < 0 || x >= slider->mesh->w || y < 0 || y >= slider->mesh->h) {
    return slider->backColor;
  } else {
    return meshAt(slider->mesh, x, y) ? slider->color : slider->backColor;
  }
}

void screen_drawSlider(screen_slider_t* slider) {
  g_screen_slider4D = slider;
  initOffsets((void*)slider);
  int16_t xoffset = slider->percent*slider->width/1000 - (slider->mesh->w >> 1);
  g_screen_XOffset4D -= xoffset;
  screen_drawD(slider->rect->x, slider->rect->y, slider->rect->w, slider->rect->h,screen_sliderFunction);
}

static void drawOutterBackgroud(const screen_rect_t* rect, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (rect->y < y) {
    screen_fill(rect->x, rect->y, rect->w, y - rect->y, color);
  }
  if (rect->x < x) {
    screen_fill(rect->x, y, x - rect->x, h, color);
  }
  if (rect->x + rect->w > x + w) {
    screen_fill(x + w, y, (rect->x + rect->w) - (x + w), h, color);
  }
  if (rect->y + rect->h > y + h) {
    screen_fill(rect->x, y + h, rect->w, (rect->y + rect->h) - (y + h), color);
  }
}


void screen_drawGrid(screen_grid_t* grid) {
  initOffsets((void*)grid);
  int16_t x0 = grid->rect->x - g_screen_XOffset4D, y0 = grid->rect->y-g_screen_YOffset4D;
  drawOutterBackgroud(grid->rect, x0, y0, grid->width, grid->height, grid->backColor);
  int16_t y = y0;
  for (int8_t i = 0; i < grid->rowCount; i++) {
    if (i == 0) {
      screen_fill(x0, y, grid->width, grid->outterLineWidth, grid->outterLineColor);
      y += grid->outterLineWidth;
    }
    int16_t x = x0;
    int16_t h;
    if (i == grid->rowCount - 1) {
      h = y0 + grid->height - grid->outterLineWidth - y;
    } else {
      h = grid->rowSpaces[i];
    }
    for (int8_t j = 0; j < grid->colCount; j++) {
      if (j==0) {
        screen_fill(x, y, grid->outterLineWidth, h, grid->outterLineColor);
        x += grid->outterLineWidth;
      }
      if (j == grid->colCount - 1) {
        x = x0 + grid->width - grid->outterLineWidth;
        screen_fill(x, y, grid->outterLineWidth, h, grid->outterLineColor);
      } else {
        x += grid->colSpaces[j];
        screen_fill(x, y, grid->innerLineWidth, h, grid->innerLineColor);
        x += grid->innerLineWidth;
      }
    }
    y += h;
    if (i == grid->rowCount - 1) {
      screen_fill(x0, y, grid->width, grid->outterLineWidth, grid->outterLineColor);
    } else {
      screen_fill(x0, y, grid->outterLineWidth, grid->innerLineWidth, grid->outterLineColor);
      screen_fill(x0 + grid->outterLineWidth, y, grid->width - grid->outterLineWidth * 2, grid->innerLineWidth, grid->innerLineColor);
      screen_fill(x0 + grid->width - grid->outterLineWidth, y, grid->outterLineWidth, grid->innerLineWidth, grid->outterLineColor);
      y += grid->innerLineWidth;
    }
  }
}

static screen_figure_t* g_screen_figure4D = 0;
static uint8_t* g_screen_figureGridBuffer4D = 0;
#define SCREEN_FIGURE_GRID_BUFFER_SIZE 64

uint16_t screen_figureFunction(int16_t x, int16_t y) {
  screen_figure_t* figure = g_screen_figure4D;
  // {
  //   return figure->spaceColor;
  // }
  x += g_screen_XOffset4D;
  y += g_screen_YOffset4D;
  if (x >= 0 && x < figure->width && y >= 0 && y < figure->height) {
    for (int8_t i = 0; i < figure->lineCount; i++) {
      screen_lineData_t* line = figure->lines[i];
      if (figure->height - 1 - y <= line->dataUpper[x] && figure->height - 1 - y >= line->dataLowwer[x]) {
        return figure->lineColors[i];
      }
    }
    uint8_t* gridBuffer = g_screen_figureGridBuffer4D;
    if (bit_check(gridBuffer,x) && bit_check(gridBuffer + SCREEN_FIGURE_GRID_BUFFER_SIZE * 2, y)) {
      return figure->gridColor;
    }
    if (bit_check(gridBuffer + SCREEN_FIGURE_GRID_BUFFER_SIZE * 2,x) && bit_check(gridBuffer+SCREEN_FIGURE_GRID_BUFFER_SIZE,y)) {
      return figure->gridColor;
    }
    return figure->spaceColor;
  } else {
		//if (x < 0 || x >= figure->width || y < 0 || y >= figure->height) {
    return figure->backColor;
  }
}

void screen_drawFigure(screen_figure_t * figure) {
  g_screen_figure4D = figure;
  initOffsets((void*)figure);
  uint8_t gridBuffer[SCREEN_FIGURE_GRID_BUFFER_SIZE*3] = {0};
  g_screen_figureGridBuffer4D = gridBuffer;
  if (figure->gridXSpace + figure->gridXWidth > 0) {
    for (int16_t i = 0; i < figure->width; i++) {
      if ((i << 8)%(figure->gridXSpace + figure->gridXWidth) < figure->gridXWidth){
        bit_set(gridBuffer, i);
      }
    }
  }
  if (figure->gridYSpace + figure->gridYWidth > 0) {
    for (int16_t i = 0; i < figure->height; i++) {
      if ((i << 8)%(figure->gridYSpace + figure->gridYWidth) < figure->gridYWidth){
        bit_set(gridBuffer + SCREEN_FIGURE_GRID_BUFFER_SIZE, i);
      }
    }
  }
  if (figure->gridDash + figure->gridDashSpace > 0) {
    for (int16_t i = 0; i < figure->width || i < figure->height; i++) {
      if ((i << 8)%(figure->gridDash + figure->gridDashSpace) < figure->gridDash){
        bit_set(gridBuffer + SCREEN_FIGURE_GRID_BUFFER_SIZE * 2, i);
      }
    }
  }
  screen_drawD(figure->rect->x, figure->rect->y, figure->rect->w, figure->rect->h,screen_figureFunction);
}

void screen_drawMenu(screen_menu_t * menu) {
  initOffsets((void*)menu);
  int16_t x0 = menu->rect->x - g_screen_XOffset4D, y0 = menu->rect->y-g_screen_YOffset4D;

  if (menu->updateBackground) {
    drawOutterBackgroud(menu->rect, x0, y0, menu->width, menu->height, menu->backColor);
  }
  int16_t colCount = (menu->itemCount - 1)/menu->rowCount + 1;
  int16_t itemWidth = (menu->width + menu->itemSpace)/colCount - menu->itemSpace;
  int16_t x = x0;
  for (int8_t i = 0; i < colCount; i++) {
    int16_t y = y0;
    for (int8_t j = 0; j < menu->rowCount; j++) {
      int16_t index = i*menu->rowCount + j;
      if (bit_check(menu->updateBits,index)) {
        screen_rect_t rect = {x,y,itemWidth,menu->itemHeight};
        screen_mesh_t const * meshs[1];
        int8_t xoffsets[1] = {menu->itemXOffset};
        int8_t yoffsets[1] = {menu->itemYOffset};
        if (index < menu->itemCount) {
          meshs[0] = menu->meshs[index];
          screen_sequence_t sequence = {&rect, menu->itemAlign, meshs, xoffsets, yoffsets, 1, menu->meshColors[index],menu->backColors[index]};
          screen_drawSequence(&sequence);
          // screen_fill(rect.x, rect.y, rect.w, rect.h, menu->meshColors[index]);
        } else {
          screen_fill(rect.x, rect.y, rect.w, rect.h, menu->backColor);
        }
      }
      y += menu->itemHeight;
      if (j != menu->rowCount -1) {
        if (menu->updateBackground) {
          screen_fill(x, y, itemWidth, menu->itemSpace, menu->backColor);
        }
        y += menu->itemSpace;
      }
    }
    if (y < y0 + menu->height) {
      if (menu->updateBackground) {
        screen_fill(x, y, itemWidth, y0 + menu->height - y, menu->backColor);
      }
    }
    x += itemWidth;
    if (i != colCount -1) {
      if (menu->updateBackground) {
        screen_fill(x, y0, menu->itemSpace, menu->height, menu->backColor);
      }
      x += menu->itemSpace;
    }
  }
  if (x < x0 + menu->width) {
    if (menu->updateBackground) {
      screen_fill(x, y0, x0 + menu->width - x, menu->height, menu->backColor);
    }
  }
}

static screen_dynamic_t* g_screen_dynamic4D = 0;
// static uint16_t g_screen_dynamicMulti4D = 0;
uint16_t screen_dynamicFunction(int16_t x, int16_t y) {
  screen_dynamic_t* dynamic = g_screen_dynamic4D;
  x += g_screen_XOffset4D;
  y += g_screen_YOffset4D;
  // int16_t xs = x * g_screen_dynamicMulti4D >>8;
  // int16_t ys = y * g_screen_dynamicMulti4D >>8;
  if (x >= 0 && x < dynamic->width && y >= 0 && y < dynamic->height) {
    // return meshAt(dynamic->mesh, xs, ys) ? dynamic->frontColor : dynamic->backColor;
    return dynamic->function(x,y);
  } else {
    return dynamic->backColor;
  }
}

void screen_drawDynamicMesh(screen_dynamic_t * dynamic) {
  // g_screen_dynamicMulti4D = (256 + (dynamic->scale>>1))/ dynamic->scale;
  // screen_area_t area = {dynamic->rect, dynamic->align, 0,0, dynamic->mesh->w*dynamic->scale,dynamic->mesh->h*dynamic->scale};
  initOffsets((void*)dynamic);
	g_screen_dynamic4D = dynamic;
  screen_drawD(dynamic->rect->x, dynamic->rect->y, dynamic->rect->w, dynamic->rect->h,screen_dynamicFunction);
}

const int8_t g_screen_gridDivers[] = {1,2,5,10};

int32_t screen_fetchGridInterval(int32_t minValue, int32_t maxValue, int16_t maxGridCount, int16_t* outGridCount) {
  uint8_t buffer[10];
  uint16_t length = fetchDecBytes(maxValue,buffer);
  if (length <1 ) {
    if (outGridCount) {
      *outGridCount = 1;
    }
    return 1;
  } else if (length < 2) {
    uint8_t value = buffer[0] - 1;
    for (int8_t i = 0; i < sizeof(g_screen_gridDivers); i++) {
      int8_t gridCount = value/g_screen_gridDivers[i] + 1;
      if (gridCount <= maxGridCount) {
        if (outGridCount) {
          *outGridCount = gridCount;
        }
        return g_screen_gridDivers[i];
      }
    }
  } else {
    uint8_t value = buffer[length-1] * 10 + buffer[length-2] - 1;
    int32_t base = 1;
    for (int8_t i = 0; i < length-2; i++) {
      base *= 10;
    }
    for (int8_t i = 0; i < sizeof(g_screen_gridDivers); i++) {
      int8_t gridCount = value/g_screen_gridDivers[i] + 1;
      if (gridCount <= maxGridCount) {
        if (outGridCount) {
          *outGridCount = gridCount;
        }
        return g_screen_gridDivers[i] * base;
      }
    }
    for (int8_t i = 1; i < sizeof(g_screen_gridDivers); i++) {
      int8_t gridCount = value/(g_screen_gridDivers[i]*10) + 1;
      if (gridCount <= maxGridCount) {
        if (outGridCount) {
          *outGridCount = gridCount;
        }
        return g_screen_gridDivers[i] * base * 10;
      }
    }
  }
	return 0;
}

uint8_t screen_meshAt(const screen_mesh_t* mesh, int16_t x, int16_t y) {
  return meshAt(mesh, x, y);
}
