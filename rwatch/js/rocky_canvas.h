#pragma once
/* rockyjs.h
 *
 * Rocky.js (On-watch JavaScript) Implementation
 * Rocky/JavaScript Canvas Library
 * 
 * RebbleOS
 *
 * Author: Davit Markarian <davit@udxs.me>
 */

#include "rebbleos.h"
#include "node_list.h"

typedef uint8_t RockyCanvasFontSize;

typedef enum
{
	RockyCanvasFontVariant_Normal,
	RockyCanvasFontVariant_Light,
	RockyCanvasFontVariant_Bold,
	RockyCanvasFontVariant_Bolder
} RockyCanvasFontWeight;

typedef enum
{
	RockyCanvasFontVariant_Default,
	RockyCanvasFontVariant_Numbers
} RockyCanvasFontVariant;

typedef enum
{
	RockyCanvasFontFamily_Gothic,
	RockyCanvasFontFamily_Bitham,
	RockyCanvasFontFamily_Bitham_numeric,
	RockyCanvasFontFamily_Roboto,
	RockyCanvasFontFamily_Roboto_subset,
	RockyCanvasFontFamily_Droid_serif,
	RockyCanvasFontFamily_Leco_numbers,
	RockyCanvasFontFamily_Leco_numbers_am_pm,
} RockyCanvasFontFamily;

// <Size>px [Weight] [Variant] <Family>
typedef struct
{
	RockyCanvasFontSize size;
	RockyCanvasFontVariant weight;
	RockyCanvasFontVariant variant;
	RockyCanvasFontFamily family;
} RockyCanvasFont;

bool rocky_font_parse(char *fontStr, RockyCanvasFont *outFont);

typedef enum
{
	RockyCanvasPathPartType_Arc
} RockyCanvasPathPartType;

typedef struct
{
	RockyCanvasPathPartType type;
	int32_t x;
	int32_t y;
	union {
		struct
		{ // Arc
			int32_t radius;
			float startAngle;
			float endAngle;
		};
		struct
		{ // Rectangle
			int32_t width;
			int32_t height;
		};
		struct // Point (moveTo/lineTo)
		{
		};
	};
} RockyCanvasPathPart;

typedef struct
{
	int32_t currentX;
	int32_t currentY;
	list_head parts;
} RockyCanvasPath;

typedef struct
{
	RockyCanvasPath path;
	list_node node;
} RockyCanvasState;

typedef struct
{
	RockyCanvasState state;
} RockyCanvas;