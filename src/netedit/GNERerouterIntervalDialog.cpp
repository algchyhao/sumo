/****************************************************************************/
/// @file    GNERerouterIntervalDialog.cpp
/// @author  Pablo Alvarez Lopez
/// @date    eb 2017
/// @version $Id: GNERerouterIntervalDialog.cpp 22824 2017-02-02 09:51:02Z palcraft $
///
/// Dialog for edit rerouter intervals
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.dlr.de/
// Copyright (C) 2001-2017 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This file is part of SUMO.
//   SUMO is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 3 of the License, or
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

#include <iostream>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <utils/gui/div/GUIDesigns.h>

#include "GNERerouterIntervalDialog.h"
#include "GNERerouterDialog.h"
#include "GNERerouter.h"
#include "GNERerouterInterval.h"
#include "GNERerouterInterval.h"
#include "GNEClosingLaneReroute.h"
#include "GNEClosingReroute.h"
#include "GNEDestProbReroute.h"
#include "GNERouteProbReroute.h"
#include "GNEEdge.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif

// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNERerouterIntervalDialog) GNERerouterIntervalDialogMap[] = {
    FXMAPFUNC(SEL_DOUBLECLICKED,    MID_GNE_REROUTEDIALOG_CLOSINGLANEREORUTE,   GNERerouterIntervalDialog::onCmdDoubleClickedClosingLaneReroute),
    FXMAPFUNC(SEL_DOUBLECLICKED,    MID_GNE_REROUTEDIALOG_CLOSINGREROUTE,       GNERerouterIntervalDialog::onCmdDoubleClickedClosingReroute),
    FXMAPFUNC(SEL_DOUBLECLICKED,    MID_GNE_REROUTEDIALOG_DESTPROBREROUTE,      GNERerouterIntervalDialog::onCmdDoubleClickedDestProbReroute),
    FXMAPFUNC(SEL_DOUBLECLICKED,    MID_GNE_REROUTEDIALOG_ROUTEPROBREROUTE,     GNERerouterIntervalDialog::onCmdDoubleClickedRouteProbReroute),
    FXMAPFUNC(SEL_KEYRELEASE,       MID_GNE_REROUTEDIALOG_CLOSINGLANEREORUTE,   GNERerouterIntervalDialog::onCmdEditClosingLaneReroute),
    FXMAPFUNC(SEL_KEYRELEASE,       MID_GNE_REROUTEDIALOG_CLOSINGREROUTE,       GNERerouterIntervalDialog::onCmdEditClosingReroute),
    FXMAPFUNC(SEL_KEYRELEASE,       MID_GNE_REROUTEDIALOG_DESTPROBREROUTE,      GNERerouterIntervalDialog::onCmdEditDestProbReroute),
    FXMAPFUNC(SEL_KEYRELEASE,       MID_GNE_REROUTEDIALOG_ROUTEPROBREROUTE,     GNERerouterIntervalDialog::onCmdEditRouteProbReroute),
    FXMAPFUNC(SEL_COMMAND,          MID_GNE_REROUTEDIALOG_CHANGESTART,          GNERerouterIntervalDialog::onCmdChangeBegin),
    FXMAPFUNC(SEL_COMMAND,          MID_GNE_REROUTEDIALOG_CHANGEEND,            GNERerouterIntervalDialog::onCmdChangeEnd),
    FXMAPFUNC(SEL_COMMAND,          MID_GNE_MODE_ADDITIONALDIALOG_ACCEPT,       GNERerouterIntervalDialog::onCmdAccept),
    FXMAPFUNC(SEL_COMMAND,          MID_GNE_MODE_ADDITIONALDIALOG_CANCEL,       GNERerouterIntervalDialog::onCmdCancel),
    FXMAPFUNC(SEL_COMMAND,          MID_GNE_MODE_ADDITIONALDIALOG_RESET,        GNERerouterIntervalDialog::onCmdReset),
};

// Object implementation
FXIMPLEMENT(GNERerouterIntervalDialog, FXDialogBox, GNERerouterIntervalDialogMap, ARRAYNUMBER(GNERerouterIntervalDialogMap))

// ===========================================================================
// member method definitions
// ===========================================================================

