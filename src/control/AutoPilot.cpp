#include "common.h"

#include "AutoPilot.h"

#include "CarCtrl.h"
#include "Curves.h"
#include "PathFind.h"
#include "SaveBuf.h"

void CAutoPilot::ModifySpeed(float speed)
{
	m_fMaxTrafficSpeed = Max(0.01f, speed);
	float positionBetweenNodes = (float)(CTimer::GetTimeInMilliseconds() - m_nTimeEnteredCurve) / m_nTimeToSpendOnCurrentCurve;
	CCarPathLink* pCurrentLink = &ThePaths.m_carPathLinks[m_nCurrentPathNodeInfo];
	CCarPathLink* pNextLink = &ThePaths.m_carPathLinks[m_nNextPathNodeInfo];
	float currentPathLinkForwardX = m_nCurrentDirection * ThePaths.m_carPathLinks[m_nCurrentPathNodeInfo].GetDirX();
	float currentPathLinkForwardY = m_nCurrentDirection * ThePaths.m_carPathLinks[m_nCurrentPathNodeInfo].GetDirY();
	float nextPathLinkForwardX = m_nNextDirection * ThePaths.m_carPathLinks[m_nNextPathNodeInfo].GetDirX();
	float nextPathLinkForwardY = m_nNextDirection * ThePaths.m_carPathLinks[m_nNextPathNodeInfo].GetDirY();
	CVector positionOnCurrentLinkIncludingLane(
		pCurrentLink->GetX() + ((m_nCurrentLane + 0.5f) * LANE_WIDTH) * currentPathLinkForwardY,
		pCurrentLink->GetY() - ((m_nCurrentLane + 0.5f) * LANE_WIDTH) * currentPathLinkForwardX,
		0.0f);
	CVector positionOnNextLinkIncludingLane(
		pNextLink->GetX() + ((m_nNextLane + 0.5f) * LANE_WIDTH) * nextPathLinkForwardY,
		pNextLink->GetY() - ((m_nNextLane + 0.5f) * LANE_WIDTH) * nextPathLinkForwardX,
		0.0f);
	m_nTimeToSpendOnCurrentCurve = CCurves::CalcSpeedScaleFactor(
		&positionOnCurrentLinkIncludingLane,
		&positionOnNextLinkIncludingLane,
		currentPathLinkForwardX, currentPathLinkForwardY,
		nextPathLinkForwardX, nextPathLinkForwardY
	) * (1000.0f / m_fMaxTrafficSpeed);
#ifdef FIX_BUGS
	/* Casting timer to float is very unwanted, and in this case even causes crashes. */
	m_nTimeEnteredCurve = CTimer::GetTimeInMilliseconds() -
		(uint32)(positionBetweenNodes * m_nTimeToSpendOnCurrentCurve);
#else
	m_nTimeEnteredCurve = CTimer::GetTimeInMilliseconds() - positionBetweenNodes * m_nTimeToSpendOnCurrentCurve;
#endif
}

void CAutoPilot::RemoveOnePathNode()
{
	--m_nPathFindNodesCount;
	for (int i = 0; i < m_nPathFindNodesCount; i++)
		m_aPathFindNodesInfo[i] = m_aPathFindNodesInfo[i + 1];
}

