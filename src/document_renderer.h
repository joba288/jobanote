#ifndef DOCUMENT_RENDERER_H
#define DOCUMENT_RENDERER_H

#include "jobanote.h"
#include "jbgl.h"

void render_document(Document* doc, JbglState* state, JbglFont* font, JbglTexture cursor_texture, float scroll_y);

void draw_selection(Document* doc,Cursor start,Cursor end,TextNode* text,float x,float y,JbglState* state,JbglFont* font);

#endif