GNERerouterIntervalDialog::GNERerouterIntervalDialog(GNERerouterDialog *rerouterDialog, GNERerouterInterval &rerouterInterval) :
    GNEAdditionalDialog(rerouterInterval.getRerouterParent(), 640, 480),
    myRerouterDialogParent(rerouterDialog),
    myRerouterInterval(&rerouterInterval),
    myBeginEndValid(true),
    myClosingLaneReroutesValid(true),
    myClosingReroutesValid(true),
    myDestProbReroutesValid(true),
    myRouteProbReroutesValid(true) {
    // change default header
    changeAdditionalDialogHeader(("Edit " + toString(rerouterInterval.getTag()) + " of " + toString(rerouterInterval.getRerouterParent()->getTag()) + 
                                  "'" + rerouterInterval.getRerouterParent()->getID() + "'").c_str());

    // Create auxiliar frames for tables
    FXHorizontalFrame *columns = new FXHorizontalFrame(myContentFrame, GUIDesignUniformHorizontalFrame);
    FXVerticalFrame *columnLeft = new FXVerticalFrame(columns, GUIDesignAuxiliarFrame);
    FXVerticalFrame *columnRight = new FXVerticalFrame(columns, GUIDesignAuxiliarFrame);

    // create horizontal frame for begin and end label
    FXHorizontalFrame *beginEndElementsLeft = new FXHorizontalFrame(columnLeft, GUIDesignAuxiliarHorizontalFrame);
    myBeginEndLabel = new FXLabel(beginEndElementsLeft, (toString(SUMO_ATTR_BEGIN) +" and " + toString(SUMO_ATTR_END) + " of " + toString(rerouterInterval.getTag())).c_str(), 0, GUIDesignLabelLeftThick);
    myCheckLabel = new FXLabel(beginEndElementsLeft, "", 0, GUIDesignLabelOnlyIcon);

    // create horizontal frame for begin and end text fields
    FXHorizontalFrame * beginEndElementsRight = new FXHorizontalFrame(columnRight, GUIDesignAuxiliarHorizontalFrame);
    myBeginTextField = new FXTextField(beginEndElementsRight, GUIDesignTextFieldNCol, this, MID_GNE_REROUTEDIALOG_CHANGESTART, GUIDesignTextFieldReal);
    myBeginTextField->setText(toString(myRerouterInterval->getBegin()).c_str());
    myEndTextField = new FXTextField(beginEndElementsRight, GUIDesignTextFieldNCol, this, MID_GNE_REROUTEDIALOG_CHANGEEND, GUIDesignTextFieldReal);
    myEndTextField->setText(toString(myRerouterInterval->getEnd()).c_str());

    // set interval flag depending if interval exists
    if(myRerouterDialogParent->findInterval(myRerouterInterval->getBegin(), myRerouterInterval->getEnd())) {
        myCheckLabel->setIcon(GUIIconSubSys::getIcon(ICON_CORRECT));
        myBeginEndValid = true;
    } else {
        myCheckLabel->setIcon(GUIIconSubSys::getIcon(ICON_ERROR));
        myBeginEndValid = false;
    }

    // Create labels and tables
    myClosingLaneReroutesLabel = new FXLabel(columnLeft, ("List of " + toString(SUMO_TAG_CLOSING_LANE_REROUTE) + "s").c_str(), 0, GUIDesignLabelThick);
    myClosingLaneRerouteList = new FXTable(columnLeft, this, MID_GNE_REROUTEDIALOG_CLOSINGLANEREORUTE, GUIDesignTableAdditionals);
    
    myCLosingReroutesLabel = new FXLabel(columnLeft, ("List of " + toString(SUMO_TAG_CLOSING_REROUTE) + "s").c_str(), 0, GUIDesignLabelThick);
    myClosingRerouteList = new FXTable(columnLeft, this, MID_GNE_REROUTEDIALOG_CLOSINGREROUTE, GUIDesignTableAdditionals);
    
    myDestProbReroutesLabel = new FXLabel(columnRight, ("List of " + toString(SUMO_TAG_DEST_PROB_REROUTE) + "s").c_str(), 0, GUIDesignLabelThick);
    myDestProbRerouteList = new FXTable(columnRight, this, MID_GNE_REROUTEDIALOG_DESTPROBREROUTE, GUIDesignTableAdditionals);
    
    myRouteProbReroutesLabel = new FXLabel(columnRight, ("List of " + toString(SUMO_TAG_ROUTE_PROB_REROUTE) + "s").c_str(), 0, GUIDesignLabelThick);
    myRouteProbReroute = new FXTable(columnRight, this, MID_GNE_REROUTEDIALOG_ROUTEPROBREROUTE, GUIDesignTableAdditionals);

    // copy Elements
    myCopyOfClosingLaneReroutes = myRerouterInterval->getClosingLaneReroutes();
    myCopyOfClosingReroutes = myRerouterInterval->getClosingReroutes();
    myCopyOfmyDestProbReroutes = myRerouterInterval->getDestProbReroutes();
    myCopyOfRouteProbReroutes = myRerouterInterval->getRouteProbReroutes();
    // update tables
    updateClosingLaneReroutesTable();
    updateClosingReroutesTable();
    updateDestProbReroutesTable();
    updateRouteProbReroutesTable();
}


GNERerouterIntervalDialog::~GNERerouterIntervalDialog() {
}


long
GNERerouterIntervalDialog::onCmdAccept(FXObject*, FXSelector, void*) {
    if(myBeginEndValid == false) {
        FXMessageBox::warning(getApp(), MBOX_OK, 
                              ("Error updating " + toString(myRerouterInterval->getTag()) + " of " + toString(SUMO_TAG_REROUTER)).c_str(), 
                              (toString(myRerouterInterval->getTag()) + " cannot be updated because " + toString(myRerouterInterval->getTag()) + " defined by " + 
                              toString(SUMO_ATTR_BEGIN) + " and " + toString(SUMO_ATTR_END) + " isn't valid").c_str());
        return 0;
    } else if(myCopyOfClosingLaneReroutes.empty() && myCopyOfClosingReroutes.empty() && myCopyOfmyDestProbReroutes.empty() && myCopyOfRouteProbReroutes.empty()) {
        FXMessageBox::warning(getApp(), MBOX_OK, 
                              ("Error updating " + toString(myRerouterInterval->getTag()) + " of " + toString(SUMO_TAG_REROUTER)).c_str(), 
                              (toString(myRerouterInterval->getTag()) + " cannot be updated because at least one " + toString(myRerouterInterval->getTag()) + " must be defined").c_str());
        return 0;
    } else if(myClosingLaneReroutesValid == false) {
        FXMessageBox::warning(getApp(), MBOX_OK,
                              ("Error updating " + toString(myRerouterInterval->getTag()) + " of " + toString(SUMO_TAG_REROUTER)).c_str(), 
                              (toString(myRerouterInterval->getTag()) + " cannot be updated because there are "+ toString(SUMO_TAG_CLOSING_LANE_REROUTE) + "s invalid").c_str());
        return 0;
    } else if(myClosingReroutesValid == false) {
        FXMessageBox::warning(getApp(), MBOX_OK,
                              ("Error updating " + toString(myRerouterInterval->getTag()) + " of " + toString(SUMO_TAG_REROUTER)).c_str(), 
                              (toString(myRerouterInterval->getTag()) + " cannot be updated because there are "+ toString(SUMO_TAG_CLOSING_REROUTE) + "s invalid").c_str());
        return 0;
    } else if(myDestProbReroutesValid == false) {
        FXMessageBox::warning(getApp(), MBOX_OK,
                              ("Error updating " + toString(myRerouterInterval->getTag()) + " of " + toString(SUMO_TAG_REROUTER)).c_str(), 
                              (toString(myRerouterInterval->getTag()) + " cannot be updated because there "+ toString(SUMO_TAG_DEST_PROB_REROUTE) + "s invalid").c_str());
        return 0;
    } else if(myRouteProbReroutesValid == false) {
        FXMessageBox::warning(getApp(), MBOX_OK,
                              ("Error updating " + toString(myRerouterInterval->getTag()) + " of " + toString(SUMO_TAG_REROUTER)).c_str(), 
                              (toString(myRerouterInterval->getTag()) + " cannot be updated because there are "+ toString(SUMO_TAG_ROUTE_PROB_REROUTE) + "s invalid").c_str());
        return 0;
    } else{
        // set new values of rerouter interval
        myRerouterInterval->setClosingLaneReroutes(myCopyOfClosingLaneReroutes);
        myRerouterInterval->setClosingReroutes(myCopyOfClosingReroutes);
        myRerouterInterval->setDestProbReroutes(myCopyOfmyDestProbReroutes);
        myRerouterInterval->setRouteProbReroutes(myCopyOfRouteProbReroutes);
        myRerouterInterval->setBegin(GNEAttributeCarrier::parse<SUMOReal>(myBeginTextField->getText().text()));
        myRerouterInterval->setEnd(GNEAttributeCarrier::parse<SUMOReal>(myEndTextField->getText().text()));

        // Stop Modal
        getApp()->stopModal(this, TRUE);
        return 1;
    }
}


long
GNERerouterIntervalDialog::onCmdCancel(FXObject*, FXSelector, void*) {
    // Stop Modal
    getApp()->stopModal(this, FALSE);
    return 1;
}


long
GNERerouterIntervalDialog::onCmdReset(FXObject*, FXSelector, void*) {
    // Copy original intervals again
    myCopyOfClosingLaneReroutes = myRerouterInterval->getClosingLaneReroutes();
    myCopyOfClosingReroutes = myRerouterInterval->getClosingReroutes();
    myCopyOfmyDestProbReroutes = myRerouterInterval->getDestProbReroutes();
    myCopyOfRouteProbReroutes = myRerouterInterval->getRouteProbReroutes();
    // update tables
    updateClosingLaneReroutesTable();
    updateClosingReroutesTable();
    updateDestProbReroutesTable();
    updateRouteProbReroutesTable();
    return 1;
}


long 
GNERerouterIntervalDialog::onCmdDoubleClickedClosingLaneReroute(FXObject*, FXSelector, void*) {
    // First check if list is emty
    if(myClosingLaneRerouteList->getNumRows() > 0) {
        // check if add button was pressed
        if(myClosingLaneRerouteList->getItem((int)myCopyOfClosingLaneReroutes.size(), 4)->hasFocus()) {
            // add new element and update table
            myCopyOfClosingLaneReroutes.push_back(GNEClosingLaneReroute(*myRerouterInterval, NULL, std::vector<SUMOVehicleClass>(), std::vector<SUMOVehicleClass>()));
            updateClosingLaneReroutesTable();
            return 1;
        } else {
            // check if some delete button was pressed
            for(int i = 0; i < (int)myCopyOfClosingLaneReroutes.size(); i++) {
                if(myClosingLaneRerouteList->getItem(i, 4)->hasFocus()) {
                    myClosingLaneRerouteList->removeRows(i);
                    myCopyOfClosingLaneReroutes.erase(myCopyOfClosingLaneReroutes.begin() + i);
                    return 1;
                }
            }
            return 1;
        }
    } else {
        return 0;
    }
}


long 
GNERerouterIntervalDialog::onCmdDoubleClickedClosingReroute(FXObject*, FXSelector, void*) {
    // First check if list is emty
    if(myClosingRerouteList->getNumRows() > 0) {
        // check if add button was pressed
        if(myClosingRerouteList->getItem((int)myCopyOfClosingReroutes.size(), 4)->hasFocus()) {
            // add new element and update table
            myCopyOfClosingReroutes.push_back(GNEClosingReroute(*myRerouterInterval, NULL, std::vector<SUMOVehicleClass>(), std::vector<SUMOVehicleClass>()));
            updateClosingReroutesTable();
            return 1;
        } else {
            // check if some delete button was pressed
            for(int i = 0; i < (int)myCopyOfClosingReroutes.size(); i++) {
                if(myClosingRerouteList->getItem(i, 4)->hasFocus()) {
                    myClosingRerouteList->removeRows(i);
                    myCopyOfClosingReroutes.erase(myCopyOfClosingReroutes.begin() + i);
                    return 1;
                }
            }
            return 1;
        }
    } else {
        return 0;
    }
}


long 
GNERerouterIntervalDialog::onCmdDoubleClickedDestProbReroute(FXObject*, FXSelector, void*) {
    // First check if list is emty
    if(myDestProbRerouteList->getNumRows() > 0) {
        // check if add button was pressed
        if(myDestProbRerouteList->getItem((int)myCopyOfmyDestProbReroutes.size(), 3)->hasFocus()) {
            // add new element and update table
            myCopyOfmyDestProbReroutes.push_back(GNEDestProbReroute(*myRerouterInterval, NULL, 0));
            updateDestProbReroutesTable();
            return 1;
        } else {
            // check if some delete button was pressed
            for(int i = 0; i < (int)myCopyOfmyDestProbReroutes.size(); i++) {
                if(myDestProbRerouteList->getItem(i, 3)->hasFocus()) {
                    myDestProbRerouteList->removeRows(i);
                    myCopyOfmyDestProbReroutes.erase(myCopyOfmyDestProbReroutes.begin() + i);
                    return 1;
                }
            }
            return 1;
        }
    } else {
        return 0;
    }
}


long 
GNERerouterIntervalDialog::onCmdDoubleClickedRouteProbReroute(FXObject*, FXSelector, void*) {
    // First check if list is emty
    if(myRouteProbReroute->getNumRows() > 0) {
        // check if add button was pressed
        if(myRouteProbReroute->getItem((int)myCopyOfRouteProbReroutes.size(), 3)->hasFocus()) {
            // add new element and update table
            myCopyOfRouteProbReroutes.push_back(GNERouteProbReroute(*myRerouterInterval,"", 0));
            updateRouteProbReroutesTable();
            return 1;
        } else {
            // check if some delete button was pressed
            for(int i = 0; i < (int)myCopyOfRouteProbReroutes.size(); i++) {
                if(myRouteProbReroute->getItem(i, 3)->hasFocus()) {
                    myRouteProbReroute->removeRows(i);
                    myCopyOfRouteProbReroutes.erase(myCopyOfRouteProbReroutes.begin() + i);
                    return 1;
                }
            }
            return 1;
        }
    } else {
        return 0;
    }
}


long 
GNERerouterIntervalDialog::onCmdEditClosingLaneReroute(FXObject*, FXSelector, void*) {
    std::cout << "EDITA 1" << std::endl;
    return 1;
}


long 
GNERerouterIntervalDialog::onCmdEditClosingReroute(FXObject*, FXSelector, void*) {
    std::cout << "EDITA 2" << std::endl;
    return 1;
}


long 
GNERerouterIntervalDialog::onCmdEditDestProbReroute(FXObject*, FXSelector, void*) {
    std::cout << "EDITA 3" << std::endl;
    return 1;
}


long 
GNERerouterIntervalDialog::onCmdEditRouteProbReroute(FXObject*, FXSelector, void*) {
    std::cout << "EDITA 4" << std::endl;
    return 1;
}


long 
GNERerouterIntervalDialog::onCmdChangeBegin(FXObject*, FXSelector, void*) {
    if(GNEAttributeCarrier::canParse<SUMOReal>(myBeginTextField->getText().text()) && 
        myRerouterDialogParent->checkInterval(GNEAttributeCarrier::parse<SUMOReal>(myBeginTextField->getText().text()), myRerouterInterval->getEnd())) {
         myBeginEndValid = true;
         myCheckLabel->setIcon(GUIIconSubSys::getIcon(ICON_CORRECT));
         return 1;
    }
    else {  
        myBeginEndValid = false;
        myCheckLabel->setIcon(GUIIconSubSys::getIcon(ICON_ERROR));
        return 0;
    }
}


long 
GNERerouterIntervalDialog::onCmdChangeEnd(FXObject*, FXSelector, void*) {
    if(GNEAttributeCarrier::canParse<SUMOReal>(myEndTextField->getText().text()) && 
        myRerouterDialogParent->checkInterval(myRerouterInterval->getBegin(), GNEAttributeCarrier::parse<SUMOReal>(myEndTextField->getText().text()))) {
         myBeginEndValid = true;
         myCheckLabel->setIcon(GUIIconSubSys::getIcon(ICON_CORRECT));
         return 1;
    }
    else {  
        myBeginEndValid = false;
        myCheckLabel->setIcon(GUIIconSubSys::getIcon(ICON_ERROR));
        return 0;
    }
}


void 
GNERerouterIntervalDialog::updateClosingLaneReroutesTable() {
    // clear table
    myClosingLaneRerouteList->clearItems();
    // set number of rows
    myClosingLaneRerouteList->setTableSize(int(myCopyOfClosingLaneReroutes.size()) + 1, 5);
    // Configure list
    myClosingLaneRerouteList->setVisibleColumns(5);
    myClosingLaneRerouteList->setColumnWidth(0, 83);
    myClosingLaneRerouteList->setColumnWidth(1, 83);
    myClosingLaneRerouteList->setColumnWidth(2, 83);
    myClosingLaneRerouteList->setColumnWidth(3, GUIDesignTableIconCellWidth);
    myClosingLaneRerouteList->setColumnWidth(4, GUIDesignTableIconCellWidth);
    myClosingLaneRerouteList->setColumnText(0, toString(SUMO_ATTR_LANE).c_str());
    myClosingLaneRerouteList->setColumnText(1, toString(SUMO_ATTR_ALLOW).c_str());
    myClosingLaneRerouteList->setColumnText(2, toString(SUMO_ATTR_DISALLOW).c_str());
    myClosingLaneRerouteList->setColumnText(3, "");
    myClosingLaneRerouteList->setColumnText(4, "");
    myClosingLaneRerouteList->getRowHeader()->setWidth(0);
    // Declare index for rows and pointer to FXTableItem
    int indexRow = 0;
    FXTableItem* item = 0;
    // iterate over values
    for (std::vector<GNEClosingLaneReroute>::iterator i = myCopyOfClosingLaneReroutes.begin(); i != myCopyOfClosingLaneReroutes.end(); i++) {
        // Set closing edge
        if(i->getClosedEdge() != NULL) {
            item = new FXTableItem(i->getClosedEdge()->getID().c_str());
            myClosingLaneRerouteList->setItem(indexRow, 0, item);
        } else {
            item = new FXTableItem("");
            myClosingLaneRerouteList->setItem(indexRow, 0, item);
        }
        // Set allow
        // @todo item = new FXTableItem(toString((*i)->getEnd()).c_str());
        // @todo myClosingLaneRerouteList->setItem(indexRow, 1, item);
        // Set disallow
        // @todo item = new FXTableItem(toString((*i)->getEnd()).c_str());
        // @todo myClosingLaneRerouteList->setItem(indexRow, 2, item);
        // set valid
        item = new FXTableItem("", GUIIconSubSys::getIcon(ICON_CORRECT));
        item->setJustify(FXTableItem::CENTER_X | FXTableItem::CENTER_Y);
        item->setEnabled(false);
        myClosingLaneRerouteList->setItem(indexRow, 3, item);
        // set remove
        item = new FXTableItem("", GUIIconSubSys::getIcon(ICON_REMOVE));
        item->setJustify(FXTableItem::CENTER_X | FXTableItem::CENTER_Y);
        item->setEnabled(false);
        myClosingLaneRerouteList->setItem(indexRow, 4, item);
        // Update index
        indexRow++;
    }
    // set add
    item = new FXTableItem("");
    item->setEnabled(false);
    myClosingLaneRerouteList->setItem(indexRow, 0, item);
    item = new FXTableItem("");
    item->setEnabled(false);
    myClosingLaneRerouteList->setItem(indexRow, 1, item);
    item = new FXTableItem("");
    item->setEnabled(false);
    myClosingLaneRerouteList->setItem(indexRow, 2, item);
    item = new FXTableItem("");
    item->setEnabled(false);
    myClosingLaneRerouteList->setItem(indexRow, 3, item);
    item = new FXTableItem("", GUIIconSubSys::getIcon(ICON_ADD));
    item->setJustify(FXTableItem::CENTER_X | FXTableItem::CENTER_Y);
    item->setEnabled(false);
    myClosingLaneRerouteList->setItem(indexRow, 4, item);
}


void 
GNERerouterIntervalDialog::updateClosingReroutesTable() {
    // clear table
    myClosingRerouteList->clearItems();
    // set number of rows
    myClosingRerouteList->setTableSize(int(myCopyOfClosingReroutes.size()) + 1, 5);
    // Configure list
    myClosingRerouteList->setVisibleColumns(5);
    myClosingRerouteList->setColumnWidth(0, 83);
    myClosingRerouteList->setColumnWidth(1, 83);
    myClosingRerouteList->setColumnWidth(2, 83);
    myClosingRerouteList->setColumnWidth(3, GUIDesignTableIconCellWidth);
    myClosingRerouteList->setColumnWidth(4, GUIDesignTableIconCellWidth);
    myClosingRerouteList->setColumnText(0, toString(SUMO_ATTR_EDGE).c_str());
    myClosingRerouteList->setColumnText(1, toString(SUMO_ATTR_ALLOW).c_str());
    myClosingRerouteList->setColumnText(2, toString(SUMO_ATTR_DISALLOW).c_str());
    myClosingRerouteList->setColumnText(3, "");
    myClosingRerouteList->setColumnText(4, "");
    myClosingRerouteList->getRowHeader()->setWidth(0);
    // Declare index for rows and pointer to FXTableItem
    int indexRow = 0;
    FXTableItem* item = 0;
    // iterate over values
    for (std::vector<GNEClosingReroute>::iterator i = myCopyOfClosingReroutes.begin(); i != myCopyOfClosingReroutes.end(); i++) {
        // Set closing edge
        if(i->getClosedEdge() != NULL) {
            item = new FXTableItem(i->getClosedEdge()->getID().c_str());
            myClosingRerouteList->setItem(indexRow, 0, item);
        } else {
            item = new FXTableItem("");
            myClosingRerouteList->setItem(indexRow, 0, item);
        }
        // Set allow
        // @todo item = new FXTableItem(toString((*i)->getEnd()).c_str());
        // @todo myClosingRerouteList->setItem(indexRow, 1, item);
        // Set disallow
        // @todo item = new FXTableItem(toString((*i)->getEnd()).c_str());
        // @todo myClosingRerouteList->setItem(indexRow, 1, item);
        // set valid
        item = new FXTableItem("", GUIIconSubSys::getIcon(ICON_CORRECT));
        item->setJustify(FXTableItem::CENTER_X | FXTableItem::CENTER_Y);
        item->setEnabled(false);
        myClosingRerouteList->setItem(indexRow, 3, item);
        // set remove
        item = new FXTableItem("", GUIIconSubSys::getIcon(ICON_REMOVE));
        item->setJustify(FXTableItem::CENTER_X | FXTableItem::CENTER_Y);
        item->setEnabled(false);
        myClosingRerouteList->setItem(indexRow, 4, item);
        // Update index
        indexRow++;
    }
    // set add
    item = new FXTableItem("");
    item->setEnabled(false);
    myClosingRerouteList->setItem(indexRow, 0, item);
    item = new FXTableItem("");
    item->setEnabled(false);
    myClosingRerouteList->setItem(indexRow, 1, item);
    item = new FXTableItem("");
    item->setEnabled(false);
    myClosingRerouteList->setItem(indexRow, 2, item);
    item = new FXTableItem("");
    item->setEnabled(false);
    myClosingRerouteList->setItem(indexRow, 3, item);
    item = new FXTableItem("", GUIIconSubSys::getIcon(ICON_ADD));
    item->setJustify(FXTableItem::CENTER_X | FXTableItem::CENTER_Y);
    item->setEnabled(false);
    myClosingRerouteList->setItem(indexRow, 4, item);
}


void 
GNERerouterIntervalDialog::updateDestProbReroutesTable() {
    // clear table
    myDestProbRerouteList->clearItems();
    // set number of rows
    myDestProbRerouteList->setTableSize(int(myCopyOfmyDestProbReroutes.size()) + 1, 4);
    // Configure list
    myDestProbRerouteList->setVisibleColumns(4);
    myDestProbRerouteList->setColumnWidth(0, 125);
    myDestProbRerouteList->setColumnWidth(1, 124);
    myDestProbRerouteList->setColumnWidth(2, GUIDesignTableIconCellWidth);
    myDestProbRerouteList->setColumnWidth(3, GUIDesignTableIconCellWidth);
    myDestProbRerouteList->setColumnText(0, toString(SUMO_ATTR_EDGE).c_str());
    myDestProbRerouteList->setColumnText(1, toString(SUMO_ATTR_PROB).c_str());
    myDestProbRerouteList->setColumnText(2, "");
    myDestProbRerouteList->setColumnText(3, "");
    myDestProbRerouteList->getRowHeader()->setWidth(0);
    // Declare index for rows and pointer to FXTableItem
    int indexRow = 0;
    FXTableItem* item = 0;
    // iterate over values
    for (std::vector<GNEDestProbReroute>::iterator i = myCopyOfmyDestProbReroutes.begin(); i != myCopyOfmyDestProbReroutes.end(); i++) {
        // Set new destination
        if(i->getNewDestination() != NULL) {
            item = new FXTableItem(i->getNewDestination()->getID().c_str());
            myDestProbRerouteList->setItem(indexRow, 0, item);
        } else {
            item = new FXTableItem("");
            myDestProbRerouteList->setItem(indexRow, 0, item);
        }
        // Set probability
        item = new FXTableItem(toString(i->getProbability()).c_str());
        myDestProbRerouteList->setItem(indexRow, 1, item);
        // set valid
        item = new FXTableItem("", GUIIconSubSys::getIcon(ICON_CORRECT));
        item->setJustify(FXTableItem::CENTER_X | FXTableItem::CENTER_Y);
        item->setEnabled(false);
        myDestProbRerouteList->setItem(indexRow, 2, item);
        // set remove
        item = new FXTableItem("", GUIIconSubSys::getIcon(ICON_REMOVE));
        item->setJustify(FXTableItem::CENTER_X | FXTableItem::CENTER_Y);
        item->setEnabled(false);
        myDestProbRerouteList->setItem(indexRow, 3, item);
        // Update index
        indexRow++;
    }
    // set add
    item = new FXTableItem("");
    item->setEnabled(false);
    myDestProbRerouteList->setItem(indexRow, 0, item);
    item = new FXTableItem("");
    item->setEnabled(false);
    myDestProbRerouteList->setItem(indexRow, 1, item);
    item = new FXTableItem("");
    item->setEnabled(false);
    myDestProbRerouteList->setItem(indexRow, 2, item);
    item = new FXTableItem("", GUIIconSubSys::getIcon(ICON_ADD));
    item->setJustify(FXTableItem::CENTER_X | FXTableItem::CENTER_Y);
    item->setEnabled(false);
    myDestProbRerouteList->setItem(indexRow, 3, item);
}


void 
GNERerouterIntervalDialog::updateRouteProbReroutesTable() {
    // clear table
    myRouteProbReroute->clearItems();
    // set number of rows
    myRouteProbReroute->setTableSize(int(myCopyOfRouteProbReroutes.size()) + 1, 4);
    // Configure list
    myRouteProbReroute->setVisibleColumns(4);
    myRouteProbReroute->setColumnWidth(0, 125);
    myRouteProbReroute->setColumnWidth(1, 124);
    myRouteProbReroute->setColumnWidth(2, GUIDesignTableIconCellWidth);
    myRouteProbReroute->setColumnWidth(3, GUIDesignTableIconCellWidth);
    myRouteProbReroute->setColumnText(0, toString(SUMO_ATTR_ROUTE).c_str());
    myRouteProbReroute->setColumnText(1, toString(SUMO_ATTR_PROB).c_str());
    myRouteProbReroute->setColumnText(2, "");
    myRouteProbReroute->setColumnText(3, "");
    myRouteProbReroute->getRowHeader()->setWidth(0);
    // Declare index for rows and pointer to FXTableItem
    int indexRow = 0;
    FXTableItem* item = 0;
    // iterate over values
    for (std::vector<GNERouteProbReroute>::iterator i = myCopyOfRouteProbReroutes.begin(); i != myCopyOfRouteProbReroutes.end(); i++) {
        // Set new route
        item = new FXTableItem(i->getNewRouteId().c_str());
        myRouteProbReroute->setItem(indexRow, 0, item);
        // Set probability
        item = new FXTableItem(toString(i->getProbability()).c_str());
        myRouteProbReroute->setItem(indexRow, 1, item);
        // set valid
        item = new FXTableItem("", GUIIconSubSys::getIcon(ICON_CORRECT));
        item->setJustify(FXTableItem::CENTER_X | FXTableItem::CENTER_Y);
        item->setEnabled(false);
        myRouteProbReroute->setItem(indexRow, 2, item);
        // set remove
        item = new FXTableItem("", GUIIconSubSys::getIcon(ICON_REMOVE));
        item->setJustify(FXTableItem::CENTER_X | FXTableItem::CENTER_Y);
        item->setEnabled(false);
        myRouteProbReroute->setItem(indexRow, 3, item);
        // Update index
        indexRow++;
    }
    // set add
    item = new FXTableItem("");
    item->setEnabled(false);
    myRouteProbReroute->setItem(indexRow, 0, item);
    item = new FXTableItem("");
    item->setEnabled(false);
    myRouteProbReroute->setItem(indexRow, 1, item);
    item = new FXTableItem("");
    item->setEnabled(false);
    myRouteProbReroute->setItem(indexRow, 2, item);
    item = new FXTableItem("", GUIIconSubSys::getIcon(ICON_ADD));
    item->setJustify(FXTableItem::CENTER_X | FXTableItem::CENTER_Y);
    item->setEnabled(false);
    myRouteProbReroute->setItem(indexRow, 3, item);
}


/****************************************************************************/
