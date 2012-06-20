/**
 * @file llpanelenavmeshrebake.h
 * @author prep
 * @brief handles the buttons for navmesh rebaking
 *
 * $LicenseInfo:firstyear=2002&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#ifndef LL_NAVMESHREBAKE_H
#define LL_NAVMESHREBAKE_H

#include "llhandle.h"
#include "llpanel.h"

class LLButton;
class LLView;

class LLPanelNavMeshRebake : public LLPanel
{

	LOG_CLASS(LLPanelNavMeshRebake);

public:

	typedef enum
	{
		kRebakeNavMesh_Available,
		kRebakeNavMesh_RequestSent,
		kRebakeNavMesh_NotAvailable,
		kRebakeNavMesh_Default = kRebakeNavMesh_NotAvailable
	} ERebakeNavMeshMode;

	static LLPanelNavMeshRebake* getInstance();

	void setMode(ERebakeNavMeshMode pRebakeNavMeshMode);
	
	virtual BOOL postBuild();

	virtual void draw();
	virtual BOOL handleToolTip( S32 x, S32 y, MASK mask );

protected:

private:
	LLPanelNavMeshRebake();
	virtual ~LLPanelNavMeshRebake();

	static LLPanelNavMeshRebake* getPanel();

	void onNavMeshRebakeClick();
	void updatePosition();

	LLButton* mNavMeshRebakeButton;
	LLButton* mNavMeshBakingButton;
};

#endif //LL_NAVMESHREBAKE_H