#ifdef COMPATIBLE_SAVES
void CAutoPilot::Save(uint8*& buf)
{
	WriteSaveBuf<int32>(buf, m_nCurrentRouteNode);
	WriteSaveBuf<int32>(buf, m_nNextRouteNode);
	WriteSaveBuf<int32>(buf, m_nPrevRouteNode);
	WriteSaveBuf<int32>(buf, m_nTimeEnteredCurve);
	WriteSaveBuf<int32>(buf, m_nTimeToSpendOnCurrentCurve);
	WriteSaveBuf<uint32>(buf, m_nCurrentPathNodeInfo);
	WriteSaveBuf<uint32>(buf, m_nNextPathNodeInfo);
	WriteSaveBuf<uint32>(buf, m_nPreviousPathNodeInfo);
	WriteSaveBuf<uint32>(buf, m_nAntiReverseTimer);
	WriteSaveBuf<uint32>(buf, m_nTimeToStartMission);
	WriteSaveBuf<int8>(buf, m_nPreviousDirection);
	WriteSaveBuf<int8>(buf, m_nCurrentDirection);
	WriteSaveBuf<int8>(buf, m_nNextDirection);
	WriteSaveBuf<int8>(buf, m_nCurrentLane);
	WriteSaveBuf<int8>(buf, m_nNextLane);
	WriteSaveBuf<uint8>(buf, m_nDrivingStyle);
	WriteSaveBuf<uint8>(buf, m_nCarMission);
	WriteSaveBuf<uint8>(buf, m_nTempAction);
	WriteSaveBuf<uint32>(buf, m_nTimeTempAction);
	WriteSaveBuf<float>(buf, m_fMaxTrafficSpeed);
	WriteSaveBuf<uint8>(buf, m_nCruiseSpeed);
	WriteSaveBuf<uint8>(buf, m_nCruiseSpeedMultiplierType);
	SkipSaveBuf(buf, 2);
	WriteSaveBuf<float>(buf, m_fCruiseSpeedMultiplier);
	uint8 flags = 0;
	if (m_bSlowedDownBecauseOfCars) flags |= BIT(0);
	if (m_bSlowedDownBecauseOfPeds) flags |= BIT(1);
	if (m_bStayInCurrentLevel) flags |= BIT(2);
	if (m_bStayInFastLane) flags |= BIT(3);
	if (m_bIgnorePathfinding) flags |= BIT(4);
	WriteSaveBuf<uint8>(buf, flags);
	WriteSaveBuf<uint8>(buf, m_nSwitchDistance);
	SkipSaveBuf(buf, 2);
	WriteSaveBuf<float>(buf, m_vecDestinationCoors.x);
	WriteSaveBuf<float>(buf, m_vecDestinationCoors.y);
	WriteSaveBuf<float>(buf, m_vecDestinationCoors.z);
	SkipSaveBuf(buf, 32);
	WriteSaveBuf<int16>(buf, m_nPathFindNodesCount);
	SkipSaveBuf(buf, 6);
}

void CAutoPilot::Load(uint8*& buf)
{
	ReadSaveBuf(&m_nCurrentRouteNode, buf);
	ReadSaveBuf(&m_nNextRouteNode, buf);
	ReadSaveBuf(&m_nPrevRouteNode, buf);
	ReadSaveBuf(&m_nTimeEnteredCurve, buf);
	ReadSaveBuf(&m_nTimeToSpendOnCurrentCurve, buf);
	ReadSaveBuf(&m_nCurrentPathNodeInfo, buf);
	ReadSaveBuf(&m_nNextPathNodeInfo, buf);
	ReadSaveBuf(&m_nPreviousPathNodeInfo, buf);
	ReadSaveBuf(&m_nAntiReverseTimer, buf);
	ReadSaveBuf(&m_nTimeToStartMission, buf);
	ReadSaveBuf(&m_nPreviousDirection, buf);
	ReadSaveBuf(&m_nCurrentDirection, buf);
	ReadSaveBuf(&m_nNextDirection, buf);
	ReadSaveBuf(&m_nCurrentLane, buf);
	ReadSaveBuf(&m_nNextLane, buf);
	ReadSaveBuf(&m_nDrivingStyle, buf);
	ReadSaveBuf(&m_nCarMission, buf);
	ReadSaveBuf(&m_nTempAction, buf);
	ReadSaveBuf(&m_nTimeTempAction, buf);
	ReadSaveBuf(&m_fMaxTrafficSpeed, buf);
	ReadSaveBuf(&m_nCruiseSpeed, buf);
	ReadSaveBuf(&m_nCruiseSpeedMultiplierType, buf);
	SkipSaveBuf(buf, 2);
	ReadSaveBuf(&m_fCruiseSpeedMultiplier, buf);
	uint8 flags;
	ReadSaveBuf(&flags, buf);
	m_bSlowedDownBecauseOfCars = !!(flags & BIT(0));
	m_bSlowedDownBecauseOfPeds = !!(flags & BIT(1));
	m_bStayInCurrentLevel = !!(flags & BIT(2));
	m_bStayInFastLane = !!(flags & BIT(3));
	m_bIgnorePathfinding = !!(flags & BIT(4));
	ReadSaveBuf(&m_nSwitchDistance, buf);
	SkipSaveBuf(buf, 2);
	ReadSaveBuf(&m_vecDestinationCoors.x, buf);
	ReadSaveBuf(&m_vecDestinationCoors.y, buf);
	ReadSaveBuf(&m_vecDestinationCoors.z, buf);
	SkipSaveBuf(buf, 32);
	ReadSaveBuf(&m_nPathFindNodesCount, buf);
	SkipSaveBuf(buf, 6);
}
#endif