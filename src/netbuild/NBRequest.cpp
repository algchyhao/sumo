/***************************************************************************
                          NBRequest.cpp
			  This class computes the logic of a junction
                             -------------------
    project              : SUMO
    subproject           : netbuilder / netconverter
    begin                : Tue, 20 Nov 2001
    copyright            : (C) 2001 by DLR http://ivf.dlr.de/
    author               : Daniel Krajzewicz
    email                : Daniel.Krajzewicz@dlr.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.12  2003/05/20 09:33:48  dkrajzew
// false computation of yielding on lane ends debugged; some debugging on tl-import; further work on vissim-import
//
// Revision 1.11  2003/04/16 10:03:48  dkrajzew
// further work on Vissim-import
//
// Revision 1.10  2003/04/14 08:34:59  dkrajzew
// some further bugs removed
//
// Revision 1.9  2003/04/10 15:45:20  dkrajzew
// some lost changes reapplied
//
// Revision 1.8  2003/04/07 12:15:43  dkrajzew
// first steps towards a junctions geometry; tyellow removed again, traffic lights have yellow times given explicitely, now
//
// Revision 1.7  2003/04/04 07:43:04  dkrajzew
// Yellow phases must be now explicetely given; comments added; order of edge sorting (false lane connections) debugged
//
// Revision 1.6  2003/04/01 15:15:54  dkrajzew
// further work on vissim-import
//
// Revision 1.5  2003/03/17 14:22:33  dkrajzew
// further debug and windows eol removed
//
// Revision 1.4  2003/03/06 17:18:44  dkrajzew
// debugging during vissim implementation
//
// Revision 1.3  2003/03/03 14:59:15  dkrajzew
// debugging; handling of imported traffic light definitions
//
// Revision 1.2  2003/02/07 10:43:44  dkrajzew
// updated
//
// Revision 1.1  2002/10/16 15:48:13  dkrajzew
// initial commit for net building classes
//
// Revision 1.6  2002/06/11 16:00:42  dkrajzew
// windows eol removed; template class definition inclusion depends now on the EXTERNAL_TEMPLATE_DEFINITION-definition
//
// Revision 1.5  2002/06/07 14:58:45  dkrajzew
// Bugs on dead ends and junctions with too few outgoing roads fixed; Comments improved
//
// Revision 1.4  2002/05/14 04:42:56  dkrajzew
// new computation flow
//
// Revision 1.3  2002/04/26 10:07:12  dkrajzew
// Windows eol removed; minor double to int conversions removed;
//
// Revision 1.2  2002/04/10 04:52:25  dkrajzew
// False priority of joining lanes removed
//
// Revision 1.1.1.1  2002/04/09 14:18:27  dkrajzew
// new version-free project name (try2)
//
// Revision 1.3  2002/03/22 10:50:03  dkrajzew
// Memory leaks debugging added (MSVC++)
//
// Revision 1.2  2002/03/15 09:19:01  traffic
// False data output patched; Number of lanes added to output; Warnings (conversion, unused variables) patched
//
// Revision 1.1.1.1  2002/02/19 15:33:04  traffic
// Initial import as a separate application.
//
//
/* =========================================================================
 * included modules
 * ======================================================================= */
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <bitset>
#include <sstream>
#include <map>
#include <cassert>
#include "NBEdge.h"
#include "NBJunctionLogicCont.h"
#include "NBContHelper.h"
#include "NBTrafficLightLogic.h"
#include "NBTrafficLightLogicCont.h"
#include "NBTrafficLightLogicVector.h"
#include "NBNode.h"
#include "NBRequestEdgeLinkIterator.h"
#include "NBRequest.h"


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * debugging definitions (MSVC++ only)
 * ======================================================================= */
#ifdef _DEBUG
   #define _CRTDBG_MAP_ALLOC // include Microsoft memory leak detection
   #define _INC_MALLOC	     // exclude standard memory alloc procedures
#endif




