/*
    Copyright (C) 2011-2026, The AROS Development Team. All rights reserved.
*/

#define MUIMASTER_YES_INLINE_STDARG

#include <proto/alib.h>
#include <proto/muimaster.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/graphics.h>

#include "Lamp_mcc.h"
#include "lamp_private.h"

//#define DEBUG 1
#include <aros/debug.h>

// Values for lmp_PenChanged and lmp_PenChangedOld
#define PCH_NONE        (0)
#define PCH_NEWPEN      (1)
#define PCH_NEWSPEC     (2)

// Lamp sizes from MUIV_Lamp_Type_Tiny to MUIV_Lamp_Type_Huge
const static ULONG lampsizes[] = {5, 6, 7, 8, 9};

/* Legacy sizes are intentionally retained; explicit diameter is additive. */
#define LAMP_MAX_DIAMETER 64
#define LAMP_EFFECT_PENS 4

static ULONG lamp_diameter(const struct Lamp_DATA *data)
{
    return data->lmp_Diameter ? data->lmp_Diameter : lampsizes[data->lmp_Type];
}

static ULONG lamp_darken(ULONG v, ULONG percent)
{
    return percent >= 100 ? v : (v / 100UL) * percent;
}

static ULONG lamp_brighten(ULONG v, ULONG percent)
{
    return v + ((0xffffffffUL - v) / 100UL) * percent;
}

static void lamp_release_pens(Object *obj, struct Lamp_DATA *data)
{
    ULONG i;
    for (i = 0; i < LAMP_EFFECT_PENS; i++)
    {
        if (data->lmp_EffectPen[i] != -1)
        {
            ReleasePen(_screen(obj)->ViewPort.ColorMap, data->lmp_EffectPen[i]);
            data->lmp_EffectPen[i] = -1;
        }
    }
    if (data->lmp_PenNr != -1)
    {
        if (data->lmp_PenChangedOld == PCH_NEWSPEC)
            MUI_ReleasePen(muiRenderInfo(obj), data->lmp_PenNr);
        else if (data->lmp_PenChangedOld == PCH_NEWPEN)
            ReleasePen(_screen(obj)->ViewPort.ColorMap, data->lmp_PenNr);
    }
    data->lmp_PenNr = -1;
    data->lmp_PenChangedOld = PCH_NONE;
}

static LONG lamp_obtain_rgb(Object *obj, ULONG r, ULONG g, ULONG b)
{
    return ObtainBestPen(_screen(obj)->ViewPort.ColorMap, r, g, b,
                         OBP_Precision, PRECISION_GUI, TAG_DONE);
}

static void lamp_acquire_pens(Object *obj, struct Lamp_DATA *data)
{
    ULONG c[3], i;
    LONG pen;
    BOOL unlit = (data->lmp_Intensity == 0 ||
                  (data->lmp_ColorType != MUIV_Lamp_ColorType_PenSpec &&
                   !data->lmp_Color[0] && !data->lmp_Color[1] && !data->lmp_Color[2]));
    lamp_release_pens(obj, data);
    if (data->lmp_ColorType == MUIV_Lamp_ColorType_PenSpec)
    {
        pen = MUI_ObtainPen(muiRenderInfo(obj), &data->lmp_PenSpec, 0);
        if (pen != -1)
        {
            data->lmp_PenNr = pen;
            data->lmp_PenChangedOld = PCH_NEWSPEC;
        }
    }
    else
    {
        for (i = 0; i < 3; ++i)
            c[i] = lamp_darken(data->lmp_Color[i], data->lmp_Intensity);
        pen = lamp_obtain_rgb(obj, c[0], c[1], c[2]);
        if (pen != -1)
        {
            data->lmp_PenNr = pen;
            data->lmp_PenChangedOld = PCH_NEWPEN;
        }
    }

    if (data->lmp_Style == MUIV_Lamp_Style_LED)
    {
        /* Neutral bezel; shaded lens; bright reflection; diffuse upper rim. */
        data->lmp_EffectPen[0] = lamp_obtain_rgb(obj, 0x25252525UL, 0x25252525UL, 0x25252525UL);
        for (i = 0; i < 3; ++i)
            c[i] = lamp_darken(data->lmp_ColorType == MUIV_Lamp_ColorType_PenSpec ?
                               0x55555555UL : data->lmp_Color[i], data->lmp_Intensity / 2UL);
        data->lmp_EffectPen[1] = lamp_obtain_rgb(obj, c[0], c[1], c[2]);
        for (i = 0; i < 3; ++i)
            c[i] = lamp_brighten(data->lmp_ColorType == MUIV_Lamp_ColorType_PenSpec ?
                                 0x99999999UL : data->lmp_Color[i], unlit ? 12UL : 65UL);
        data->lmp_EffectPen[2] = lamp_obtain_rgb(obj, c[0], c[1], c[2]);
        for (i = 0; i < 3; ++i)
            c[i] = lamp_brighten(data->lmp_ColorType == MUIV_Lamp_ColorType_PenSpec ?
                                 0x88888888UL : data->lmp_Color[i], unlit ? 5UL : 20UL);
        data->lmp_EffectPen[3] = lamp_obtain_rgb(obj, c[0], c[1], c[2]);
    }
    data->lmp_PenChanged = data->lmp_PenNr != -1 ? PCH_NONE :
        (data->lmp_ColorType == MUIV_Lamp_ColorType_PenSpec ? PCH_NEWSPEC : PCH_NEWPEN);
}

