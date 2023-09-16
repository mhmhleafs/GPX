/*
Author: Michael Harris
Date: 02/10/2021
*/
#ifndef GPX_HELPERS
#define GPX_HELPERS

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "GPXParser.h"
#include "LinkedListAPI.h"


GPXData* createGPXData (xmlNode* data);

Waypoint* createWaypoint (xmlNode* data);

Route* createRoute (xmlNode* data);

TrackSegment* createTrackSegment (xmlNode* data);

Track* createTrack (xmlNode* data);

xmlNode* nameToNode (char* name, xmlNsPtr namespaceNode);

xmlNode* gpxDataToNode (GPXData* data, xmlNsPtr namespaceNode);

xmlNode* waypointToNode (Waypoint* waypoint, int type, xmlNsPtr namespaceNode);

xmlNode* routeToNode (Route* route, xmlNsPtr namespaceNode);

xmlNode* trackSegmentToNode (TrackSegment* seg, xmlNsPtr namespaceNode);

xmlNode* trackToNode (Track* track, xmlNsPtr namespaceNode);

xmlNode* gpxDocToNode (GPXdoc* doc);

int getSubstringIndex(char* original, char* substring);

float mag(float num);

double haversine(double lat1, double lon1, double lat2, double lon2);

void dummyDelete(void* data);

char* gpxDataToJSON(void* data);

char* listToJSON(List* list, char* (*toJSON)(void*));

char* waypointToJSON(void* data, int pointIndex);

char* trackSegmentToJSON(void* data);

char* routeToJSON2(void* rt);

char* trackToJSON2(void* tr);

int getIndex(char* string, char target);

char* substringCopy(char* s, int start, int end);

char* gpxFileToJSON(char* filename);

bool validGPXFile(char* filename, char* schemaFile);

int numWaypointsInTrack(Track* trk);

char* routeCompToJSON(int compNum, Route* rt);

char* trackCompToJSON(int compNum, Track* trk);

char* docCompToJSON(char* filename);

char* getOtherDataAsJSON(char* filename, char* componentName);

bool changeCompName(char* filename, char* oldName, char* newName);

char* addRouteToFile(char* filename, double lat1, double lon1, double lat2, double lon2, char* name1, char* name2);

bool newGPXdoc(char* filename);

char* routesToJSON(char* filename);

char* pointsToJSON(char* routeName, char* filename);

bool renameRoutes(char* filename);

#endif