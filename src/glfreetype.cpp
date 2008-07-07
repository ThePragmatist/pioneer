#include <SDL_opengl.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <map>
#include <ft2build.h>
#include "glfreetype.h"

#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H


#ifdef _WIN32
typedef GLvoid (APIENTRY *_GLUfuncptr)();
#endif

FT_Library library;

#include <vector>

static GLUtesselator *tobj;

static inline double fac(int n)
{
	double r = 1.0;
	for (int i=2; i<=n; i++) {
		r *= (double)i;
	}
	return r;
}

static inline double binomial_coeff(int n, int m)
{
	return fac(n)/(fac(m)*(fac(n-m)));
}

static void eval_bezier (GLdouble *out, double t, int n_points, double *points)
{
	std::vector<double> c(n_points);

	for (int i=0; i<n_points; i++) {
		c[i] = pow(1.0f-t, n_points-(i+1)) * pow(t,i) *
			binomial_coeff(n_points-1, i);
	}

	out[0] = out[1] = out[2] = 0;

	for (int i=0; i<n_points; i++) {
		out[0] += points[3*i] * c[i];
		out[1] += points[3*i+1] * c[i];
		out[2] += points[3*i+2] * c[i];
	}
}

#define DIV 2048.0f

bool GenContourPoints(int a_char, FT_Outline *a_outline, const int a_contour, int a_bezierIters, std::vector<double> *ao_points)
{
#define push_point(__p) { \
	ao_points->push_back((__p)[0]); \
	ao_points->push_back((__p)[1]); \
	ao_points->push_back((__p)[2]); \
	}

	int cont = (a_contour-1 < 0 ? 0 : 1+a_outline->contours[a_contour-1]);

	double point_buf[256][3];
	char point_type[256];
	int pos = 0;

	for (; cont<=a_outline->contours[a_contour]; cont++, pos++) {
		point_type[pos] = a_outline->tags[cont];
		point_buf[pos][0] = a_outline->points[cont].x/DIV;
		point_buf[pos][1] = a_outline->points[cont].y/DIV;
		point_buf[pos][2] = 0;
	}
	if (!point_type[pos-1]) {
		// need to duplicate first vertex if last
		// section is a bezier
		point_type[pos] = 1;
		point_buf[pos][0] = point_buf[0][0];
		point_buf[pos][1] = point_buf[0][1];
		point_buf[pos][2] = 0;
		pos++;
	}

	int start = -1;
	for (int k=0; k<pos; k++) {
		if (!(point_type[k] & 1)) continue;
			
		if (start == -1) { start = k; continue; }
		
		int len = 1+k-start;
		// trace segment
		if (len == 2) {
			// straight line
			push_point(point_buf[k-1]);
			push_point(point_buf[k]);
		} else {
			// bezier
			double b_in[3][3];
			double v[3];

			// truetype is all quadratic bezier,
			// using average points between
			// 'control points' as end points
			
			// first bezier
			b_in[0][0] = point_buf[start][0];
			b_in[0][1] = point_buf[start][1];
			b_in[0][2] = 0;

			b_in[1][0] = point_buf[start+1][0];
			b_in[1][1] = point_buf[start+1][1];
			b_in[1][2] = 0;

			if (len > 3) {
				b_in[2][0] = 0.5 * (point_buf[start+1][0] + point_buf[start+2][0]);
				b_in[2][1] = 0.5 * (point_buf[start+1][1] + point_buf[start+2][1]);
				b_in[2][2] = 0;
			} else {
				b_in[2][0] = point_buf[start+2][0];
				b_in[2][1] = point_buf[start+2][1];
				b_in[2][2] = 0;
			}

			for (int l=0; l<=a_bezierIters; l++) {
				double t = (1.0/a_bezierIters)*l;
				eval_bezier(v, t, 3, &b_in[0][0]);
				v[2] = 0.0;
				push_point(v);
			}

			// middle beziers
			
			if (len > 4) {
				for (int _p=1; _p < len-3; _p++) {
					b_in[0][0] = 0.5*(point_buf[start+_p][0] + point_buf[start+_p+1][0]);
					b_in[0][1] = 0.5*(point_buf[start+_p][1] + point_buf[start+_p+1][1]);
					b_in[0][2] = 0;

					b_in[1][0] = point_buf[start+_p+1][0];
					b_in[1][1] = point_buf[start+_p+1][1];
					b_in[1][2] = 0;

					b_in[2][0] = 0.5*(point_buf[start+_p+1][0] + point_buf[start+_p+2][0]);
					b_in[2][1] = 0.5*(point_buf[start+_p+1][1] + point_buf[start+_p+2][1]);
					b_in[2][2] = 0;
					
					for (int l=0; l<=a_bezierIters; l++) {
						double t = (1.0/a_bezierIters)*l;
						eval_bezier(v, t, 3, &b_in[0][0]);
						v[2] = 0.0;
						push_point(v);
					}
				}
			}

			// end
			if (len > 3) {
				const int _p = start+len-3;
				b_in[0][0] = 0.5 * (point_buf[_p][0] + point_buf[_p+1][0]);
				b_in[0][1] = 0.5 * (point_buf[_p][1] + point_buf[_p+1][1]);
				b_in[0][2] = 0;

				b_in[1][0] = point_buf[_p+1][0];
				b_in[1][1] = point_buf[_p+1][1];
				b_in[1][2] = 0;

				b_in[2][0] = point_buf[_p+2][0];
				b_in[2][1] = point_buf[_p+2][1];
				b_in[2][2] = 0;

				for (int l=0; l<=a_bezierIters; l++) {
					double t = (1.0/a_bezierIters)*l;
					eval_bezier(v, t, 3, &b_in[0][0]);
					v[2] = 0.0;
					push_point(v);
				}
			}
		}
		start = k;
	}
	return true;
}

#ifndef CALLBACK
# ifdef WIN32
#  define CALLBACK __attribute__ ((__stdcall__))
# else
#  define CALLBACK
# endif
#endif /* CALLBACK */



struct TessData
{
	std::vector<double> *pts;		// inputs, added by combine
	int numvtx;

	std::vector<Uint16> index;		// output index list
	GLenum lasttype;
	int state;		// 0, no vertices, 1, 1 vertex, 2, 2 or more
					// 0x4 => clockwise
	Uint16 vtx[2];
};

static Uint16 g_index[65536];

void CALLBACK beginCallback(GLenum which, GLvoid *poly_data)
{
	TessData *pData = (TessData *)poly_data;
	pData->lasttype = which;
	pData->state = 0;
}

void CALLBACK errorCallback(GLenum errorCode)
{
	const GLubyte *estring;

	estring = gluErrorString(errorCode);
	fprintf(stderr, "Tessellation Error: %s\n", estring);
}

void CALLBACK endCallback(void)
{
}

void CALLBACK vertexCallback(GLvoid *vertex, GLvoid *poly_data)
{
	TessData *pData = (TessData *)poly_data;
	Uint16 index = *(Uint16 *)vertex;
	switch (pData->lasttype)
	{
		case GL_TRIANGLES:
		pData->index.push_back(index);
		break;
	
		case GL_TRIANGLE_STRIP:
		if ((pData->state & 3) < 2)
			pData->vtx[pData->state++] = index;
		else {
			pData->index.push_back(index);
			if (pData->state & 0x4) {
				pData->index.push_back(pData->vtx[1]);
				pData->index.push_back(pData->vtx[0]);
			} else {
				pData->index.push_back(pData->vtx[0]);
				pData->index.push_back(pData->vtx[1]);
			}
			pData->vtx[0] = pData->vtx[1];
			pData->vtx[1] = index;
			pData->state ^= 0x4;
		}
		break;

		case GL_TRIANGLE_FAN:
		if ((pData->state & 3) < 2)
			pData->vtx[pData->state++] = index;
		else {
			pData->index.push_back(index);
			pData->index.push_back(pData->vtx[0]);
			pData->index.push_back(pData->vtx[1]);
			pData->vtx[1] = index;
		}		
	}
}

void CALLBACK combineCallback(GLdouble coords[3], 
                     GLdouble *vertex_data[4],
                     GLfloat weight[4], void **dataOut, void *poly_data)
{
	TessData *pData = (TessData *)poly_data;
	pData->pts->push_back(coords[0]);
	pData->pts->push_back(coords[1]);
	pData->pts->push_back(coords[2]);
	*dataOut = (void *)&g_index[pData->numvtx++];
}

#define BEZIER_STEPS 2