size_t NBRequest::myGoodBuilds = 0;
size_t NBRequest::myNotBuild = 0;

/* =========================================================================
 * method definitions
 * ======================================================================= */
NBRequest::NBRequest(NBNode *junction, const EdgeVector * const all,
                     const EdgeVector * const incoming,
                     const EdgeVector * const outgoing,
                     const NBConnectionProhibits &loadedProhibits) :
    _junction(junction),
    _all(all), _incoming(incoming), _outgoing(outgoing)
{
    size_t variations = _incoming->size() * _outgoing->size();
    _forbids.reserve(variations);
    _done.reserve(variations);
    for(size_t i=0; i<variations; i++) {
        _forbids.push_back(LinkInfoCont(variations, false));
        _done.push_back(LinkInfoCont(variations, false));
    }
    // insert loaded prohibits
    for(NBConnectionProhibits::const_iterator j=loadedProhibits.begin(); j!=loadedProhibits.end(); j++) {
        NBConnection prohibited = (*j).first;
        bool ok1 = prohibited.check();
        if(find(_incoming->begin(), _incoming->end(), prohibited.getFrom())==_incoming->end()) {
            ok1 = false;
        }
        if(find(_outgoing->begin(), _outgoing->end(), prohibited.getTo())==_outgoing->end()) {
            ok1 = false;
        }
        size_t idx1 = 0;
        if(ok1) {
            idx1 = getIndex(prohibited.getFrom(), prohibited.getTo());
        }
        const NBConnectionVector &prohibiting = (*j).second;
        for(NBConnectionVector::const_iterator k=prohibiting.begin(); k!=prohibiting.end(); k++) {
            NBConnection sprohibiting = *k;
            bool ok2 = sprohibiting.check();
            if(find(_incoming->begin(), _incoming->end(), sprohibiting.getFrom())==_incoming->end()) {
                ok2 = false;
            }
            if(find(_outgoing->begin(), _outgoing->end(), sprohibiting.getTo())==_outgoing->end()) {
                ok2 = false;
            }
            if(ok1&&ok2) {
                size_t idx2 = getIndex(sprohibiting.getFrom(), sprohibiting.getTo());
                _forbids[idx2][idx1] = true;
                _done[idx2][idx1] = true;
                _done[idx1][idx2] = true;
                myGoodBuilds++;
            } else {
                string pfID = prohibited.getFrom()!=0 ? prohibited.getFrom()->getID() : "UNKNOWN";
                string ptID = prohibited.getTo()!=0 ? prohibited.getTo()->getID() : "UNKNOWN";
                string bfID = sprohibiting.getFrom()!=0 ? sprohibiting.getFrom()->getID() : "UNKNOWN";
                string btID = sprohibiting.getTo()!=0 ? sprohibiting.getTo()->getID() : "UNKNOWN";
				cout << " Warning: could not prohibit "
					<< pfID << "->" << ptID
					<< " by "
					<< bfID << "->" << ptID
					<< endl;
                myNotBuild++;
            }
        }
    }
}


NBRequest::~NBRequest()
{
}


void
NBRequest::buildBitfieldLogic(const std::string &key)
{
    EdgeVector::const_iterator i, j;
    for(i=_incoming->begin(); i!=_incoming->end(); i++) {
//        const EdgeVector &connected = (*i)->getConnected();
        for(j=_outgoing->begin(); j!=_outgoing->end(); j++) {
            computeRightOutgoingLinkCrossings(*i, *j);
            computeLeftOutgoingLinkCrossings(*i, *j);
	EdgeVector::const_iterator i2, j2;
    for(i2=_incoming->begin(); i2!=_incoming->end(); i2++) {
        for(j2=_outgoing->begin(); j2!=_outgoing->end(); j2++) {
			if(*i!=*i2||*j!=*j2) {
				setBlocking(*i, *j, *i2, *j2);
			}
        }
    }

        }
    }
    NBJunctionLogicCont::add(key, bitsetToXML(key));
}


void
NBRequest::computeRightOutgoingLinkCrossings(NBEdge *from, NBEdge *to)
{
    EdgeVector::const_iterator pfrom = find(_all->begin(), _all->end(), from);
    while(*pfrom!=to) {
        pfrom = NBContHelper::nextCCW(_all, pfrom);
        if((*pfrom)->getToNode()==_junction) {
            EdgeVector::const_iterator pto =
                find(_all->begin(), _all->end(), to);
            while(*pto!=from) {
                if(!((*pto)->getToNode()==_junction)) {
                    setBlocking(from, to, *pfrom, *pto);
                }
                pto = NBContHelper::nextCCW(_all, pto);
            }
        }
    }
}


void
NBRequest::computeLeftOutgoingLinkCrossings(NBEdge *from, NBEdge *to)
{
    EdgeVector::const_iterator pfrom = find(_all->begin(), _all->end(), from);
    while(*pfrom!=to) {
        pfrom = NBContHelper::nextCW(_all, pfrom);
        if((*pfrom)->getToNode()==_junction) {
            EdgeVector::const_iterator pto =
                find(_all->begin(), _all->end(), to);
            while(*pto!=from) {
                if(!((*pto)->getToNode()==_junction)) {
                    setBlocking(from, to, *pfrom, *pto);
                }
                pto = NBContHelper::nextCW(_all, pto);
            }
        }
    }
}


void
NBRequest::setBlocking(NBEdge *from1, NBEdge *to1,
                       NBEdge *from2, NBEdge *to2)
{
    // check whether one of the links has a dead end
    if(to1==0||to2==0) {
        return;
    }
    // get the indices of both links
    size_t idx1 = getIndex(from1, to1);
    size_t idx2 = getIndex(from2, to2);
    // check whether the link crossing has already been checked
    assert(idx1<_incoming->size()*_outgoing->size());
    if(_done[idx1][idx2]) {
        return;
    }
    // mark the crossings as done
    _done[idx1][idx2] = true;
    _done[idx2][idx1] = true;
    // check if one of the links is a turn; this link is always not priorised
    if(from1->isTurningDirection(to1)) {
        _forbids[idx2][idx1] = true;
        return;
    }
    if(from2->isTurningDirection(to2)) {
        _forbids[idx1][idx2] = true;
        return;
    }

    // check the priorities
    int from1p = from1->getJunctionPriority(_junction);
    int from2p = from2->getJunctionPriority(_junction);
    // check if one of the connections is higher priorised when incoming into
    // the junction
    // the connection road will yield
    if(from1p>from2p) {
        _forbids[idx1][idx2] = true;
        return;
    }
    if(from2p>from1p) {
        _forbids[idx2][idx1] = true;
        return;
    }
    // check whether one of the connections is higher priorised on
    // the outgoing edge when both roads are high priorised
    // the connection with the lower priorised outgoing edge will lead
    if(from1p>0&&from2p>0) {
        int to1p = to1->getJunctionPriority(_junction);
        int to2p = to2->getJunctionPriority(_junction);
        if(to1p>to2p) {
            _forbids[idx1][idx2] = true;
            return;
        }
        if(to2p>to1p) {
            _forbids[idx2][idx1] = true;
            return;
        }
    }
    // compute the yielding due to the right-before-left rule
    EdgeVector::const_iterator inIncoming1 =
        find(_incoming->begin(), _incoming->end(), from1);
    EdgeVector::const_iterator inIncoming2 =
        find(_incoming->begin(), _incoming->end(), from2);
        // get the position of the incoming lanes in the junction-wheel
    size_t d1 = distance(_incoming->begin(), inIncoming1);
    size_t d2 = distance(_incoming->begin(), inIncoming2);
        // compute the information whether one of the lanes is right of
        // the other (this will then be priorised)
    size_t du, dg;
    if(d1>d2) {
        du = (_incoming->size() - d1) + d2;
        dg = d1 - d2;
    } else {
        du = d2 - d1;
        dg = d1 + (_incoming->size() - d2);
    }
        // the incoming lanes are opposite
        // check which of them will cross the other due to moving to the left
        // this will be the yielding lane
    if(du==dg) {
        size_t dist1 = distanceCounterClockwise(from1, to1);
        size_t dist2 = distanceCounterClockwise(from2, to2);
        if(dist1<dist2)
            _forbids[idx1][idx2] = true;
        if(dist2<dist1)
            _forbids[idx2][idx1] = true;
        return;
    }
    // connection2 forbids proceeding on connection1
    if(dg<du)
        _forbids[idx2][idx1] = true;
    // connection1 forbids proceeding on connection2
    if(dg>du)
        _forbids[idx1][idx2] = true;
}


