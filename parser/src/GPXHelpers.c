/*
Author: Michael Harris
Date: 02/10/2021
*/
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
#include "GPXHelpers.h"

GPXData *createGPXData(xmlNode *data)
{
    if (data == NULL)
    {
        return NULL;
    }

    char *content = calloc(256, sizeof(char));

    if (data->children->content == NULL)
    {
        strcpy(content, "NULL CONTENT");
    }

    else if (strcmp((char *)data->children->content, "") == 0)
    {
        strcpy(content, "EMPTY CONTENT");
    }

    else
    {
        strcpy(content, (char *)data->children->content);
    }

    GPXData *gpxData = calloc(1, sizeof(GPXData) + 256);

    if (data->name != NULL)
        strcpy(gpxData->name, (char *)data->name);
    else
        strcpy(gpxData->name, "NO NAME");

    strcpy(gpxData->value, content);

    free(content);

    return gpxData;
}

Waypoint *createWaypoint(xmlNode *data)
{
    if (data == NULL)
    {
        return NULL;
    }

    Waypoint *waypoint = calloc(1, sizeof(Waypoint));

    waypoint->name = calloc(256, sizeof(char));

    waypoint->latitude = 0.00;
    waypoint->longitude = 0.00;

    waypoint->otherData = initializeList(&(gpxDataToString), &(deleteGpxData), &(compareGpxData));

    for (xmlNode *child = data->children; child != NULL; child = child->next)
    {
        char *name = (char *)child->name;

        if (strcmp(name, "name") == 0)
        {
            char *dummy = (char *)xmlNodeGetContent(child->children);

            if (dummy != NULL)
            {
                strcpy(waypoint->name, dummy);

                free(dummy);
            }
        }
    }

    xmlAttr *attr;
    for (attr = data->properties; attr != NULL; attr = attr->next)
    {
        xmlNode *value = attr->children;
        char *attrName = (char *)attr->name;
        char *cont = (char *)(value->content);

        if (attrName == NULL)
        {
            continue;
        }

        if (cont == NULL)
        {
            continue;
        }

        if (strcmp(attrName, "lat") == 0)
        {
            waypoint->latitude = atof(cont);
        }

        if (strcmp(attrName, "lon") == 0)
        {
            waypoint->longitude = atof(cont);
        }
    }

    for (xmlNode *child = data->children; child != NULL; child = child->next)
    {
        char *name = (char *)child->name;

        if (name == NULL)
            continue;

        if ((strcmp(name, "name") != 0) && (strcmp(name, "text") != 0))
        {
            GPXData *gpx = createGPXData(child);

            if (gpx != NULL)
            {
                insertBack(waypoint->otherData, gpx);
            }
        }
    }

    return waypoint;
}

Route *createRoute(xmlNode *data)
{
    if (data == NULL)
    {
        return NULL;
    }

    if (data->name == NULL)
    {
        return NULL;
    }

    Route *route = calloc(1, sizeof(Route));

    route->name = calloc(256, sizeof(char));

    for (xmlNode *child = data->children; child != NULL; child = child->next)
    {
        char *name = (char *)child->name;

        if (strcmp(name, "name") == 0)
        {
            char *dummy = (char *)xmlNodeGetContent(child->children);

            if (dummy != NULL)
            {
                strcpy(route->name, dummy);

                free(dummy);
            }
        }
    }

    route->waypoints = initializeList(&(waypointToString), &(deleteWaypoint), &(compareWaypoints));

    route->otherData = initializeList(&(gpxDataToString), &(deleteGpxData), &(compareGpxData));

    for (xmlNode *child = data->children; child != NULL; child = child->next)
    {
        char *name = (char *)child->name;

        if (name == NULL)
        {
            continue;
        }

        if ((strcmp(name, "name") != 0) && (strcmp(name, "text") != 0))
        {

            if (strcmp(name, "rtept") == 0)
            {
                Waypoint *waypoint = createWaypoint(child);

                if (waypoint != NULL)
                {
                    insertBack(route->waypoints, waypoint);
                }
            }

            else
            {
                GPXData *gpx = createGPXData(child);

                if (gpx != NULL)
                {
                    insertBack(route->otherData, gpx);
                }
            }
        }
    }

    return route;
}

TrackSegment *createTrackSegment(xmlNode *data)
{
    if (data == NULL)
    {
        return NULL;
    }

    TrackSegment *seg = calloc(1, sizeof(TrackSegment));

    seg->waypoints = initializeList(&(waypointToString), &(deleteWaypoint), &(compareWaypoints));

    for (xmlNode *child = data->children; child != NULL; child = child->next)
    {
        char *name = (char *)child->name;

        if (name == NULL)
        {
            continue;
        }

        if ((strcmp(name, "trkpt") == 0))
        {
            Waypoint *waypoint = createWaypoint(child);

            if (waypoint != NULL)
            {
                insertBack(seg->waypoints, waypoint);
            }
        }
    }

    return seg;
}

Track *createTrack(xmlNode *data)
{
    if (data == NULL)
    {
        return NULL;
    }

    Track *track = calloc(1, sizeof(Track));

    track->name = calloc(256, sizeof(char));

    track->segments = initializeList(&(trackSegmentToString), &(deleteTrackSegment), &(compareTrackSegments));

    track->otherData = initializeList(&(gpxDataToString), &(deleteGpxData), &(compareGpxData));

    for (xmlNode *child = data->children; child != NULL; child = child->next)
    {
        char *name = (char *)child->name;

        if (name == NULL)
        {
            continue;
        }

        if (strcmp(name, "name") == 0)
        {
            char *dummy = (char *)xmlNodeGetContent(child->children);

            if (dummy != NULL)
            {
                strcpy(track->name, dummy);

                free(dummy);
            }
        }
    }

    for (xmlNode *child = data->children; child != NULL; child = child->next)
    {
        char *name = (char *)child->name;

        if (name == NULL)
        {
            continue;
        }

        if ((strcmp(name, "name") != 0) && (strcmp(name, "text") != 0))
        {

            if (strcmp(name, "trkseg") == 0)
            {
                TrackSegment *seg = createTrackSegment(child);

                if (seg != NULL)
                {
                    insertBack(track->segments, seg);
                }
            }

            else
            {
                GPXData *gpx = createGPXData(child);

                if (gpx != NULL)
                {
                    insertBack(track->otherData, gpx);
                }
            }
        }
    }

    return track;
}

// A2

xmlNode *nameToNode(char *name, xmlNsPtr namespaceNode)
{
    xmlNode *n = NULL;
    n = xmlNewNode(NULL, BAD_CAST "name");
    xmlSetNs(n, namespaceNode);

    xmlNode *text;

    text = xmlNewText(BAD_CAST name);

    xmlAddChild(n, text);

    return n;
}

xmlNode *gpxDataToNode(GPXData *data, xmlNsPtr namespaceNode)
{
    xmlNode *n = NULL;

    n = xmlNewNode(NULL, BAD_CAST data->name);
    xmlSetNs(n, namespaceNode);

    xmlNode *text;

    text = xmlNewText(BAD_CAST data->value);

    xmlAddChild(n, text);

    return n;
}

xmlNode *waypointToNode(Waypoint *waypoint, int type, xmlNsPtr namespaceNode)
{
    xmlNode *n = NULL;

    if (type == 0)
    {
        n = xmlNewNode(NULL, BAD_CAST "wpt");
        xmlSetNs(n, namespaceNode);
    }
    else if (type == 1)
    {
        n = xmlNewNode(NULL, BAD_CAST "rtept");
        xmlSetNs(n, namespaceNode);
    }
    else if (type == 2)
    {
        n = xmlNewNode(NULL, BAD_CAST "trkpt");
        xmlSetNs(n, namespaceNode);
    }
    else
        return NULL;

    for (Node *child = waypoint->otherData->head; child != NULL; child = child->next)
    {
        xmlAddChild(n, gpxDataToNode(child->data, namespaceNode));
    }

    xmlAddChild(n, nameToNode(waypoint->name, namespaceNode));

    char buffer[10];

    strcpy(buffer, "");

    sprintf(buffer, "%.2f", waypoint->latitude);

    xmlNewProp(n, BAD_CAST "lat", BAD_CAST buffer);

    strcpy(buffer, "");

    sprintf(buffer, "%.2f", waypoint->longitude);

    xmlNewProp(n, BAD_CAST "lon", BAD_CAST buffer);

    return n;
}

