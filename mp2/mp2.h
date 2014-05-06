#ifndef __MP2_H__
#define __MP2_H__
#ifdef __cplusplus
extern "C" {
#endif
extern GLfloat sealevel;
extern GLfloat *verts;
extern GLfloat *norms;
extern GLuint *faces;
extern int res;

void makemountain(void);
#ifdef __cplusplus
}
#endif
#endif