size_t
NBRequest::distanceCounterClockwise(NBEdge *from, NBEdge *to)
{
    EdgeVector::const_iterator p = find(_all->begin(), _all->end(), from);
    size_t ret = 0;
    while(true) {
        ret++;
        if(p==_all->begin())
            p = _all->end();
        p--;
        if((*p)==to)
            return ret;
    }
}


string
NBRequest::bitsetToXML(string key)
{
    ostringstream os;
    // init
    pair<size_t, size_t> sizes = getSizes();
    size_t absNoLinks = sizes.second;
    size_t absNoLanes = sizes.first;
    assert(absNoLinks>=absNoLanes);
    os << "   <row-logic>" << endl;
    os << "      <key>" << key << "</key>" << endl;
    os << "      <requestsize>" << absNoLinks << "</requestsize>" << endl;
    os << "      <lanenumber>" << absNoLanes << "</lanenumber>" << endl;
    int pos = 0;
    // save the logic
    os << "      <logic>" << endl;
    EdgeVector::const_iterator i;
    for(i=_incoming->begin(); i!=_incoming->end(); i++) {
        size_t noLanes = (*i)->getNoLanes();
        for(size_t k=0; k<noLanes; k++) {
            pos = writeLaneResponse(os, *i, k, pos);
        }
    }
    os << "      </logic>" << endl;
    os << "   </row-logic>" << endl;
    return string(os.str());
}


pair<size_t, size_t>
NBRequest::getSizes() const
{
    size_t noLanes = 0;
    size_t noLinks = 0;
    for( EdgeVector::const_iterator i=_incoming->begin();
         i!=_incoming->end(); i++) {
        size_t noLanesEdge = (*i)->getNoLanes();
        for(size_t j=0; j<noLanesEdge; j++) {
			assert((*i)->getEdgeLanesFromLane(j)->size()!=0);
            noLinks += (*i)->getEdgeLanesFromLane(j)->size();
        }
        noLanes += noLanesEdge;
    }
    return pair<size_t, size_t>(noLanes, noLinks);
}


int
NBRequest::buildTrafficLight(const std::string &key,
                             const NBNode::SignalGroupCont &defs,
                             size_t cycleTime, size_t breakingTime,
                             bool buildAll) const
{
    NBTrafficLightLogicVector *logics = defs.size()!=0
        ? buildLoadedTrafficLights(key, defs, cycleTime)
        : buildOwnTrafficLights(key, breakingTime, buildAll);
    NBTrafficLightLogicCont::insert(key, logics);
    return logics->size();
}



NBTrafficLightLogicVector *
NBRequest::buildLoadedTrafficLights(const std::string &key,
                                    const NBNode::SignalGroupCont &defs,
                                    size_t cycleTime) const
{
	if(_junction->getID()=="334LSA 178") {
		int bla = 0;
	}
    // sort the phases
    NBNode::SignalGroupCont::const_iterator i;
/*    for(i=defs.begin(); i!=defs.end(); i++) {
        (*i).second->sortPhases();
    }
*/
    // compute the switching times
    std::set<double> tmpSwitchTimes;
    for(i=defs.begin(); i!=defs.end(); i++) {
        NBNode::SignalGroup *group = (*i).second;
        DoubleVector gtimes = group->getTimes();
        for(DoubleVector::const_iterator k=gtimes.begin(); k!=gtimes.end(); k++) {
            tmpSwitchTimes.insert(*k);
        }
    }
    std::vector<double> switchTimes;
    copy(tmpSwitchTimes.begin(), tmpSwitchTimes.end(),
        back_inserter(switchTimes));
    sort(switchTimes.begin(), switchTimes.end());

	if(_junction->getID()=="334LSA 178") {
		int bla = 0;
	/// !!! debug-out
	cout << "SwitchTimes:";
	for(std::vector<double>::iterator p=switchTimes.begin(); p!=switchTimes.end(); p++) {
		if(p!=switchTimes.begin()) {
			cout << ", ";
		}
		cout << *p;
	}
	cout << endl;
	/// !!! debug-out
	}

    // count the links (!!!)
    size_t noLinks = 0;
    for(i=defs.begin(); i!=defs.end(); i++) {
        noLinks += (*i).second->getLinkNo();
    }

    // build the phases
    NBTrafficLightLogic *logic =
        new NBTrafficLightLogic(key, noLinks);
    for(std::vector<double>::iterator l=switchTimes.begin(); l!=switchTimes.end(); l++) {
        // compute the duration of the current phase
        size_t duration;
        if(l!=switchTimes.end()-1) {
            // get from the difference to the next switching time
            duration = (size_t) ((*(l+1)) - (*l));
        } else {
            // get from the differenc to the first switching time
            duration = (size_t) (cycleTime - (*l) + *(switchTimes.begin())) ;
        }
        std::pair<std::bitset<64>, std::bitset<64> > masks =
            buildPhaseMasks(defs, *l);
        logic->addStep(duration, masks.first, masks.second);
    }
    // returns the build logic
    NBTrafficLightLogicVector *ret =
        new NBTrafficLightLogicVector(*_incoming);
    ret->add(logic);
    return ret;
}


std::pair<std::bitset<64>, std::bitset<64> >
NBRequest::buildPhaseMasks(const NBNode::SignalGroupCont &defs,
                           size_t time) const
{
    NBRequestEdgeLinkIterator cei1(this, false, false, LRT_NO_REMOVAL);
    // set the masks
    std::bitset<64> driveMask;
    std::bitset<64> brakeMask;
    size_t pos = 0;
    do {
        NBEdge *fromEdge = cei1.getFromEdge();
        NBEdge *toEdge = cei1.getToEdge();
        NBNode::SignalGroup *group =
            _junction->findGroup(fromEdge, toEdge);
        if(group==0) {
            driveMask[pos] = false;
            brakeMask[pos] = true;
        } else {
            driveMask[pos] = group->mayDrive(time);
            brakeMask[pos] = true; // !!! cei1.getBrakeNeeded(defs, *l);
        }
        pos++;
    } while(cei1.pp());
    return std::pair<std::bitset<64>, std::bitset<64> >(driveMask, brakeMask);
}


NBTrafficLightLogicVector *
NBRequest::buildOwnTrafficLights(const std::string &key,
                                 size_t breakingTime,
                                 bool buildAll) const
{
    bool appendSmallestOnly = true;
    bool skipLarger = true;

    bool joinLaneLinks = false;
    bool removeTurnArounds = true;
    LinkRemovalType removal = LRT_REMOVE_WHEN_NOT_OWN;
    NBTrafficLightLogicVector *logics1 =
        computeTrafficLightLogics(key,
            joinLaneLinks, removeTurnArounds, removal,
            appendSmallestOnly, skipLarger, breakingTime);

    if(buildAll) {
        joinLaneLinks = false;
        removeTurnArounds = true;
        removal = LRT_NO_REMOVAL;
        NBTrafficLightLogicVector *logics2 =
            computeTrafficLightLogics(key,
                joinLaneLinks, removeTurnArounds, removal,
                appendSmallestOnly, skipLarger, breakingTime);

        joinLaneLinks = false;
        removeTurnArounds = true;
        removal = LRT_REMOVE_ALL_LEFT;
        NBTrafficLightLogicVector *logics3 =
            computeTrafficLightLogics(key,
                joinLaneLinks, removeTurnArounds, removal,
                appendSmallestOnly, skipLarger, breakingTime);

        // join build logics
        logics1->add(*logics2);
        logics1->add(*logics3);
        delete logics2;
        delete logics3;
    }
    return logics1;
}


NBTrafficLightLogicVector *
NBRequest::computeTrafficLightLogics(const std::string &key,
                                     bool joinLaneLinks,
                                     bool removeTurnArounds,
                                     LinkRemovalType removal,
                                     bool appendSmallestOnly,
                                     bool skipLarger,
                                     size_t breakingTime) const
{
    // compute the matrix of possible links x links
    //  (links allowing each other the parallel execution)
    NBLinkPossibilityMatrix *v = getPossibilityMatrix(joinLaneLinks,
        removeTurnArounds, removal);
    // get the number of regarded links
    NBRequestEdgeLinkIterator cei1(this,
        joinLaneLinks, removeTurnArounds, removal);
    size_t maxStromAnz = cei1.getNoValidLinks();

#ifdef TL_DEBUG
    if(maxStromAnz>=10) {
        cout << _junction->getID() << ":" << maxStromAnz << endl;
    }
#endif

    // compute the link cliquen
    NBLinkCliqueContainer cliquen(v, maxStromAnz);
    // compute the phases
    NBTrafficLightPhases *phases = cliquen.computePhases(v,
        maxStromAnz, appendSmallestOnly, skipLarger);
    // compute the possible logics
    NBTrafficLightLogicVector *logics =
        phases->computeLogics(key, getSizes().second, cei1,
        *_incoming, breakingTime);
    // clean everything
    delete v;
    delete phases;
    return logics;
}


std::vector<std::bitset<64> > *
NBRequest::getPossibilityMatrix(bool joinLaneLinks,
                                bool removeTurnArounds,
                                LinkRemovalType removalType) const
{
    size_t noEdges = _incoming->size();
    // go through all links
    NBRequestEdgeLinkIterator cei1(this, joinLaneLinks, removeTurnArounds,
        removalType);
    std::vector<std::bitset<64> > *ret =
        new std::vector<std::bitset<64> >(cei1.getNoValidLinks(),
        std::bitset<64>());
    do {
        assert(ret!=0 && cei1.getLinkNumber()<ret->size());
        (*ret)[cei1.getLinkNumber()].set(cei1.getLinkNumber(), 1);
        NBRequestEdgeLinkIterator cei2(cei1);
        if(cei2.pp()) {
            do {
                if(cei1.forbids(cei2)) {
                    assert(ret!=0 && cei1.getLinkNumber()<ret->size());
                    assert(ret!=0 && cei2.getLinkNumber()<ret->size());
                    (*ret)[cei1.getLinkNumber()].set(cei2.getLinkNumber(), 0);
                    (*ret)[cei2.getLinkNumber()].set(cei1.getLinkNumber(), 0);
                } else {
                    (*ret)[cei1.getLinkNumber()].set(cei2.getLinkNumber(), 1);
                    (*ret)[cei2.getLinkNumber()].set(cei1.getLinkNumber(), 1);
                }
            } while(cei2.pp());
        }
    } while(cei1.pp());
    return ret;
}




bool
NBRequest::forbidden(NBEdge *from1, NBEdge *to1,
                     NBEdge *from2, NBEdge *to2) const
{
    // unconnected edges do not forbid other edges
    if(to1==0 || to2==0) {
        return false;
    }
    // get the indices
    size_t idx1 = getIndex(from1, to1);
    size_t idx2 = getIndex(from2, to2);
    assert(idx1<_incoming->size()*_outgoing->size());
    assert(idx2<_incoming->size()*_outgoing->size());
    return _forbids[idx1][idx2] || _forbids[idx2][idx1];
}