xmlNode *routeToNode(Route *route, xmlNsPtr namespaceNode)
{
    xmlNode *n = NULL;

    n = xmlNewNode(NULL, BAD_CAST "rte");
    xmlSetNs(n, namespaceNode);

    xmlAddChild(n, nameToNode(route->name, namespaceNode));

    for (Node *child = route->otherData->head; child != NULL; child = child->next)
    {
        xmlAddChild(n, gpxDataToNode(child->data, namespaceNode));
    }

    for (Node *child = route->waypoints->head; child != NULL; child = child->next)
    {
        xmlAddChild(n, waypointToNode(child->data, 1, namespaceNode));
    }

    return n;
}

xmlNode *trackSegmentToNode(TrackSegment *seg, xmlNsPtr namespaceNode)
{
    xmlNode *n = NULL;

    n = xmlNewNode(NULL, BAD_CAST "trkseg");
    xmlSetNs(n, namespaceNode);

    for (Node *child = seg->waypoints->head; child != NULL; child = child->next)
    {
        xmlAddChild(n, waypointToNode(child->data, 2, namespaceNode));
    }

    return n;
}

xmlNode *trackToNode(Track *track, xmlNsPtr namespaceNode)
{
    xmlNode *n = NULL;

    n = xmlNewNode(NULL, BAD_CAST "trk");
    xmlSetNs(n, namespaceNode);

    xmlAddChild(n, nameToNode(track->name, namespaceNode));

    for (Node *child = track->otherData->head; child != NULL; child = child->next)
    {
        xmlAddChild(n, gpxDataToNode(child->data, namespaceNode));
    }

    for (Node *child = track->segments->head; child != NULL; child = child->next)
    {
        xmlAddChild(n, trackSegmentToNode(child->data, namespaceNode));
    }

    return n;
}

xmlNode *gpxDocToNode(GPXdoc *doc)
{
    xmlNode *n = xmlNewNode(NULL, BAD_CAST "gpx");

    xmlNsPtr namespaceNode = xmlNewNs(n, BAD_CAST doc->namespace, NULL);
    xmlSetNs(n, namespaceNode);

    char buffer[10];

    strcpy(buffer, "");

    sprintf(buffer, "%.1f", doc->version);

    xmlNewProp(n, BAD_CAST "version", BAD_CAST buffer);

    xmlNewProp(n, BAD_CAST "creator", BAD_CAST doc->creator);

    for (Node *child = doc->waypoints->head; child != NULL; child = child->next)
    {
        xmlAddChild(n, waypointToNode(child->data, 0, namespaceNode));
    }

    for (Node *child = doc->routes->head; child != NULL; child = child->next)
    {
        xmlAddChild(n, routeToNode(child->data, namespaceNode));
    }

    for (Node *child = doc->tracks->head; child != NULL; child = child->next)
    {
        xmlAddChild(n, trackToNode(child->data, namespaceNode));
    }

    return n;
}

int getSubstringIndex(char *original, char *substring)
{
    int index = -1;
    char *temp = strstr(original, substring);

    if (temp != NULL)
    {
        index = temp - original;
    }

    return index;
}

float mag(float num)
{
    float dum = num;

    if (dum < 0)
        dum *= -1;

    return dum;
}

double haversine(double lat1, double lon1, double lat2, double lon2)
{
    double a = 0.0, c = 0.0, d = 0.0, dist = 0.0;
    double phi1 = 0.0, phi2 = 0.0, deltaPhi = 0.0, deltaLambda = 0.0;
    double r = 6371000.0;

    phi1 = lat1 * M_PI / 180.0;
    phi2 = lat2 * M_PI / 180.0;

    deltaPhi = (lat2 - lat1) * M_PI / 180.0;
    deltaLambda = (lon2 - lon1) * M_PI / 180.0;

    a = sin(deltaPhi / 2.0) * sin(deltaPhi / 2.0) + (cos(phi1) * cos(phi2) * sin(deltaLambda / 2.0) * sin(deltaLambda / 2.0));

    c = 2.0 * asin(sqrt(a));

    d = r * c;

    dist += d;

    return dist;
}

void dummyDelete(void *data)
{
    return;
}

char *gpxDataToJSON(void *data) //useless
{
    if (data == NULL)
        return NULL;

    GPXData *gpxData = data;

    /*if(strlen(gpxData->value) > 0)
    {
        for(int i = 0; i < strlen(gpxData->value); i++);
        {
            if((gpxData->value)[i] == '\n' || (gpxData->value)[i] == '\t' || (gpxData->value)[i] == '\r'))
            {
                (gpxData->value)[i] = ' ';
            }
        }
    }*/

    char *JSONstring = calloc(strlen(gpxData->name) + strlen(gpxData->value) + 512, sizeof(char));

    sprintf(JSONstring, "{\"name\":\"%s\",\"value\":\"%s\"}", gpxData->name, gpxData->value);

    return JSONstring;
}

char *listToJSON(List *list, char *(*toJSON)(void *))
{
    int memSize = 100;

    char *JSON = calloc(memSize, sizeof(char));

    strcat(JSON, "[");

    for (Node *node = list->head; node != NULL; node = node->next)
    {
        char *temp = toJSON(node->data);

        memSize += strlen(temp) + 10;

        JSON = realloc(JSON, memSize * sizeof(char));

        strcat(JSON, temp);

        free(temp);

        if (node->next != NULL)
            strcat(JSON, ",");
    }

    strcat(JSON, "]");

    return JSON;
}

char *waypointToJSON(void *data, int pointIndex) //useless
{
    if (data == NULL)
        return NULL;

    Waypoint *waypoint = data;

    char *otherDataJSON = listToJSON(waypoint->otherData, gpxDataToJSON);

    int memSize = strlen(otherDataJSON) + 500;

    char *JSON = calloc(memSize, sizeof(char));

    char* name = calloc(256, sizeof(char));

    if(strcmp(waypoint->name, "") == 0)
    {
        strcpy(name, "null");
    }
    else
    {
        strcpy(name, waypoint->name);
    }

    sprintf(JSON, "{\"name\":\"%s\",\"latitude\":%.6f,\"longitude\":%.6f,\"otherData\":%s,\"index\":%d}", name, waypoint->latitude, waypoint->longitude, otherDataJSON, pointIndex);

    free(otherDataJSON);

    return JSON;
}

char *trackSegmentToJSON(void *data) //useless
{
    return NULL;
    /*
    if (data == NULL)
        return NULL;

    TrackSegment *seg = data;

    char *waypointJSON = listToJSON(seg->waypoints, waypointToJSON, -1);

    int memSize = strlen(waypointJSON) + 500;

    char *JSON = calloc(memSize, sizeof(char));

    sprintf(JSON, "{\"waypoints\":%s}", waypointJSON);

    free(waypointJSON);

    return JSON;*/
}

