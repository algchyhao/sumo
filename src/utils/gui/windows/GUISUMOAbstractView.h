#ifndef GUISUMOAbstractView_h
#define GUISUMOAbstractView_h
//---------------------------------------------------------------------------//
//                        GUISUMOAbstractView.h -
//  The base class for a view
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Sept 2002
//  copyright            : (C) 2002 by Daniel Krajzewicz
//  organisation         : IVF/DLR http://ivf.dlr.de
//  email                : Daniel.Krajzewicz@dlr.de
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//---------------------------------------------------------------------------//
// $Log$
// Revision 1.3  2004/12/15 09:20:19  dkrajzew
// made guisim independent of giant/netedit
//
// Revision 1.2  2004/12/12 17:23:59  agaubatz
// Editor Tool Widgets included
//
// Revision 1.1  2004/11/23 10:38:32  dkrajzew
// debugging
//
// Revision 1.3  2004/11/22 12:56:02  dksumo
// coloring of lanes by loaded weights added
//
// Revision 1.2  2004/10/29 06:01:56  dksumo
// renamed boundery to boundary
//
// Revision 1.1  2004/10/22 12:50:57  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.15  2004/08/02 11:54:52  dkrajzew
// added the possibility to take snapshots
//
// Revision 1.14  2004/07/02 08:32:10  dkrajzew
// changes due to the global object selection applied; some debugging (on zoom)
//
// Revision 1.13  2004/06/17 13:06:55  dkrajzew
// Polygon visualisation added
//
// Revision 1.12  2004/04/02 11:11:24  dkrajzew
// visualisation whether an item is selected added
//
// Revision 1.11  2004/03/19 12:54:08  dkrajzew
// porting to FOX
//
// Revision 1.10  2004/01/26 06:47:11  dkrajzew
// Added the possibility to draw arrows by a detector drawer; documentation added
//
// Revision 1.9  2003/11/12 14:07:46  dkrajzew
// clean up after recent changes
//
// Revision 1.8  2003/09/23 14:25:13  dkrajzew
// possibility to visualise detectors using different geometry complexities added
//
// Revision 1.7  2003/09/05 14:54:06  dkrajzew
// implementations of artefact drawers moved to folder "drawerimpl"
//
// Revision 1.6  2003/08/15 12:19:16  dkrajzew
// legend display patched
//
// Revision 1.5  2003/08/14 13:42:43  dkrajzew
// definition of the tls/row - drawer added
//
// Revision 1.4  2003/07/30 08:52:16  dkrajzew
// further work on visualisation of all geometrical objects
//
// Revision 1.3  2003/07/22 14:56:46  dkrajzew
// changes due to new detector handling
//
// Revision 1.2  2003/07/16 15:18:23  dkrajzew
// new interfaces for drawing classes; junction drawer interface added
//
// Revision 1.1  2003/05/20 09:25:13  dkrajzew
// new view hierarchy; some debugging done
//
// Revision 1.6  2003/04/16 09:50:05  dkrajzew
// centering of the network debugged; additional parameter of maximum display size added
//
// Revision 1.4  2003/04/02 11:50:28  dkrajzew
// a working tool tip implemented
//
// Revision 1.3  2003/02/07 10:34:15  dkrajzew
// files updated
//
/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <string>
#include <vector>
#include <fx.h>
#include <fx3d.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/Position2D.h>
#include <utils/gfx/RGBColor.h>
#include <utils/foxtools/FXMutex.h>
#include <utils/geom/Polygon2D.h>
#include <utils/gui/globjects/GUIGlObjectTypes.h>
#include "GUIGrid.h"

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#endif


/* =========================================================================
 * class declarations
 * ======================================================================= */
class GUIGLObjectToolTip;
class MSVehicle;
class GUINet;
class GUIGlChildWindow;
class GUIVehicle;
class GUILaneWrapper;
class GUIPerspectiveChanger;
class GUIMainWindow;
class GUIJunctionWrapper;
class GUIDetectorWrapper;
class GUIGLObjectPopupMenu;
class GUIGlObject;
class GUIGlObjectStorage;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class GUISUMOAbstractView
 * This class is meant to be pure virtual later;
 * It shall be the main class to inherit views of the simulation (micro-
 * or macroscopic ones) from it.
 */
class GUISUMOAbstractView : public FXGLCanvas {
  FXDECLARE(GUISUMOAbstractView)
public:
    /// constructor
    GUISUMOAbstractView(FXComposite *p, GUIMainWindow &app,
        GUIGlChildWindow *parent, const GUIGrid &grid,
        FXGLVisual *glVis);