int
NBRequest::writeLaneResponse(std::ostream &os, NBEdge *from,
                             int lane, int pos)
{
    const EdgeLaneVector * const connected =
        from->getEdgeLanesFromLane(lane);
    for( EdgeLaneVector::const_iterator j=connected->begin();
                j!=connected->end(); j++) {
        os << "         <logicitem request=\"" << pos++
            << "\" response=\"";
        writeResponse(os, from, (*j).edge, lane);
        os << "\"/>" << endl;
    }
    return pos;
}


void
NBRequest::writeResponse(std::ostream &os, NBEdge *from, NBEdge *to, int lane)
{
    // remember the case when the lane is a "dead end" in the meaning that
    // vehicles must choose another lane to move over the following
    // junction
    size_t idx = 0;
    if(to!=0) {
        idx = getIndex(from, to);
    }
    // !!! move to forbidden
    for( EdgeVector::const_reverse_iterator i=_incoming->rbegin();
         i!=_incoming->rend(); i++) {
        unsigned int noLanes = (*i)->getNoLanes();
        for(unsigned int j=noLanes; j-->0; ) {
            const EdgeLaneVector *connected = (*i)->getEdgeLanesFromLane(j);
            size_t size = connected->size();
            for(int k=size; k-->0; ) {
                if(to==0) {
                    os << '1';
                } else if((*i)==from&&lane==j) {
                    // do not prohibit a connection by others from same lane
                    os << '0';
                } else {
                    assert(connected!=0&&k<connected->size());
                    assert(idx<_incoming->size()*_outgoing->size());
                    assert((*connected)[k].edge==0 || getIndex(*i, (*connected)[k].edge)<_incoming->size()*_outgoing->size());
                    if((*connected)[k].edge!=0 &&
                        _forbids[getIndex(*i, (*connected)[k].edge)][idx])
                        os << '1';
                    else
                        os << '0';
                }
            }
        }
    }
}


size_t
NBRequest::getIndex(NBEdge *from, NBEdge *to) const
{
    EdgeVector::const_iterator fp = find(_incoming->begin(),
        _incoming->end(), from);
    EdgeVector::const_iterator tp = find(_outgoing->begin(),
        _outgoing->end(), to);
    // the next two assertions should always fail
    assert(fp!=_incoming->end());
    assert(tp!=_outgoing->end());
    // compute the index
    return distance(
        _incoming->begin(), fp) * _outgoing->size()
        + distance(_outgoing->begin(), tp);
}


std::ostream &operator<<(std::ostream &os, const NBRequest &r) {
    size_t variations = r._incoming->size() * r._outgoing->size();
    for(size_t i=0; i<variations; i++) {
        cout << i << ' ';
        for(size_t j=0; j<variations; j++) {
            if(r._forbids[i][j])
                cout << '1';
            else
                cout << '0';
        }
        cout << endl;
    }
    cout << endl;
    return os;
}


bool
NBRequest::mustBrake(NBEdge *from, NBEdge *to) const
{
	// vehicles which do not have a following lane must always decelerate to the end
	if(to==0) {
		return true;
	}
    // get the indices
    size_t idx2 = getIndex(from, to);
    assert(idx2<_incoming->size()*_outgoing->size());
	for(size_t idx1=0; idx1<_incoming->size()*_outgoing->size(); idx1++) {
		if(_forbids[idx1][idx2]) {
			return true;
		}
	}
	return false;
}


void
NBRequest::reportWarnings()
{
    // check if any errors occured on build the link prohibitions
    if(myNotBuild!=0) {
        cout << "Warning: " << myNotBuild << " of "
            << (myNotBuild+myGoodBuilds) << " prohibitions were not build."
            << endl;
    }
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/
//#ifdef DISABLE_INLINE
//#include "NBRequest.icc"
//#endif

// Local Variables:
// mode:C++
// End:

