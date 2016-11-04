/****************************************************************************/
/// @file    MSStoppingPlace.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Mon, 13.12.2005
/// @version $Id: MSStoppingPlace.cpp 19388 2015-11-19 21:33:01Z behrisch $
///
// A lane area vehicles can halt at
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.dlr.de/
// Copyright (C) 2005-2015 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This file is part of SUMO.
//   SUMO is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <cassert>
#include <utils/vehicle/SUMOVehicle.h>
#include <utils/geom/Position.h>
#include <microsim/MSVehicleType.h>
#include <utils/foxtools/MFXMutex.h>
#include "MSLane.h"
#include "MSTransportable.h"
#include "MSParkingArea.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// method definitions
// ===========================================================================
MSParkingArea::MSParkingArea(const std::string& id,
                             const std::vector<std::string>& lines,
                             MSLane& lane,
                             SUMOReal begPos, SUMOReal endPos,
							 unsigned int capacity,
							 SUMOReal width, SUMOReal length, SUMOReal angle)
    : MSStoppingPlace(id, lines, lane, begPos, endPos), myCapacity(capacity), myWidth(width), myLength(length), myAngle(angle) {
		
	// initialize space occupancies if there is a capacity
	if (myCapacity > 0) {
		for (unsigned int i = 1; i <= myCapacity; ++i) {
			mySpaceOccupancies[i] = LotSpaceDefinition();
			mySpaceOccupancies[i].index = i;
			mySpaceOccupancies[i].vehicle = 0;
			mySpaceOccupancies[i].myFGPosition = Position(0.,0.,0.);
			mySpaceOccupancies[i].myFGWidth = width;
			mySpaceOccupancies[i].myFGLength = length;
			mySpaceOccupancies[i].myFGRotation = angle;
		}
	}

    computeLastFreePos();
}

MSParkingArea::~MSParkingArea() {}

SUMOReal
MSParkingArea::getLastFreePos(const SUMOVehicle& forVehicle) const {
    if (myLastFreePos == myBegPos) {
        return myLastFreePos - forVehicle.getVehicleType().getMinGap();
    }
    return myLastFreePos;
}

Position
MSParkingArea::getVehiclePosition(const SUMOVehicle& forVehicle) {
    std::map<unsigned int, LotSpaceDefinition >::iterator i;
    for (i = mySpaceOccupancies.begin(); i != mySpaceOccupancies.end(); i++) {
        if ((*i).second.vehicle == &forVehicle) { 
			Position pos = (*i).second.myFGPosition;
			SUMOReal len = forVehicle.getVehicleType().getLength() / 2.;
			SUMOReal angle = (((*i).second.myFGRotation - 90.) * (SUMOReal) PI / (SUMOReal) 180.0);
			return Position(pos.x() + len * cos(angle), pos.y() + len * sin(angle));
        }
    }
    return Position::INVALID;
}

SUMOReal
MSParkingArea::getVehicleAngle(const SUMOVehicle& forVehicle) {
    std::map<unsigned int, LotSpaceDefinition >::iterator i;
    for (i = mySpaceOccupancies.begin(); i != mySpaceOccupancies.end(); i++) {
        if ((*i).second.vehicle == &forVehicle) {  
			return (((*i).second.myFGRotation - 90.) * (SUMOReal) PI / (SUMOReal) 180.0);
        }
    }
    return 0.;
}


SUMOReal
MSParkingArea::getSpaceDim() const {
    return ((myEndPos - myBegPos) / myCapacity);
}

bool
MSParkingArea::addLotEntry(SUMOReal x, SUMOReal y, SUMOReal z,
						   SUMOReal width, SUMOReal length, SUMOReal angle) {

	unsigned int i = mySpaceOccupancies.size() + 1;

	mySpaceOccupancies[i] = LotSpaceDefinition();
	mySpaceOccupancies[i].index = i;
	mySpaceOccupancies[i].vehicle = 0;
	mySpaceOccupancies[i].myFGPosition = Position(x, y, z);
	mySpaceOccupancies[i].myFGWidth = width;
	mySpaceOccupancies[i].myFGLength = length;
	mySpaceOccupancies[i].myFGRotation = angle;

	myCapacity = mySpaceOccupancies.size();

	return true;
}

void
MSParkingArea::enter(SUMOVehicle* what, SUMOReal beg, SUMOReal end) {
	if (myLastFreeLot >= 1 && myLastFreeLot <= mySpaceOccupancies.size()) {
		mySpaceOccupancies[myLastFreeLot].vehicle = what;
		myEndPositions[what] = std::pair<SUMOReal, SUMOReal>(beg, end);
		computeLastFreePos();
	}
}


void
MSParkingArea::leaveFrom(SUMOVehicle* what) {
    assert(myEndPositions.find(what) != myEndPositions.end());
    std::map<unsigned int, LotSpaceDefinition >::iterator i;
    for (i = mySpaceOccupancies.begin(); i != mySpaceOccupancies.end(); i++) {
        if ((*i).second.vehicle == what) {  
			(*i).second.vehicle = 0;
			break;
        }
    }
    myEndPositions.erase(myEndPositions.find(what));
    computeLastFreePos();
}


void
MSParkingArea::computeLastFreePos() {
	myLastFreeLot = 1;
	myLastFreePos = myBegPos;
    std::map<unsigned int, LotSpaceDefinition >::iterator i;
    for (i = mySpaceOccupancies.begin(); i != mySpaceOccupancies.end(); i++) {
        if ((*i).second.vehicle == 0) {
			myLastFreeLot = (*i).first;
			myLastFreePos = myBegPos + getSpaceDim() * ((*i).first);
			break;
        }
    }
}


SUMOReal
MSParkingArea::getWidth() const {
    return myWidth;
}


SUMOReal
MSParkingArea::getLength() const {
    return myLength;
}


SUMOReal
MSParkingArea::getAngle() const {
    return myAngle;
}


unsigned int
MSParkingArea::getCapacity() const {
    return myCapacity;
}


unsigned int
MSParkingArea::getOccupancy() const {
    return myEndPositions.size();
}


/****************************************************************************/