    /// constructor
    GUISUMOAbstractView(FXComposite *p, GUIMainWindow &app,
        GUIGlChildWindow *parent, const GUIGrid &grid,
        FXGLVisual *glVis, FXGLCanvas *share);

    /// destructor
    virtual ~GUISUMOAbstractView();

    /// builds the view toolbars
    virtual void buildViewToolBars(GUIGlChildWindow &) { }

    /// recenters the view
    void recenterView();

    /// centers to the chosen artifact
    virtual void centerTo(GUIGlObject *o);

    /// meter-to-pixels conversion method
    double m2p(double meter);

    /// pixels-to-meters conversion method
    double p2m(double pixel);

    std::pair<double, double> canvas2World(double x, double y);

    /// Returns the information whether rotation is allowd
    bool allowRotation() const;

    /// Returns the gl-id of the object under the given coordinates
    void setTooltipPosition(size_t x, size_t y, size_t mouseX, size_t mouseY);

    /// A reimplementation due to some internal reasons
    FXbool makeCurrent();

	/// returns true, if the edit button was pressed
	bool isInEditMode();

    //GUINet &getNet() const;


    long onConfigure(FXObject*,FXSelector,void*);
    long onPaint(FXObject*,FXSelector,void*);
    virtual long onLeftBtnPress(FXObject*,FXSelector,void*);
    virtual long onLeftBtnRelease(FXObject*,FXSelector,void*);
    virtual long onRightBtnPress(FXObject*,FXSelector,void*);
    virtual long onRightBtnRelease(FXObject*,FXSelector,void*);
    virtual long onMouseMove(FXObject*,FXSelector,void*);
    virtual long onRightMouseTimeOut(FXObject*,FXSelector,void*);
    virtual long onCmdShowToolTips(FXObject*,FXSelector,void*);

    long onCmdShowGrid(FXObject*,FXSelector,void*);
    long onSimStep(FXObject*sender,FXSelector,void*);

    long onKeyPress(FXObject *o,FXSelector sel,void *data);
    long onKeyRelease(FXObject *o,FXSelector sel,void *data);



    /// A method that updates the tooltip
    void updateToolTip();

    /// Returns the maximum width of gl-windows
    int getMaxGLWidth() const;

    /// Returns the maximum height of gl-windows
    int getMaxGLHeight() const;

    /// paints the area to a buffer
    FXColor *getSnapshot();

    /// Builds the popup-menu containing the location-menu
    virtual FXPopup *getLocatorPopup(GUIGlChildWindow &p);

public:
    /**
     * VehicleColoringScheme
     * This enumeration holds the possible vehicle colouring schemes
     */
    enum VehicleColoringScheme {
        /// colouring by vehicle speed
        VCS_BY_SPEED = 0,
        /// use the colour specified in the input
        VCS_SPECIFIED = 1,
        /// use the type color
        VCS_TYPE = 2,
        /// use the route color
        VCS_ROUTE = 3,
        /// use random scheme 1
        VCS_RANDOM1 = 4,
        /// use random scheme 2
        VCS_RANDOM2 = 5,
        /// use lanechanging scheme 1
        VCS_LANECHANGE1 = 6,
        /// use lanechanging scheme 2
        VCS_LANECHANGE2 = 7,
        /// use lanechanging scheme 3
        VCS_LANECHANGE3 = 8,
        /// use waiting scheme 1
        VCS_WAITING1 = 9,
        /// use the route change offset
        VCS_ROUTECHANGEOFFSET = 10,
        /// use the route change offset
        VCS_ROUTECHANGENUMBER = 11,

        VCS_LANECHANGE4 = 12,

    };

    /**
     * LaneColoringScheme
     * This enumeration holds the possible lane colouring schemes
     */
    enum LaneColoringScheme {
        /// all lanes will be black
        LCS_BLACK = 0,
        /** colouring by purpose of the edge the lane lies in
            (sources:blue, sinks:red, normal:black) */
        LCS_BY_PURPOSE = 1,
        /// use the lane's speed
        LCS_BY_SPEED = 2,
        /// use the information whether the lane is selected or not
        LCS_BY_SELECTION = 3,
        /// aggregated views: use density
        LCS_BY_DENSITY = 4,
        /// aggregated views: use mean speed
        LCS_BY_MEAN_SPEED = 5,
        /// aggregated views: use mean halting duration
        LCS_BY_MEAN_HALTS = 6,
        /// all views: white (for better visualisation of other things)
        LCS_WHITE = 7,
        LCS_BY_LOADED_WEIGHTS = 8,

    };

    /**
     * JunctionColoringScheme
     * This enumeration holds the possible vehicle colouring schemes
     */
    enum JunctionColoringScheme {
        /// colouring by vehicle speed
        VCS_BY_TYPE = 0
    };

public:
    /**
     * @class ViewSettings
     * This class stores the viewport information for an easier checking whether
     *  it has changed.
     */
    class ViewSettings {
    public:
        /// Constructor
        ViewSettings();

        /// Parametrised Constructor
        ViewSettings(double x, double y, double xoff, double yoff);

        /// Destructor
        ~ViewSettings();

        /// Returns the information whether the stored setting differs from the given
        bool differ(double x, double y, double xoff, double yoff);

        /// Sets the setting information to the given values
        void set(double x, double y, double xoff, double yoff);

    private:
        /// Position and size information to describe the viewport
        double myX, myY, myXOff, myYOff;

    };

protected:

    /// performs the painting of the simulation
    void paintGL();

    /// Draws the given polygon
    void drawPolygon2D(Polygon2D &polygon);

protected:
    virtual void doPaintGL(int mode, double scale) { }

    virtual void doInit() { }

    /// paints a grid
    void paintGLGrid();

    /** applies the changes arised from window resize or movement */
    void applyChanges(double scale,
        size_t xoff, size_t yoff);

    /// draws the legend
    void displayLegend(bool flip=false);

    /// centers the view to the given position and size
//    void centerTo(Position2D pos, double radius);

    /// centers the given boundary
    void centerTo(Boundary bound);

    /// returns the id of the object under the cursor using GL_SELECT
    unsigned int getObjectUnderCursor();

    /// invokes the tooltip for the given object
    void showToolTipFor(unsigned int id);

    /// Clears the usetable, filling it with false
    void clearUsetable(size_t *_edges2Show, size_t _edges2ShowSize);

protected:
    /// The application
    GUIMainWindow *myApp;

    /// the parent window
    GUIGlChildWindow *_parent;

    /// the network used (stored here for a faster access)
    GUIGrid *myGrid;

    /// the sizes of the window
    int _widthInPixels, _heightInPixels;

    /// the scale of the net (the maximum size, either width or height)
    double myNetScale;

    /// information whether the grid shall be displayed
    bool _showGrid;

    /// The perspective changer
    GUIPerspectiveChanger *_changer;

    /// Information whether tool-tip informations shall be generated
    bool _useToolTips;

	bool _inEditMode;

    /// The used tooltip-class
    GUIGLObjectToolTip *_toolTip;

    /// position to display the tooltip at
    size_t _toolTipX, _toolTipY;

    /// The current mouse position (if the mouse is over this canvas)
    size_t _mouseX, _mouseY;

    /// Offset to the mouse-hotspot from the mouse position
    int _mouseHotspotX, _mouseHotspotY;

    /// A lock for drawing operations
    FXEX::FXMutex _lock;

    /// _widthInPixels / _heightInPixels
    double _ratio;

    /// Additional scaling factor for meters-to-pixels conversion
    double _addScl;

    /// The timer id
    int _timer;

    /// The reason (mouse state) of the timer
    int _timerReason;

    /// The current popup-menu
    GUIGLObjectPopupMenu *_popup;

    /// the description of the viewport
    ViewSettings myViewSettings;

    /// Internal information whether doInit() was called
    bool myAmInitialised;

    /// The locator menu
    FXPopup *myLocatorPopup;

    enum VehicleOperationType {
        VO_TRACK,
        VO_SHOW_ROUTE
    };

    struct VehicleOps {
        VehicleOperationType type;
        GUIVehicle *vehicle;
        int routeNo;
    };

    /// List of vehicles for which something has to be done with
    std::vector<VehicleOps> myVehicleOps;

    /**
     * A class to find the matching lane wrapper
     */
    class vehicle_in_ops_finder {
    public:
        /** constructor */
        explicit vehicle_in_ops_finder(const GUIVehicle * const v)
            : myVehicle(v) { }

        /** the comparing function */
        bool operator() (const VehicleOps &vo) {
            return vo.vehicle == myVehicle;
        }

    private:
        /// The vehicle to search for
        const GUIVehicle * const myVehicle;

    };

protected:
    GUISUMOAbstractView() { }

};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

#endif

// Local Variables:
// mode:C++
// End:

