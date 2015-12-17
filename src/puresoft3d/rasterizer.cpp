#include <stdlib.h>
#include <stdexcept>
#include "rasterizer.h"

using namespace std;

PuresoftRasterizer::PuresoftRasterizer(int width, int height)
{
	m_width = width;
	m_height = height;
	m_halfWidth = m_width / 2;
	m_halfHeight = m_height / 2;
	m_output.m_rows = new RESULT_ROW[height];
}


PuresoftRasterizer::~PuresoftRasterizer(void)
{
	delete[] m_output.m_rows;
}

class LineSegment
{
	int dx, dy, x0, y0;
public:
	LineSegment(const PuresoftRasterizer::VERTEX2I* verts, int idx0, int idx1)
	{
		x0 = verts[idx0].x;
		y0 = verts[idx0].y;
		dx = verts[idx1].x - x0;
		dy = verts[idx1].y - y0;
		index0 = idx0;
		index1 = idx1;
	}

	int operator() (int y) const
	{
		return dx * (y - y0) / dy + x0;
	}

	int index0, index1;
};

void PuresoftRasterizer::pushVertex(int idx, const float* vert)
{
	m_output.vertices[idx].y = (int)(m_halfHeight * vert[1]) + m_halfHeight;
	m_output.vertices[idx].x = (int)(m_halfWidth  * vert[0]) + m_halfWidth;

	if(0 == idx)
	{
		m_output.firstRow = m_output.lastRow = m_output.vertices[0].y;
	}
	else if(m_output.vertices[idx].y > m_output.lastRow)
	{
		m_output.lastRow = m_output.vertices[idx].y;
	}
	else if(m_output.vertices[idx].y < m_output.firstRow)
	{
		m_output.firstRow = m_output.vertices[idx].y;
	}
}

// for flat top/bottom triangle
void PuresoftRasterizer::processTriangle(const LineSegment& edgeL, const LineSegment& edgeR, int yMin, int yMax)
{
	for(yMin = max(yMin, 0); yMin <= yMax && yMin < m_height; yMin++) 
	{
		m_output.m_rows[yMin].left = edgeL(yMin);
		if(m_output.m_rows[yMin].left < 0)
			m_output.m_rows[yMin].left = 0;
		m_output.m_rows[yMin].right = edgeR(yMin);
		if(m_output.m_rows[yMin].right >= m_width)
			m_output.m_rows[yMin].right = m_width - 1;
		m_output.m_rows[yMin].leftVerts[0] = edgeL.index0;
		m_output.m_rows[yMin].leftVerts[1] = edgeL.index1;
		m_output.m_rows[yMin].rightVerts[0] = edgeR.index0;
		m_output.m_rows[yMin].rightVerts[1] = edgeR.index1;
	}
}

// for non- flat top/bottom triangle, we split it to two flat top/bottom triangles
void PuresoftRasterizer::processStandingTriangle(const VERTEX2I* verts, int top, int bottom, int third)
{
	LineSegment edgeTopThird(verts, top, third), edgeTopBottom(verts, top, bottom), edgeThirdBottom(verts, third, bottom);
	
	// the triangle is pointing left
	if(edgeTopBottom(verts[third].y) > edgeTopThird(verts[third].y))
	{
		// upper half
		processTriangle(edgeTopThird, edgeTopBottom, verts[third].y, verts[top].y);
		// lower half
		processTriangle(edgeThirdBottom, edgeTopBottom, verts[bottom].y, verts[third].y);
	}
	else
	{
		// upper half
		processTriangle(edgeTopBottom, edgeTopThird, verts[third].y, verts[top].y);
		// lower half
		processTriangle(edgeTopBottom, edgeThirdBottom, verts[bottom].y, verts[third].y);
	}
}

const PuresoftRasterizer::RESULT* PuresoftRasterizer::getResultPtr(void) const
{
	if(!m_output.m_rows)
	{
		throw bad_alloc("PuresoftRasterizer::RESULT::RESULT_ROWS");
	}

	return &m_output;
}

void PuresoftRasterizer::pushTriangle(const float* vert0, const float* vert1, const float* vert2)
{
	// integerize
	pushVertex(0, vert0);
	pushVertex(1, vert1);
	pushVertex(2, vert2);

	if(m_output.firstRow >= m_height || m_output.lastRow < 0)
	{
		m_output.firstRow = 0;
		m_output.lastRow = -1;
	}

	if(m_output.firstRow < 0)
	{
		m_output.firstRow = 0;
	}

	if(m_output.lastRow >= m_height)
	{
		m_output.lastRow = m_height - 1;
	}

	if(m_output.firstRow == m_output.lastRow)
	{
		return;
	}

	// flat top/bottom
	if(m_output.vertices[0].y == m_output.vertices[1].y)
	{
		if(m_output.vertices[0].x < m_output.vertices[1].x)
		{
			processTriangle(LineSegment(m_output.vertices, 0, 2), LineSegment(m_output.vertices, 1, 2), m_output.firstRow, m_output.lastRow);
		}
		else
		{
			processTriangle(LineSegment(m_output.vertices, 1, 2), LineSegment(m_output.vertices, 0, 2), m_output.firstRow, m_output.lastRow);
		}
	}
	else if(m_output.vertices[0].y == m_output.vertices[2].y)
	{
		if(m_output.vertices[0].x < m_output.vertices[2].x)
		{
			processTriangle(LineSegment(m_output.vertices, 0, 1), LineSegment(m_output.vertices, 2, 1), m_output.firstRow, m_output.lastRow);
		}
		else
		{
			processTriangle(LineSegment(m_output.vertices, 2, 1), LineSegment(m_output.vertices, 0, 1), m_output.firstRow, m_output.lastRow);
		}
	}
	else if(m_output.vertices[1].y == m_output.vertices[2].y)
	{
		if(m_output.vertices[1].x < m_output.vertices[2].x)
		{
			processTriangle(LineSegment(m_output.vertices, 1, 0), LineSegment(m_output.vertices, 2, 0), m_output.firstRow, m_output.lastRow);
		}
		else
		{
			processTriangle(LineSegment(m_output.vertices, 2, 0), LineSegment(m_output.vertices, 1, 0), m_output.firstRow, m_output.lastRow);
		}
	}
	// non- flat top/bottom
	else
	{
		// sort the 3 Ys
		int top, bottom, third = 2;

		if(m_output.vertices[0].y > m_output.vertices[1].y)
		{
			top = 0;
			bottom = 1;
		}
		else
		{
			top = 1;
			bottom = 0;
		}

		if(m_output.vertices[top].y < m_output.vertices[third].y)
		{
			top ^= third, third ^= top, top ^= third;
		}
		else if(m_output.vertices[bottom].y > m_output.vertices[third].y)
		{
			bottom ^= third, third ^= bottom, bottom ^= third;
		}

		processStandingTriangle(m_output.vertices, top, bottom, third);
	}
}
