#ifndef EXPANDED_SQUAD_LAYOUT_H
#define EXPANDED_SQUAD_LAYOUT_H
/* All coordinates are original SWOS 320x256 indexed framebuffer pixels. */
enum { EXP_SQUAD_ROW_TOP=92, EXP_SQUAD_ROW_H=8,
       EXP_SQUAD_TOOLTIP_X=70, EXP_SQUAD_TOOLTIP_W=172,
       EXP_SQUAD_TOOLTIP_H=52 };
static inline int Expanded_SquadRowY(unsigned row) {
    return EXP_SQUAD_ROW_TOP+(int)row*EXP_SQUAD_ROW_H;
}
static inline int Expanded_SquadTooltipY(unsigned row) {
    const int focusY=Expanded_SquadRowY(row);
    const int below=focusY+EXP_SQUAD_ROW_H+2;
    /* A tooltip must never intersect the selected row, even at row 16. */
    return below+EXP_SQUAD_TOOLTIP_H<=232 ? below :
           focusY-EXP_SQUAD_TOOLTIP_H-3;
}
#endif