static LONG lamp_draw_pen(const struct Lamp_DATA *data, unsigned int region)
{
    LONG p = data->lmp_EffectPen[region];
    return p != -1 ? p : -1;
}

static void lamp_draw_led(Object *obj, const struct Lamp_DATA *data, ULONG size)
{
    LONG x, y, r, inner;
    LONG left = _mleft(obj), top = _mtop(obj);
    LONG base = data->lmp_PenNr;
    LONG bezel = lamp_draw_pen(data, 0);
    LONG shade = lamp_draw_pen(data, 1);
    LONG shine = lamp_draw_pen(data, 2);
    LONG glow = lamp_draw_pen(data, 3);
    if (!size) return;
    if (base == -1)
    {
        if (!_pens(obj)) return;
        base = _pens(obj)[MPEN_TEXT];
    }
    else if (data->lmp_PenChangedOld == PCH_NEWSPEC) base = MUIPEN(base);
    if (bezel == -1) bezel = base;
    if (shade == -1) shade = base;
    if (shine == -1) shine = base;
    if (glow == -1) glow = base;
    r = (LONG)size - 1;
    inner = r > 3 ? r - 3 : 0;
    for (y = 0; y < (LONG)size; ++y)
    {
        LONG current = -2;
        LONG runstart = 0;
        LONG dy = 2 * y + 1 - (LONG)size;
        for (x = 0; x <= (LONG)size; ++x)
        {
            LONG next = -2;
            if (x < (LONG)size)
            {
                LONG dx = 2 * x + 1 - (LONG)size;
                if (dx * dx + dy * dy <= r * r)
                {
                    next = base;
                    if (dx * dx + dy * dy > inner * inner) next = bezel;
                    else if (y >= (LONG)(size * 2 / 3)) next = shade;
                    else if (x < (LONG)(size / 2) && y < (LONG)(size / 3)) next = shine;
                    else if (y < (LONG)(size / 3)) next = glow;
                }
            }
            if (next != current)
            {
                if (current >= 0)
                {
                    SetAPen(_rp(obj), current);
                    RectFill(_rp(obj), left + runstart, top + y, left + x - 1, top + y);
                }
                runstart = x;
                current = next;
            }
        }
    }
}

// Lamp default colors
const static ULONG defaultcolors[][3] =
{
    {0x00000000, 0x00000000, 0x00000000}, // MUIV_Lamp_Color_Off
    {0x00000000, 0xffffffff, 0x00000000}, // MUIV_Lamp_Color_Ok
    {0x80808080, 0x00000000, 0x00000000}, // MUIV_Lamp_Color_Warning
    {0xc0c0c0c0, 0x00000000, 0x00000000}, // MUIV_Lamp_Color_Error
    {0xffffffff, 0x00000000, 0x00000000}, // MUIV_Lamp_Color_FatalError
    {0x00000000, 0xc0c0c0c0, 0x00000000}, // MUIV_Lamp_Color_Processing
    {0xc0c0c0c0, 0x00000000, 0x00000000}, // MUIV_Lamp_Color_LookingUp
    {0xffffffff, 0x00000000, 0x00000000}, // MUIV_Lamp_Color_Connecting
    {0xffffffff, 0xffffffff, 0x00000000}, // MUIV_Lamp_Color_SendingData
    {0x00000000, 0x00000000, 0xffffffff}, // MUIV_Lamp_Color_ReceivingData
    {0x00000000, 0x00000000, 0xffffffff}, // MUIV_Lamp_Color_LoadingData
    {0xffffffff, 0xffffffff, 0x00000000}, // MUIV_Lamp_Color_SavingData
};