char *routeToJSON2(void *rt)
{
    if (rt == NULL)
        return NULL;

    Route *route = rt;

    int memSize = 1024;

    char *JSON = calloc(memSize, sizeof(char));

    char *name = calloc(256, sizeof(char));
    char *loopStatus = calloc(256, sizeof(char));

    if (strcmp(route->name, "") == 0)
        strcpy(name, "None");
    else
        strcpy(name, route->name);

    if (isLoopRoute(route, 10))
        strcpy(loopStatus, "true");
    else
        strcpy(loopStatus, "false");

    sprintf(JSON, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", name, getLength(route->waypoints), round10(getRouteLen(route)), loopStatus);

    free(name);
    free(loopStatus);

    return JSON;
}

char *trackToJSON2(void *tr)
{
    if (tr == NULL)
        return NULL;

    Track *track = tr;

    int memSize = 1024;

    char *JSON = calloc(memSize, sizeof(char));

    char *name = calloc(256, sizeof(char));
    char *loopStatus = calloc(256, sizeof(char));

    if (strcmp(track->name, "") == 0)
        strcpy(name, "None");
    else
        strcpy(name, track->name);

    if (isLoopTrack(track, 10))
        strcpy(loopStatus, "true");
    else
        strcpy(loopStatus, "false");

    sprintf(JSON, "{\"name\":\"%s\",\"len\":%.1f,\"loop\":%s}", name, round10(getTrackLen(track)), loopStatus);

    free(name);
    free(loopStatus);

    return JSON;
}

int getIndex(char *string, char target)
{
    if (string == NULL)
        return -1;

    for (int i = 0; i < strlen(string); i++)
    {
        if (string[i] == target)
            return i;
    }

    return -1;
}

char *substringCopy(char *s, int start, int end)
{
    if (s == NULL || (start > end))
        return NULL;

    char *copy = calloc(strlen(s) + 1, sizeof(char));

    for (int i = start; i < end; i++)
    {
        copy[i - start] = s[i];
    }

    return copy;
}

char *gpxFileToJSON(char *filename)
{
    if (filename == NULL)
        return "{}";

    GPXdoc *doc = createGPXdoc(filename);

    if (doc == NULL)
        return "{}";

    int routeNum = 1;

    for(Node* node = doc->routes->head; node != NULL; node=node->next)
    {
        Route* rt = node->data;

        if(strlen(rt->name) == 0)
        {
            char newName[50];

            sprintf(newName, "Unnamed Route %d", routeNum);

            routeNum++;
        }
    }

    writeGPXdoc(doc, filename);

    int memSize = 1024;

    char *JSON = calloc(memSize, sizeof(char));

    sprintf(JSON, "{\"fileName\":\"%s\",\"version\":%.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}", filename, doc->version, doc->creator, getNumWaypoints(doc), getNumRoutes(doc), getNumTracks(doc));

    return JSON;
}

bool validGPXFile(char *filename, char *schemaFile)
{
    if (filename == NULL || schemaFile == NULL)
        return false;

    GPXdoc *doc = createValidGPXdoc(filename, schemaFile);

    if (doc == NULL)
        return false;

    bool flag = validateGPXDoc(doc, schemaFile);

    deleteGPXdoc(doc);

    return flag;
}

int numWaypointsInTrack(Track *trk)
{
    if (trk == NULL)
        return 0;

    int count = 0;

    for (Node *node = trk->segments->head; node != NULL; node = node->next)
    {
        TrackSegment *tseg = node->data;

        count += getLength(tseg->waypoints);
    }

    return count;
}

char *routeCompToJSON(int compNum, Route *rt)
{
    if (rt == NULL)
        return "{}";

    Route *route = rt;

    int memSize = 1024;

    char *JSON = calloc(memSize, sizeof(char));

    char *name = calloc(256, sizeof(char));
    char *loopStatus = calloc(256, sizeof(char));

    if (strcmp(route->name, "") == 0)
        strcpy(name, "None");
    else
        strcpy(name, route->name);

    if (isLoopRoute(route, 10))
        strcpy(loopStatus, "true");
    else
        strcpy(loopStatus, "false");

    char array[50];
    sprintf(array, "Route %d", compNum);

    sprintf(JSON, "{\"component\":\"%s\",\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", array, name, getLength(route->waypoints), round10(getRouteLen(route)), loopStatus);

    free(name);
    free(loopStatus);

    return JSON;
}

char *trackCompToJSON(int compNum, Track *rt)
{
    if (rt == NULL)
        return "{}";

    Track *route = rt;

    int memSize = 1024;

    char *JSON = calloc(memSize, sizeof(char));

    char *name = calloc(256, sizeof(char));
    char *loopStatus = calloc(256, sizeof(char));

    if (strcmp(route->name, "") == 0)
        strcpy(name, "None");
    else
        strcpy(name, route->name);

    if (isLoopTrack(route, 10))
        strcpy(loopStatus, "true");
    else
        strcpy(loopStatus, "false");

    int numPoints = numWaypointsInTrack(rt);

    char array[50];
    sprintf(array, "Track %d", compNum);

    sprintf(JSON, "{\"component\":\"%s\",\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", array, name, numPoints, round10(getTrackLen(route)), loopStatus);

    free(name);
    free(loopStatus);

    return JSON;
}

char *docCompToJSON(char *filename)
{
    if (filename == NULL)
        return "[]";

    GPXdoc *doc = createGPXdoc(filename);

    if (doc == NULL)
        return "[]";

    int count = 1;
    int trackLen = getLength(doc->tracks);
    int routeLen = getLength(doc->routes);

    int memSize = 100;
    char *JSON = calloc(memSize, sizeof(char));
    strcat(JSON, "[");

    if (trackLen > 0 && routeLen > 0)
    {
        for (Node *node = doc->routes->head; node != NULL; node = node->next)
        {
            char *temp = routeCompToJSON(count, node->data);

            memSize += strlen(temp) + 2;

            JSON = realloc(JSON, memSize * sizeof(char));

            strcat(JSON, temp);
            strcat(JSON, ",");

            free(temp);

            count++;
        }
        count = 1;
        for (Node *node = doc->tracks->head; node != NULL; node = node->next)
        {
            char *temp = trackCompToJSON(count, node->data);

            memSize += strlen(temp) + 2;

            JSON = realloc(JSON, memSize * sizeof(char));

            strcat(JSON, temp);
            if (node->next != NULL)
                strcat(JSON, ",");

            free(temp);

            count++;
        }

        strcat(JSON, "]");

        return JSON;
    }

    if (routeLen > 0)
    {
        for (Node *node = doc->routes->head; node != NULL; node = node->next)
        {
            char *temp = routeCompToJSON(count, node->data);

            memSize += strlen(temp) + 2;

            JSON = realloc(JSON, memSize * sizeof(char));

            strcat(JSON, temp);
            if (node->next != NULL)
                strcat(JSON, ",");

            free(temp);

            count++;
        }

        strcat(JSON, "]");

        return JSON;
    }

    if (trackLen > 0)
    {
        for (Node *node = doc->tracks->head; node != NULL; node = node->next)
        {
            char *temp = trackCompToJSON(count, node->data);

            memSize += strlen(temp) + 2;

            JSON = realloc(JSON, memSize * sizeof(char));

            strcat(JSON, temp);
            if (node->next != NULL)
                strcat(JSON, ",");

            free(temp);

            count++;
        }

        strcat(JSON, "]");

        return JSON;
    }

    strcat(JSON, "]");

    return JSON;
}

char *getOtherDataAsJSON(char *filename, char *componentName)
{
    if (filename == NULL || componentName == NULL)
        return "null";

    GPXdoc *doc = createGPXdoc(filename);

    if (doc == NULL)
        return "null";

    Route *rt = getRoute(doc, componentName);

    Track *trk = getTrack(doc, componentName);

    if (rt == NULL && trk == NULL)
    {
        return "\"No otherData found.\"";
    }
    else if (rt != NULL)
    {
        char *JSON = listToJSON(rt->otherData, &(gpxDataToJSON));

        return JSON;
    }
    else
    {
        char *JSON = listToJSON(trk->otherData, &(gpxDataToJSON));

        return JSON;
    }
}

bool changeCompName(char *filename, char *oldName, char *newName)
{

    //printf("calling f'n\n");

    if (filename == NULL || oldName == NULL || newName == NULL)
        return false;
    //printf("no null arg: %s, %s, %s\n", filename, oldName, newName);
    GPXdoc *doc = createGPXdoc(filename);

    if (doc == NULL)
        return false;
    //printf("doc not null\n");
    Route *rt = getRoute(doc, oldName);

    Track *trk = getTrack(doc, oldName);

    if (rt == NULL && trk == NULL)
    {
        printf("both null\n");
        return false;
    }
    else if (rt != NULL)
    {
        strcpy(rt->name, newName);
    }
    else
    {
        strcpy(trk->name, newName);
    }
    bool written = writeGPXdoc(doc, filename);

    printf("written = %d\n", written);
    return written;
}

char* addRouteToFile(char *filename, double lat1, double lon1, double lat2, double lon2, char *name1, char *name2)
{
    if (filename == NULL || name1 == NULL || name1 == NULL)
        return false;
    //printf("no null arg: %s, %s, %s\n", filename, oldName, newName);
    GPXdoc *doc = createGPXdoc(filename);

    if (doc == NULL)
        return false;

    Waypoint *wpt1 = calloc(1, sizeof(Waypoint));

    wpt1->latitude = lat1;
    wpt1->longitude = lon1;

    wpt1->otherData = initializeList(&(gpxDataToString), &(deleteGpxData), &(compareGpxData));

    Waypoint *wpt2 = calloc(1, sizeof(Waypoint));

    wpt2->latitude = lat2;
    wpt2->longitude = lon2;

    wpt2->otherData = initializeList(&(gpxDataToString), &(deleteGpxData), &(compareGpxData));

    Route *rt = calloc(1, sizeof(Route));
    rt->name = calloc(512, sizeof(char));
    strcpy(rt->name, name1);

    rt->otherData = initializeList(&(gpxDataToString), &(deleteGpxData), &(compareGpxData));

    rt->waypoints = initializeList(&(waypointToString), &(deleteWaypoint), &(compareWaypoints));

    insertBack(rt->waypoints, wpt1);
    insertBack(rt->waypoints, wpt2);

    addRoute(doc, rt);

    bool written = writeGPXdoc(doc, filename);

    if(written)
        return(routeToJSON2(rt));
    else
        return "{}";
}

bool newGPXdoc(char *filename)
{
    GPXdoc *doc = calloc(1, sizeof(GPXdoc));

    doc->waypoints = initializeList(&(waypointToString), &(deleteWaypoint), &(compareWaypoints));

    doc->routes = initializeList(&(routeToString), &(deleteRoute), &(compareRoutes));

    doc->tracks = initializeList(&(trackToString), &(deleteTrack), &(compareTracks));

    strcpy(doc->namespace, "http://www.topografix.com/GPX/1/1");

    doc->version = 1.1;

    doc->creator = calloc(512, sizeof(char));

    strcpy(doc->creator, "DefaultCreator");

    bool written = writeGPXdoc(doc, filename);
    printf("CREATED A NEW DOC? %d\n", written);
    return written;
}

char *routesToJSON(char *filename)
{
    GPXdoc *doc = createGPXdoc(filename);

    if (doc == NULL)
        return "[]";

    return listToJSON(doc->routes, routeToJSON2);
}

char *pointsToJSON(char *routeName, char *filename)
{
    if (routeName == NULL || filename == NULL)
        return "[]";

    GPXdoc *doc = createGPXdoc(filename);

    if (doc == NULL)
        return "[]";

    Route *rt = getRoute(doc, routeName);

    if (rt == NULL)
        return "[]";


    int memSize = 100;

    char *JSON = calloc(memSize, sizeof(char));

    strcat(JSON, "[");

    int index = 0;

    for (Node *node = rt->waypoints->head; node != NULL; node = node->next)
    {
        char *temp = waypointToJSON(node->data, index);

        memSize += strlen(temp) + 10;

        JSON = realloc(JSON, memSize * sizeof(char));

        strcat(JSON, temp);

        free(temp);

        if (node->next != NULL)
            strcat(JSON, ",");

        index++;
    }

    strcat(JSON, "]");

    return JSON;
}

bool renameRoutes(char* filename)
{
    if (filename == NULL)
        return "{}";

    GPXdoc *doc = createGPXdoc(filename);

    if (doc == NULL)
        return "{}";

    int routeNum = 1;

    for(Node* node = doc->routes->head; node != NULL; node=node->next)
    {
        Route* rt = node->data;

        if(strlen(rt->name) == 0)
        {
            char newName[50];

            sprintf(newName, "Unnamed Route %d", routeNum);

            strcpy(rt->name, newName);

            routeNum++;
        }
    }

    return writeGPXdoc(doc, filename);
}
