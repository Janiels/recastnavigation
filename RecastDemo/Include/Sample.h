//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef RECASTSAMPLE_H
#define RECASTSAMPLE_H

#include "Recast.h"
#include "DetourNavMeshQuery.h"
#include "SampleInterfaces.h"


/// Tool types.
enum SampleToolType
{
	TOOL_NONE = 0,
	TOOL_TILE_EDIT,
	TOOL_TILE_HIGHLIGHT,
	TOOL_TEMP_OBSTACLE,
	TOOL_NAVMESH_TESTER,
	TOOL_NAVMESH_PRUNE,
	TOOL_OFFMESH_CONNECTION,
	TOOL_CONVEX_VOLUME,
	TOOL_CROWD,
	MAX_TOOLS
};

enum SamplePolyAreas
{
	// 32-bits are available in each polygon to mark the area.
	// The user is free to choose how these bits are used. In the
	// sample, 24 bits are used to specify an area type, and 8
	// bits are used to specify additional flags. During the build
	// process, we can set bits in polygons with rcAreaModification, see the 
	// convex volume tool and the mesh processes.
	// When querying the mesh, the bits can be used to modify how queries behave;
	// see XXX (TODO).
	SAMPLE_POLYAREA_TYPE_MASK		= 0x00ffffff,
	SAMPLE_POLYAREA_TYPE_GROUND		= 0x01,
	SAMPLE_POLYAREA_TYPE_WATER		= 0x02,
	SAMPLE_POLYAREA_TYPE_ROAD		= 0x03,
	SAMPLE_POLYAREA_TYPE_GRASS		= 0x04,

	SAMPLE_POLYAREA_FLAG_DOOR		= 0x01000000, // Door flag. Can be combined with areas.
	SAMPLE_POLYAREA_FLAG_JUMP		= 0x02000000, // Jump flag. Can be combined with areas.
	SAMPLE_POLYAREA_FLAG_DISABLED	= 0x04000000, // Disabled flag. Can be combined with areas.
};

class SampleDebugDraw : public DebugDrawGL
{
public:
	virtual unsigned int areaToCol(unsigned int area);
};

class SampleQueryFilter : public dtQueryFilter
{
	unsigned int m_includeFlags;
	unsigned int m_excludeFlags;
public:
	SampleQueryFilter();

	bool passFilter(
		const dtPolyRef ref,
		const dtMeshTile* tile,
		const dtPoly* poly) const override;

	float getCost(
		const float* pa, const float* pb,
		const dtPolyRef prevRef, const dtMeshTile* prevTile, const dtPoly* prevPoly,
		const dtPolyRef curRef, const dtMeshTile* curTile, const dtPoly* curPoly,
		const dtPolyRef nextRef, const dtMeshTile* nextTile, const dtPoly* nextPoly) const override;

	unsigned int getIncludeFlags() const { return m_includeFlags; }
	void setIncludeFlags(unsigned int flags) { m_includeFlags = flags; }
	unsigned int getExcludeFlags() const { return m_excludeFlags; }
	void setExcludeFlags(unsigned int flags) { m_excludeFlags = flags; }
};

enum SamplePartitionType
{
	SAMPLE_PARTITION_WATERSHED,
	SAMPLE_PARTITION_MONOTONE,
	SAMPLE_PARTITION_LAYERS,
};

struct SampleTool
{
	virtual ~SampleTool() {}
	virtual int type() = 0;
	virtual void init(class Sample* sample) = 0;
	virtual void reset() = 0;
	virtual void handleMenu() = 0;
	virtual void handleClick(const float* s, const float* p, bool shift) = 0;
	virtual void handleRender() = 0;
	virtual void handleRenderOverlay(double* proj, double* model, int* view) = 0;
	virtual void handleToggle() = 0;
	virtual void handleStep() = 0;
	virtual void handleUpdate(const float dt) = 0;
};

struct SampleToolState {
	virtual ~SampleToolState() {}
	virtual void init(class Sample* sample) = 0;
	virtual void reset() = 0;
	virtual void handleRender() = 0;
	virtual void handleRenderOverlay(double* proj, double* model, int* view) = 0;
	virtual void handleUpdate(const float dt) = 0;
};

class Sample
{
protected:
	class InputGeom* m_geom;
	class dtNavMesh* m_navMesh;
	class dtNavMeshQuery* m_navQuery;
	class dtCrowd* m_crowd;

	unsigned char m_navMeshDrawFlags;

	float m_cellSize;
	float m_cellHeight;
	float m_agentHeight;
	float m_agentRadius;
	float m_agentMaxClimb;
	float m_agentMaxSlope;
	float m_regionMinSize;
	float m_regionMergeSize;
	float m_edgeMaxLen;
	float m_edgeMaxError;
	float m_vertsPerPoly;
	float m_detailSampleDist;
	float m_detailSampleMaxError;
	int m_partitionType;

	bool m_filterLowHangingObstacles;
	bool m_filterLedgeSpans;
	bool m_filterWalkableLowHeightSpans;
	
	SampleTool* m_tool;
	SampleToolState* m_toolStates[MAX_TOOLS];
	
	BuildContext* m_ctx;

	SampleDebugDraw m_dd;
	
public:
	Sample();
	virtual ~Sample();
	
	void setContext(BuildContext* ctx) { m_ctx = ctx; }
	
	void setTool(SampleTool* tool);
	SampleToolState* getToolState(int type) { return m_toolStates[type]; }
	void setToolState(int type, SampleToolState* s) { m_toolStates[type] = s; }

	SampleDebugDraw& getDebugDraw() { return m_dd; }

	virtual void handleSettings();
	virtual void handleTools();
	virtual void handleDebugMode();
	virtual void handleClick(const float* s, const float* p, bool shift);
	virtual void handleToggle();
	virtual void handleStep();
	virtual void handleRender();
	virtual void handleRenderOverlay(double* proj, double* model, int* view);
	virtual void handleMeshChanged(class InputGeom* geom);
	virtual bool handleBuild();
	virtual void handleUpdate(const float dt);
	virtual void collectSettings(struct BuildSettings& settings);

	virtual class InputGeom* getInputGeom() { return m_geom; }
	virtual class dtNavMesh* getNavMesh() { return m_navMesh; }
	virtual class dtNavMeshQuery* getNavMeshQuery() { return m_navQuery; }
	virtual class dtCrowd* getCrowd() { return m_crowd; }
	virtual float getAgentRadius() { return m_agentRadius; }
	virtual float getAgentHeight() { return m_agentHeight; }
	virtual float getAgentClimb() { return m_agentMaxClimb; }
	
	unsigned char getNavMeshDrawFlags() const { return m_navMeshDrawFlags; }
	void setNavMeshDrawFlags(unsigned char flags) { m_navMeshDrawFlags = flags; }

	void updateToolStates(const float dt);
	void initToolStates(Sample* sample);
	void resetToolStates();
	void renderToolStates();
	void renderOverlayToolStates(double* proj, double* model, int* view);

	void resetCommonSettings();
	void handleCommonSettings();

private:
	// Explicitly disabled copy constructor and copy assignment operator.
	Sample(const Sample&);
	Sample& operator=(const Sample&);
};


#endif // RECASTSAMPLE_H