/****** Lamp.mcc/MUIC_Lamp ************
*
*   NAME
*       MUIC_Lamp
*
*   SUPERCLASS
*       Area.mui
*
*   DESCRIPTION
*       Draw a colored status lamp from a preset, RGB components or a MUI
*       pen specification. Defaults: Type_Medium, Color_Off and Flat
*       rendering. LED rendering, explicit diameter and intensity are
*       opt-in presentation controls.
*
*   NOTES
*       Color and PenSpec inputs are copied. Getter pointers into instance
*       storage are borrowed and valid only while the object remains alive.
*       Pen acquisition failures are not reported by the attribute API. If no
*       pen can be acquired, drawing falls back to the MUI text pen and the
*       requested acquisition remains pending for a later draw.
*
*/
/****** Lamp.mcc/MUIA_Lamp_Type ************
*
*   NAME
*       MUIA_Lamp_Type
*
*   FUNCTION
*       Select the status lamp size.
*
*   INPUTS
*       Initialize/set/get; ULONG. Tiny (0), Small (1), Medium (2, default),
*       Big (3), Huge (4).
*
*   NOTES
*       Values outside 0..4 are ignored. The table sizes (5..9) augment
*       layout bounds. Changing Type while the object is set up requests
*       parent-group relayout when Diameter is zero.
*
*/
/****** Lamp.mcc/MUIA_Lamp_Color ************
*
*   NAME
*       MUIA_Lamp_Color
*
*   FUNCTION
*       Select one of 12 status colors, or copy three ULONG RGB values.
*
*   INPUTS
*       Initialize/set/get; IPTR. Values 0..11 are MUIV_Lamp_Color_* presets;
*       other values are pointers to three readable ULONGs. Default Off (0).
*
*   NOTES
*       The getter returns a preset index in UserDefined mode, a borrowed
*       RGB array pointer in Color mode, or zero in PenSpec mode. Query ColorType
*       before interpreting it. Input RGB arrays are copied immediately; no
*       separate RGB-component or ColorType notifications are generated.
*
*/
/****** Lamp.mcc/MUIA_Lamp_ColorType ************
*
*   NAME
*       MUIA_Lamp_ColorType
*
*   FUNCTION
*       Identify the current color representation.
*
*   INPUTS
*       Get only; ULONG: UserDefined (0), Color/RGB (1), PenSpec (2).
*
*   NOTES
*       Indicates representation, not successful pen acquisition.
*
*/
/****** Lamp.mcc/MUIA_Lamp_Red ************
*
*   NAME
*       MUIA_Lamp_Red
*
*   FUNCTION
*       Set or read the stored red RGB component.
*
*   INPUTS
*       Initialize/set/get; ULONG, default 0.
*
*   NOTES
*       Set selects RGB mode, preserves the other components and redraws.
*       Get returns the saved value, even while PenSpec mode is selected.
*       No separate Color/ColorType notification is generated.
*
*/
/****** Lamp.mcc/MUIA_Lamp_Green ************
*
*   NAME
*       MUIA_Lamp_Green
*
*   FUNCTION
*       Set or read the stored green RGB component.
*
*   INPUTS
*       Initialize/set/get; ULONG, default 0.
*
*   NOTES
*       Set selects RGB mode, preserves the other components and redraws.
*       Get returns the saved value, even while PenSpec mode is selected.
*       No separate Color/ColorType notification is generated.
*
*/
/****** Lamp.mcc/MUIA_Lamp_Blue ************
*
*   NAME
*       MUIA_Lamp_Blue
*
*   FUNCTION
*       Set or read the stored blue RGB component.
*
*   INPUTS
*       Initialize/set/get; ULONG, default 0.
*
*   NOTES
*       Set selects RGB mode, preserves the other components and redraws.
*       Get returns the saved value, even while PenSpec mode is selected.
*       No separate Color/ColorType notification is generated.
*
*/
/****** Lamp.mcc/MUIA_Lamp_PenSpec ************
*
*   NAME
*       MUIA_Lamp_PenSpec
*
*   FUNCTION
*       Set a MUI pen specification.
*
*   INPUTS
*       Initialize/set/get; non-NULL struct MUI_PenSpec *.
*
*   NOTES
*       Input is copied. Get returns a borrowed pointer to instance storage.
*       Initial and later PenSpec values select MUI pen acquisition. Pen
*       resolution is not reported; failed acquisition remains pending.
*       Intensity does not modify the selected PenSpec base pen.
*
*/
/****** Lamp.mcc/MUIA_Lamp_Style ************
*
*   NAME
*       MUIA_Lamp_Style
*
*   FUNCTION
*       Select circular LED rendering or the historical flat rectangle.
*
*   INPUTS
*       Initialize/set/get; ULONG. Flat=0 (default), LED=1.
*
*   NOTES
*       Invalid values are ignored. Style changes request full redraw.
*
*/
/****** Lamp.mcc/MUIA_Lamp_Diameter ************
*
*   NAME
*       MUIA_Lamp_Diameter
*
*   FUNCTION
*       Override the size preset with an explicit square diameter.
*
*   INPUTS
*       Initialize/set/get; ULONG. Zero (default) uses Type; 5..64 specify
*       the exact diameter in pixels. Other values are ignored.
*
*   NOTES
*       A change requests parent layout. Applications using Lamp as a list
*       image may require their list to recalculate its image dimensions.
*
*/
/****** Lamp.mcc/MUIA_Lamp_Intensity ************
*
*   NAME
*       MUIA_Lamp_Intensity
*
*   FUNCTION
*       Scale the displayed RGB brightness independently from stored color.
*
*   INPUTS
*       Initialize/set/get; ULONG 0..100, default 100. Values above 100
*       are ignored. Intensity does not alter PenSpec-defined base pens.
*
*   NOTES
*       Color getters return the unscaled RGB components. A change causes
*       pen reacquisition at the next draw, not a periodic timer wakeup.
*
*/
/****** Lamp.mcc/MUIM_Lamp_SetRGB ************
*
*   NAME
*       MUIM_Lamp_SetRGB
*
*   SYNOPSIS
*       DoMethod(obj, MUIM_Lamp_SetRGB, red, green, blue);
*
*   FUNCTION
*       Set three ULONG RGB components together.
*
*   RESULT
*       Zero; setter success is not reported.
*
*   NOTES
*       Color notification receives a pointer to object-owned RGB storage.
*       The pointer is borrowed and remains valid only while the object lives.
*
*/