void FontFace::RenderGlyph(int chr)
{
	glEnableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_NORMAL_ARRAY);

	glfglyph_t *glyph = &m_glyphs[chr];
	glVertexPointer (3, GL_FLOAT, 3*sizeof(float), glyph->varray);
	glDrawElements (GL_TRIANGLES, glyph->numidx, GL_UNSIGNED_SHORT, glyph->iarray);
}

void FontFace::RenderString(const char *str)
{
	glPushMatrix();
	for (unsigned int i=0; i<strlen(str); i++) {
		if (str[i] == '\n') {
			glPopMatrix();
			glTranslatef(0,-m_height,0);
			glPushMatrix();
		} else {
			glfglyph_t *glyph = &m_glyphs[str[i]];
			if (glyph->numidx) RenderGlyph(str[i]);
			glTranslatef(glyph->advx,0,0);
		}
	}
	glPopMatrix();
}

FontFace::FontFace(const char *filename_ttf)
{
	FT_Face face;
	if (0 != FT_New_Face(library, filename_ttf, 0, &face)) {
		fprintf(stderr, "Terrible error! Couldn't load '%s'\n", filename_ttf);
	} else {
		FT_Set_Char_Size(face, 50*64, 0, 100, 0);
		for (int chr=32; chr<127; chr++) {
			if (0 != FT_Load_Char(face, chr, FT_LOAD_NO_SCALE)) {
				printf("Couldn't load glyph\n");
				continue;
			}
			
			assert(face->glyph->format == FT_GLYPH_FORMAT_OUTLINE);
			FT_Outline *outline = &face->glyph->outline;

			std::vector<double> temppts;
			std::vector<Uint16> indices;
			std::vector<double> pts;
			int nv = 0;

			TessData tessdata;
			tessdata.pts = &pts;

			gluTessNormal (tobj, 0, 0, 1);
			gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
			gluTessBeginPolygon (tobj, &tessdata);
			for (int contour=0; contour < outline->n_contours; contour++)
			{
				gluTessBeginContour (tobj);
				temppts.clear();
				GenContourPoints(chr, outline, contour, BEZIER_STEPS, &temppts);
				for (size_t i=0; i<temppts.size(); i++) pts.push_back(temppts[i]);
				for (size_t i=0; i<temppts.size(); i+=3, nv++)
					gluTessVertex(tobj,&pts[nv*3],&g_index[nv]);
				gluTessEndContour(tobj);
			}
			tessdata.numvtx = nv;
			gluTessEndPolygon(tobj);

			glfglyph_t _face;

			nv = tessdata.numvtx;
			_face.varray = (float *) malloc (nv*3*sizeof(float));
			for (int i=0; i<nv*3; i++) _face.varray[i] = (float) pts[i];

			_face.numidx = (int) tessdata.index.size();
			_face.iarray = (Uint16 *) malloc (_face.numidx*sizeof(Uint16));
			for (int i=0; i<_face.numidx; i++) _face.iarray[i] = tessdata.index[i];

			_face.advx = face->glyph->linearHoriAdvance/(float)(1<<16)/72.0f;
			_face.advy = face->glyph->linearVertAdvance/(float)(1<<16)/72.0f;
			//printf("%f,%f\n", _face.advx, _face.advy);
			m_glyphs[chr] = _face;
		}
		
		m_height = m_glyphs['M'].advy;
		m_width = m_glyphs['M'].advx;
	}
}

void GLFTInit()
{
	if (0 != FT_Init_FreeType(&library)) {
		printf("Couldn't init freetype library.\n");
		exit(0);
	}

	tobj = gluNewTess ();
	gluTessCallback(tobj, GLU_TESS_VERTEX_DATA, (_GLUfuncptr) vertexCallback);
	gluTessCallback(tobj, GLU_TESS_BEGIN_DATA, (_GLUfuncptr) beginCallback);
	gluTessCallback(tobj, GLU_TESS_END, (_GLUfuncptr) endCallback);
	gluTessCallback(tobj, GLU_TESS_ERROR, (_GLUfuncptr) errorCallback);
	gluTessCallback(tobj, GLU_TESS_COMBINE_DATA, (_GLUfuncptr) combineCallback);

	for (Uint16 i=0; i<65535; i++) g_index[i] = i;
}