/*** Methods ****************************************************************/
/* Create the superclass, returning NULL on failure. Initialize Medium size,
 * black preset, RGB acquisition pending, and no acquired pen. Process tags
 * in order, range-check Type, and copy RGB or PenSpec input. PenSpec selects
 * the matching pending acquisition kind.
 */
Object *Lamp__OM_NEW(struct IClass *cl, Object *obj, struct opSet *msg)
{
    struct Lamp_DATA           *data;
    struct TagItem             *tstate = msg->ops_AttrList;
    struct TagItem             *tag;

    obj = (Object *)DoSuperMethodA(cl, obj, (Msg)msg);
    if (!obj) return FALSE;

    data = INST_DATA(cl, obj);

    // Defaults
    data->lmp_Type = MUIV_Lamp_Type_Medium;
    data->lmp_ColorSpec = MUIV_Lamp_Color_Off;
    data->lmp_Color[0] = defaultcolors[0][0];
    data->lmp_Color[1] = defaultcolors[0][1];
    data->lmp_Color[2] = defaultcolors[0][2];
    data->lmp_ColorType = MUIV_Lamp_ColorType_UserDefined;
    data->lmp_PenChanged = PCH_NEWPEN;
    data->lmp_PenChangedOld = PCH_NONE;
    data->lmp_PenNr = -1;
    data->lmp_Style = MUIV_Lamp_Style_Flat;
    data->lmp_Diameter = 0;
    data->lmp_Intensity = 100;
    {
        ULONG i;
        for (i = 0; i < LAMP_EFFECT_PENS; i++)
            data->lmp_EffectPen[i] = -1;
    }

    while ((tag = NextTagItem(&tstate)) != NULL)
    {
        switch (tag->ti_Tag)
        {
            case MUIA_Lamp_Type:
                if (tag->ti_Data <= MUIV_Lamp_Type_Huge)
                    data->lmp_Type = tag->ti_Data;
                break;

            case MUIA_Lamp_Color:
                if (tag->ti_Data < 12)
                {
                    data->lmp_ColorSpec = tag->ti_Data;
                    data->lmp_Color[0] = defaultcolors[data->lmp_ColorSpec][0];
                    data->lmp_Color[1] = defaultcolors[data->lmp_ColorSpec][1];
                    data->lmp_Color[2] = defaultcolors[data->lmp_ColorSpec][2];
                    data->lmp_ColorType = MUIV_Lamp_ColorType_UserDefined;
                }
                else
                {
                    ULONG *cols = (ULONG *)tag->ti_Data;
                    data->lmp_Color[0] = cols[0];
                    data->lmp_Color[1] = cols[1];
                    data->lmp_Color[2] = cols[2];
                    data->lmp_ColorType = MUIV_Lamp_ColorType_Color;
                }
                break;

            case MUIA_Lamp_Red:
                data->lmp_Color[0] = tag->ti_Data;
                data->lmp_ColorType = MUIV_Lamp_ColorType_Color;
                break;

            case MUIA_Lamp_Green:
                data->lmp_Color[1] = tag->ti_Data;
                data->lmp_ColorType = MUIV_Lamp_ColorType_Color;
                break;

            case MUIA_Lamp_Blue:
                data->lmp_Color[2] = tag->ti_Data;
                data->lmp_ColorType = MUIV_Lamp_ColorType_Color;
                break;

            case MUIA_Lamp_PenSpec:
                if (tag->ti_Data)
                {
                    data->lmp_PenSpec = *((struct MUI_PenSpec *)tag->ti_Data);
                    data->lmp_ColorType = MUIV_Lamp_ColorType_PenSpec;
                    data->lmp_PenChanged = PCH_NEWSPEC;
                }
                break;

            case MUIA_Lamp_Style:
                if (tag->ti_Data <= MUIV_Lamp_Style_LED)
                    data->lmp_Style = tag->ti_Data;
                break;

            case MUIA_Lamp_Diameter:
                if (!tag->ti_Data || (tag->ti_Data >= 5 && tag->ti_Data <= LAMP_MAX_DIAMETER))
                    data->lmp_Diameter = tag->ti_Data;
                break;

            case MUIA_Lamp_Intensity:
                if (tag->ti_Data <= 100)
                    data->lmp_Intensity = tag->ti_Data;
                break;
        }
    }

    return obj;
}


/* Apply color, style and size updates; relayout only in a setup window. */
IPTR Lamp__OM_SET(struct IClass *cl, Object *obj, struct opSet *msg)
{
    struct Lamp_DATA *data = INST_DATA(cl, obj);
    struct TagItem *tstate = msg->ops_AttrList, *tag;
    BOOL relayout = FALSE, redraw = FALSE, full = FALSE, recolor = FALSE;
    IPTR result;
    while ((tag = NextTagItem(&tstate)) != NULL)
    {
        switch (tag->ti_Tag)
        {
            case MUIA_Lamp_Type:
                if (tag->ti_Data <= MUIV_Lamp_Type_Huge && data->lmp_Type != tag->ti_Data)
                {
                    data->lmp_Type = tag->ti_Data;
                    redraw = TRUE;
                    if (!data->lmp_Diameter) relayout = TRUE;
                }
                break;
            case MUIA_Lamp_Color:
                if (tag->ti_Data < 12)
                {
                    data->lmp_ColorSpec = tag->ti_Data;
                    data->lmp_Color[0] = defaultcolors[tag->ti_Data][0];
                    data->lmp_Color[1] = defaultcolors[tag->ti_Data][1];
                    data->lmp_Color[2] = defaultcolors[tag->ti_Data][2];
                    data->lmp_ColorType = MUIV_Lamp_ColorType_UserDefined;
                    recolor = TRUE;
                }
                else
                {
                    const ULONG *rgb = (const ULONG *)tag->ti_Data;
                    data->lmp_Color[0] = rgb[0];
                    data->lmp_Color[1] = rgb[1];
                    data->lmp_Color[2] = rgb[2];
                    data->lmp_ColorType = MUIV_Lamp_ColorType_Color;
                    recolor = TRUE;
                }
                break;
            case MUIA_Lamp_Red:
                data->lmp_Color[0] = tag->ti_Data;
                data->lmp_ColorType = MUIV_Lamp_ColorType_Color;
                recolor = TRUE;
                break;
            case MUIA_Lamp_Green:
                data->lmp_Color[1] = tag->ti_Data;
                data->lmp_ColorType = MUIV_Lamp_ColorType_Color;
                recolor = TRUE;
                break;
            case MUIA_Lamp_Blue:
                data->lmp_Color[2] = tag->ti_Data;
                data->lmp_ColorType = MUIV_Lamp_ColorType_Color;
                recolor = TRUE;
                break;
            case MUIA_Lamp_PenSpec:
                if (tag->ti_Data)
                {
                    data->lmp_PenSpec = *((struct MUI_PenSpec *)tag->ti_Data);
                    data->lmp_ColorType = MUIV_Lamp_ColorType_PenSpec;
                    recolor = TRUE;
                }
                break;
            case MUIA_Lamp_Style:
                if (tag->ti_Data <= MUIV_Lamp_Style_LED && data->lmp_Style != tag->ti_Data)
                {
                    data->lmp_Style = tag->ti_Data;
                    full = TRUE;
                    recolor = TRUE;
                }
                break;
            case MUIA_Lamp_Diameter:
                if ((!tag->ti_Data || (tag->ti_Data >= 5 && tag->ti_Data <= LAMP_MAX_DIAMETER)) &&
                    data->lmp_Diameter != tag->ti_Data)
                {
                    data->lmp_Diameter = tag->ti_Data;
                    relayout = TRUE;
                    full = TRUE;
                }
                break;
            case MUIA_Lamp_Intensity:
                if (tag->ti_Data <= 100 && data->lmp_Intensity != tag->ti_Data)
                {
                    data->lmp_Intensity = tag->ti_Data;
                    recolor = TRUE;
                }
                break;
        }
    }
    if (recolor)
    {
        data->lmp_PenChanged = data->lmp_ColorType == MUIV_Lamp_ColorType_PenSpec ?
                               PCH_NEWSPEC : PCH_NEWPEN;
        redraw = TRUE;
    }
    result = DoSuperMethodA(cl, obj, (Msg)msg);
    if (relayout && XGET(obj, MUIA_WindowObject) && _parent(obj))
    {
        DoMethod(_parent(obj), MUIM_Group_InitChange);
        DoMethod(_parent(obj), MUIM_Group_ExitChange);
    }
    if (full) MUI_Redraw(obj, MADF_DRAWOBJECT);
    else if (redraw) MUI_Redraw(obj, MADF_DRAWUPDATE);
    return result;
}


/* Return selectors, stored RGB values or borrowed instance pointers according to ColorType. */
IPTR Lamp__OM_GET(struct IClass *cl, Object *obj, struct opGet *msg)
{
    struct Lamp_DATA   *data  = INST_DATA(cl, obj);
    IPTR               *store = msg->opg_Storage;

    switch (msg->opg_AttrID)
    {
        case MUIA_Lamp_Type:
            *store = data->lmp_Type;
            return TRUE;

        case MUIA_Lamp_Color:
            if (data->lmp_ColorType == MUIV_Lamp_ColorType_UserDefined)
            {
                *store = (IPTR)data->lmp_ColorSpec;
            }
            else if (data->lmp_ColorType == MUIV_Lamp_ColorType_Color)
            {
                *store = (IPTR)data->lmp_Color;
            }
            else // MUIV_Lamp_ColorType_PenSpec
            {
                *store = 0;
            }
            return TRUE;

        case MUIA_Lamp_ColorType:
            *store = data->lmp_ColorType;
            return TRUE;

        case MUIA_Lamp_Red:
            *store = data->lmp_Color[0];
            return TRUE;

        case MUIA_Lamp_Green:
            *store = data->lmp_Color[1];
            return TRUE;

        case MUIA_Lamp_Blue:
            *store = data->lmp_Color[2];
            return TRUE;

        case MUIA_Lamp_Style:
            *store = data->lmp_Style;
            return TRUE;

        case MUIA_Lamp_Diameter:
            *store = data->lmp_Diameter;
            return TRUE;

        case MUIA_Lamp_Intensity:
            *store = data->lmp_Intensity;
            return TRUE;

        case MUIA_Lamp_PenSpec:
            *store = (IPTR)&data->lmp_PenSpec;
            return TRUE;
    }
    
    return DoSuperMethodA(cl, obj, (Msg) msg);
}


/* Release all acquired color/effect pens; preserve pending color kind. */
IPTR Lamp__MUIM_Cleanup(struct IClass *cl, Object *obj, Msg msg)
{
    struct Lamp_DATA *data = INST_DATA(cl, obj);
    lamp_release_pens(obj, data);
    data->lmp_PenChanged = data->lmp_ColorType == MUIV_Lamp_ColorType_PenSpec ?
                           PCH_NEWSPEC : PCH_NEWPEN;
    return DoSuperMethodA(cl, obj, msg);
}


/* Draw a legacy flat indicator or optional shaded LED using cached pens. */
IPTR Lamp__MUIM_Draw(struct IClass *cl, Object *obj, struct MUIP_Draw *msg)
{
    struct Lamp_DATA *data = INST_DATA(cl, obj);
    ULONG size;
    DoSuperMethodA(cl, obj, (Msg)msg);
    if (!(msg->flags & (MADF_DRAWOBJECT | MADF_DRAWUPDATE))) return 0;
    if (data->lmp_PenChanged) lamp_acquire_pens(obj, data);
    size = lamp_diameter(data);
    if (_mright(obj) < _mleft(obj) || _mbottom(obj) < _mtop(obj)) return 0;
    if (size > (ULONG)(_mright(obj) - _mleft(obj) + 1))
        size = (ULONG)(_mright(obj) - _mleft(obj) + 1);
    if (size > (ULONG)(_mbottom(obj) - _mtop(obj) + 1))
        size = (ULONG)(_mbottom(obj) - _mtop(obj) + 1);
    if (!size) return 0;
    if (data->lmp_Style == MUIV_Lamp_Style_LED)
    {
        lamp_draw_led(obj, data, size);
    }
    else
    {
        LONG right = _mleft(obj) + (LONG)size;
        LONG bottom = _mtop(obj) + (LONG)size;
        if (right > _mright(obj)) right = _mright(obj);
        if (bottom > _mbottom(obj)) bottom = _bottom(obj);
        if (data->lmp_PenNr != -1)
        {
            if (data->lmp_PenChangedOld == PCH_NEWSPEC)
                SetAPen(_rp(obj), MUIPEN(data->lmp_PenNr));
            else SetAPen(_rp(obj), data->lmp_PenNr);
        }
        else if (_pens(obj)) SetAPen(_rp(obj), _pens(obj)[MPEN_TEXT]);
        else return 0;
        RectFill(_rp(obj), _mleft(obj), _mtop(obj), right, bottom);
    }
    return 0;
}


/* Add validated Type or explicit Diameter dimensions to superclass bounds. */
IPTR Lamp__MUIM_AskMinMax(struct IClass *cl, Object *obj, struct MUIP_AskMinMax *msg)
{
    struct Lamp_DATA *data = INST_DATA(cl, obj);
    ULONG size = lamp_diameter(data);
    DoSuperMethodA(cl, obj, (Msg)msg);
    msg->MinMaxInfo->MinWidth += size;
    msg->MinMaxInfo->DefWidth += size;
    msg->MinMaxInfo->MaxWidth += size;
    msg->MinMaxInfo->MinHeight += size;
    msg->MinMaxInfo->DefHeight += size;
    msg->MinMaxInfo->MaxHeight += size;
    return 0;
}


/* Set Color using persistent instance storage for notification safety. */
IPTR Lamp__MUIM_Lamp_SetRGB(struct IClass *cl, Object *obj, struct MUIP_Lamp_SetRGB *msg)
{
    struct Lamp_DATA *data = INST_DATA(cl, obj);

    data->lmp_Color[0] = msg->red;
    data->lmp_Color[1] = msg->green;
    data->lmp_Color[2] = msg->blue;

    SET(obj, MUIA_Lamp_Color, data->lmp_Color);

    return 0;
}
