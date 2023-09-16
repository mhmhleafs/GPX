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

void deleteGpxData( void* data)
{
    if (data==NULL)
    {
        return;
    }

    GPXData* temp = (GPXData*)data;

    free(temp);

    return;
}


char* gpxDataToString( void* data)
{
    if (data==NULL)
    {
        return NULL;
    }

    GPXData* temp = (GPXData*)data;

    char* toPrint = calloc(256 + strlen(temp->value) + 1, sizeof(char));

    sprintf(toPrint, "\nGPXData\n  name:%s\n  value:%s\n\n", temp->name, temp->value);
    


    return toPrint;
}

int compareGpxData(const void *first, const void *second)
{
    return -1;
}

void deleteWaypoint(void* data)
{
    if(data == NULL)
    {
        return;
    }

    Waypoint* temp = (Waypoint*) data;

    free(temp->name);

    if(temp->otherData != NULL)
    {
        freeList(temp->otherData);
    }

    free(temp);

    return;
}

char* waypointToString(void* data)
{
    if(data == NULL)
    {
        return NULL;
    }

    Waypoint* temp = (Waypoint*) data;

    char* otherDataString = toString(temp->otherData);

    char* toPrint = calloc(1024 + strlen(otherDataString), sizeof(char));

    sprintf(toPrint, "\nWaypoint\n  name:%s\n  lon:%.2f\n  lat:%.2f\n  other data: %s\n\n", temp->name, temp->longitude, temp->latitude, otherDataString);

    free(otherDataString);

    return toPrint;
}

int compareWaypoints(const void *first, const void *second)
{
    return -1;
}

void deleteRoute(void* data)
{
    if(data == NULL)
    {
        return;
    }

    Route* temp = (Route*) data;

    free(temp->name);

     if(temp->waypoints != NULL)
    {
        freeList(temp->waypoints);
    }

    if(temp->otherData != NULL)
    {
        freeList(temp->otherData);
    }

    free(temp);

    return;
}

char* routeToString(void* data)
{
    if(data == NULL)
    {
        return NULL;
    }

    Route* temp = (Route*) data;

    char* otherDataString = toString(temp->otherData);

    char* waypointString = toString(temp->waypoints);

    char* toPrint = calloc(1024 + strlen(otherDataString) + strlen(waypointString), sizeof(char));


    sprintf(toPrint, "\nRoute\n  name:%s\n  waypoints: %s\n  other data: %s\n\n", temp->name, waypointString, otherDataString);

    free(otherDataString);

    free(waypointString);

    return toPrint;
}

int compareRoutes(const void *first, const void *second)
{
    return -1;
}

void deleteTrackSegment(void* data)
{
    if(data == NULL)
    {
        return;
    }

    TrackSegment* seg = (TrackSegment*) data;

    if(seg->waypoints != NULL)
    {
        freeList(seg->waypoints);
    }

    free(seg);

    return;
}
char* trackSegmentToString(void* data)
{
    if(data == NULL)
    {
        return NULL;
    }

    TrackSegment* temp = (TrackSegment*) data;

    char* waypointString = toString(temp->waypoints);

    char* toPrint = calloc(1024 + strlen(waypointString), sizeof(char));


    sprintf(toPrint, "\nTrackSegment\n  waypoints: %s\n\n", waypointString);

    free(waypointString);

    return toPrint;

}
int compareTrackSegments(const void *first, const void *second)
{
    return -1;
}

void deleteTrack(void* data)
{
    if(data == NULL)
    {
        return;
    }

    Track* track = (Track*) data;

    if(track->name != NULL)
    {
        free(track->name);
    }

    if(track->segments != NULL)
    {
        freeList(track->segments);
    }

    if(track->otherData != NULL)
    {
        freeList(track->otherData);
    }

    free(track);

    return;
}

char* trackToString(void* data)
{
    if(data == NULL)
    {
        return NULL;
    }

    Track* temp = (Track*) data;

    char* otherDataString = toString(temp->otherData);

    char* segmentString = toString(temp->segments);

    char* toPrint = calloc(1024 + strlen(otherDataString) + strlen(segmentString), sizeof(char));



    sprintf(toPrint, "\nTrack\n  name:%s\n  waypoints: %s\n  other data: %s\n\n", temp->name, segmentString, otherDataString);

    free(otherDataString);

    free(segmentString);

    return toPrint;
}

int compareTracks(const void *first, const void *second)
{
    return -1;
}

GPXdoc* createGPXdoc(char* fileName)
{
    if(fileName == NULL)
    {
        return NULL;
    }

    xmlDoc* doc = NULL;
    xmlNode* root_element = NULL;

    LIBXML_TEST_VERSION

    /*parse the file and get the DOM */
    doc = xmlReadFile(fileName, NULL, 0);

    if (doc == NULL) 
    {
        xmlCleanupParser();

        return NULL;
    }

    root_element = xmlDocGetRootElement(doc);

    if (root_element == NULL || root_element->type != XML_ELEMENT_NODE)
    {
        return NULL;
    }

    GPXdoc* gpxDoc = calloc(1, sizeof(GPXdoc));

    char* namespace = (char*) root_element->ns->href;

    if(namespace == NULL)
    {
        strcpy(gpxDoc->namespace, "NAMESPACE IS NULL\n");
    }

    else if(strlen(namespace) == 0)
    {
        strcpy(gpxDoc->namespace, "NAMESPACE IS EMPTY\n");
    }

    else
    {
        strcpy(gpxDoc->namespace, namespace);
    }

    gpxDoc->creator = calloc(256, sizeof(char));

    strcpy(gpxDoc->creator, "DUMMY CREATOR");

    gpxDoc->version = 1.1;

    xmlAttr *attr;
    for (attr = root_element->properties; attr != NULL; attr = attr->next)
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

        if (strcmp(attrName, "creator") == 0)
        {
            if(cont != NULL)
                strcpy(gpxDoc->creator, cont);
        }

        if (strcmp(attrName, "version") == 0)
        {
            if(cont != NULL)
                gpxDoc->version = atof(cont);
        }
    }



    gpxDoc->waypoints = initializeList(&(waypointToString), &(deleteWaypoint), &(compareWaypoints));

    gpxDoc->routes = initializeList(&(routeToString), &(deleteRoute), &(compareRoutes));

    gpxDoc->tracks = initializeList(&(trackToString), &(deleteTrack), &(compareTracks));
    

    for (xmlNode *child = root_element->children; child != NULL; child = child->next)
    {
        char* name = (char*) child->name;

        if(strcmp(name, "wpt") == 0)
        {
            Waypoint* waypoint = createWaypoint(child);

            if(waypoint != NULL)
            {
                insertBack(gpxDoc->waypoints, waypoint);
            }
        }

        if(strcmp(name, "rte") == 0)
        {
            Route* route = createRoute(child);

            if(route != NULL)
            {
                insertBack(gpxDoc->routes, route);
            }
        }

        if(strcmp(name, "trk") == 0)
        {
            Track* track = createTrack(child);

            if(track != NULL)
            {
                insertBack(gpxDoc->tracks, track);
            }
        }
    }

    xmlFreeDoc(doc);

    xmlCleanupParser();

    return gpxDoc;
}

char* GPXdocToString(GPXdoc* doc)
{
    if(doc == NULL)
    {
        return NULL;
    }

    char* waypointString = toString(doc->waypoints);

    char* routeString = toString(doc->routes);

    char* trackString = toString(doc->tracks);

    char* toPrint = calloc(1024 + strlen(waypointString) + strlen(routeString) + strlen(trackString), sizeof(char));

    

    sprintf(toPrint, "\nGPXdoc\n  namespace:%s\n  version: %.2f\n  creator: %s\n waypoints: %s\n routes: %s\n tracks: %s\n\n", doc->namespace, doc->version, doc->creator, waypointString, routeString, trackString);

    free(waypointString);

    free(routeString);

    free(trackString);

    return toPrint;
}

void deleteGPXdoc(GPXdoc* doc)
{
    if(doc == NULL)
    {
        return;
    }

    if(doc->creator != NULL)
    {
        free(doc->creator);
    }

    if(doc->waypoints != NULL)
    {
        freeList(doc->waypoints);
    }

    if(doc->routes != NULL)
    {
        freeList(doc->routes);
    }

    if(doc->tracks != NULL)
    {
        freeList(doc->tracks);
    }

    free(doc);

    return;
}

/***************************************MODULE 2***************************************/

int getNumWaypoints(const GPXdoc* doc)
{
    if(doc == NULL)
    {
        return 0;
    }

    int numWaypoints = getLength(doc->waypoints);

    return numWaypoints;
}

int getNumRoutes(const GPXdoc* doc)
{
    if(doc == NULL)
    {
        return 0;
    }

    int numRoutes = getLength(doc->routes);

    return numRoutes;
}

int getNumTracks(const GPXdoc* doc)
{
    if(doc == NULL)
    {
        return 0;
    }

    int numTracks = getLength(doc->tracks);

    return numTracks;
}

int getNumSegments(const GPXdoc* doc)
{
    if(doc == NULL)
    {
        return 0;
    }

    int numSegs = 0;

    if(getLength(doc->tracks) > 0)
    {
        ListIterator iter = createIterator(doc->tracks);

        void* elem;

        while((elem = nextElement(&iter)) != NULL)
        {
            Track* track = (Track*) elem;

            if(track->segments != NULL)
            {
                numSegs += getLength(track->segments);
            }
        }
    }

    return numSegs;
}

int getNumGPXData(const GPXdoc* doc)
{
    if(doc == NULL)
    {
        return 0;
    }

    int numData = 0;


    Waypoint* waypoint;

    //Wpt
    if(getLength(doc->waypoints) > 0)
    {
        ListIterator dwIter = createIterator(doc->waypoints);

        void* elem;

        while((elem = nextElement(&dwIter)) != NULL)
        {
            waypoint = (Waypoint*) elem;

            if(waypoint->name != NULL)
            {
                if(strlen(waypoint->name) > 0)
                {
                    numData += 1;
                }
            }

            if(waypoint->otherData != NULL)
            {
                numData += getLength(waypoint->otherData);
            }
        }
    }

    //Doc->Rte
    //Doc->Rte->Wpt
    Route* route;
    Waypoint* waypoint2;

    if(getLength(doc->routes) > 0)
    {
        //iterate through routes in the doc
        ListIterator drIter = createIterator(doc->routes);


        void* elem2;
        void* elem3;

        //for each route in the doc
        while((elem2 = nextElement(&drIter)) != NULL)
        {

            route = (Route*) elem2;

            if(route->name != NULL)
            {
                if(strlen(route->name) > 0)
                {
                    numData += 1;
                }
            }

            if(route->otherData != NULL)
            {
                numData += getLength(route->otherData);
            }

            if(route->waypoints != NULL)
            {
                if(getLength(route->waypoints) > 0)
                {
                    ListIterator drwIter = createIterator(route->waypoints);

                    //for each waypoint in the route
                    while((elem3 = nextElement(&drwIter)) != NULL)
                    {
                        waypoint2 = (Waypoint*) elem3;

                        if(waypoint2->name != NULL)
                        {
                            if(strlen(waypoint2->name) > 0)
                            {
                                numData += 1;
                            }
                        }

                        if(waypoint2->otherData != NULL)
                        {
                            numData += getLength(waypoint2->otherData);
                        }
                    }
                }
            }
        }
    }


    //Doc->Trk
    //Doc->Trk->Seg->Wpt
    if(getLength(doc->tracks) > 0)
    {

        void* elem4;
        void* elem5;
        void* elem6;

        Track* track;

        //iterate through tracks in the doc
        ListIterator dtIter = createIterator(doc->tracks);
        //for each track in the doc
        while((elem4 = nextElement(&dtIter)) != NULL)
        {
            track = (Track*) elem4;

            if(track->name != NULL)
            {
                if(strlen(track->name) > 0)
                {
                    numData += 1;
                }
            }

            if(track->otherData != NULL)
            {
                numData += getLength(track->otherData);
            }

            if(track->segments != NULL)
            {
                if(getLength(track->segments) > 0)
                {
                    TrackSegment* segment;

                    ListIterator dtsIter = createIterator(track->segments);

                    //for each seggie in the track
                    while((elem5 = nextElement(&dtsIter)) != NULL)
                    {
                        segment = (TrackSegment*) elem5;

                        if(segment->waypoints != NULL)
                        {
                            if(getLength(segment->waypoints) > 0)
                            {
                                Waypoint* waypoint3;

                                ListIterator dtswIter = createIterator(segment->waypoints);

                                while((elem6 = nextElement(&dtswIter)) != NULL)
                                {
                                    waypoint3 = (Waypoint*) elem6;

                                    if(waypoint3->name != NULL)
                                    {
                                        if(strlen(waypoint3->name) > 0)
                                        {
                                            numData += 1;
                                        }
                                    }

                                    if(waypoint3->otherData != NULL)
                                    {
                                        numData += getLength(waypoint3->otherData);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return numData;
}

Waypoint* getWaypoint(const GPXdoc* doc, char* name)
{
    if((doc == NULL) || (name == NULL))
    {
        return NULL;
    }

    if(strlen(name) < 1)
    {
        return NULL;
    }

    if(doc->waypoints == NULL)
    {
        return NULL;
    }

    if(getLength(doc->waypoints) < 1)
    {
        return NULL;
    }

    ListIterator iter = createIterator(doc->waypoints);

    Waypoint* wpt;

    void* elem;

    while((elem = nextElement(&iter)) != NULL)
    {
        wpt = (Waypoint*) elem;

        if(wpt->name != NULL)
        {
            if((strcmp(wpt->name, name)) == 0)
            {
                return wpt;
            }
        }
    }

    //rtept
    ListIterator drIter = createIterator(doc->routes);
    Route* route;

    void* elem2;
    void* elem3;

    //for each route in the doc
    while((elem2 = nextElement(&drIter)) != NULL)
    {

        route = (Route*) elem2;

        if(route->waypoints != NULL)
        {
            if(getLength(route->waypoints) > 0)
            {
                ListIterator drwIter = createIterator(route->waypoints);

                //for each waypoint in the route
                while((elem3 = nextElement(&drwIter)) != NULL)
                {
                    wpt = (Waypoint*) elem3;

                    if(strcmp(wpt->name, name) == 0)
                    {
                        return wpt;
                    }
                }
            }
        }
    }

    //trkpt
    void* elem4;
    void* elem5;
    void* elem6;

    Track* track;

    //iterate through tracks in the doc
    ListIterator dtIter = createIterator(doc->tracks);
    //for each track in the doc
    while((elem4 = nextElement(&dtIter)) != NULL)
    {
        track = (Track*) elem4;

        if(track->segments != NULL)
        {
            if(getLength(track->segments) > 0)
            {
                TrackSegment* segment;

                ListIterator dtsIter = createIterator(track->segments);

                //for each seggie in the track
                while((elem5 = nextElement(&dtsIter)) != NULL)
                {
                    segment = (TrackSegment*) elem5;

                    if(segment->waypoints != NULL)
                    {
                        if(getLength(segment->waypoints) > 0)
                        {
                            Waypoint* waypoint3;

                            ListIterator dtswIter = createIterator(segment->waypoints);

                            while((elem6 = nextElement(&dtswIter)) != NULL)
                            {
                                waypoint3 = (Waypoint*) elem6;

                                if(strcmp(waypoint3->name, name) == 0)
                                {
                                    return waypoint3;
                                }
                            }
                        }
                    }
                }
            }
        }
    }


    return NULL;
}

Track* getTrack(const GPXdoc* doc, char* name)
{
    if((doc == NULL) || (name == NULL))
    {
        return NULL;
    }

    if(strlen(name) < 1)
    {
        return NULL;
    }

    if(doc->tracks == NULL)
    {
        return NULL;
    }

    if(getLength(doc->tracks) < 1)
    {
        return NULL;
    }

    ListIterator iter = createIterator(doc->tracks);

    Track* trk;

    void* elem;

    while((elem = nextElement(&iter)) != NULL)
    {
        trk = (Track*) elem;

        if(trk->name != NULL)
        {
            if((strcmp(trk->name, name)) == 0)
            {
                return trk;
            }
        }
    }

    return NULL;
}

Route* getRoute(const GPXdoc* doc, char* name)
{
    if((doc == NULL) || (name == NULL))
    {
        return NULL;
    }

    if(strlen(name) < 1)
    {
        return NULL;
    }

    if(doc->routes == NULL)
    {
        return NULL;
    }

    if(getLength(doc->routes) < 1)
    {
        return NULL;
    }

    ListIterator iter = createIterator(doc->routes);

    Route* rte;

    void* elem;

    while((elem = nextElement(&iter)) != NULL)
    {
        rte = (Route*) elem;

        if(rte->name != NULL)
        {
            if((strcmp(rte->name, name)) == 0)
            {
                return rte;
            }
        }
    }

    return NULL;
}

//A2




GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile)
{
    if(fileName == NULL || gpxSchemaFile == NULL)
    {
        return NULL;
    }


    xmlDocPtr doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;
    //char *XMLFileName = "test.xml";
    //char *XSDFileName = "test.xsd";

    //xmlLineNumbersDefault(1);

    ctxt = xmlSchemaNewParserCtxt(gpxSchemaFile);

    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);
    //xmlSchemaDump(stdout, schema); //To print schema dump

    doc = xmlReadFile(fileName, NULL, 0);

    GPXdoc* returnDoc = NULL;

    if (doc == NULL)
    {
        fprintf(stderr, "Could not parse %s\n", fileName);
        xmlSchemaFree(schema);
        xmlSchemaCleanupTypes();
        xmlCleanupParser();
        xmlMemoryDump();
        return NULL;
    }

    else
    {
        xmlSchemaValidCtxtPtr ctxt;
        int ret;

        ctxt = xmlSchemaNewValidCtxt(schema);
        xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
        ret = xmlSchemaValidateDoc(ctxt, doc);

        if (ret == 0)
        {
            xmlFreeDoc(doc);
            xmlCleanupParser();
            returnDoc = createGPXdoc(fileName);
        }
        else if (ret > 0)
        {
            xmlFreeDoc(doc);
        }
        else
        {
            xmlFreeDoc(doc);
        }

        xmlSchemaFreeValidCtxt(ctxt);
    }

    // free the resource
    if(schema != NULL)
        xmlSchemaFree(schema);

    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();

    return returnDoc;

}



bool validateGPXDoc(GPXdoc* doc, char* gpxSchemaFile)
{
    if(doc == NULL || gpxSchemaFile == NULL)
        return false;

    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;

    ctxt = xmlSchemaNewParserCtxt(gpxSchemaFile);

    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);

    xmlDocPtr xmlDoc = xmlNewDoc(BAD_CAST "1.0");

    xmlDoc->type = XML_DOCUMENT_NODE;
    

    xmlNode* root_node = gpxDocToNode(doc);
    xmlDocSetRootElement(xmlDoc, root_node);

    xmlSchemaValidCtxtPtr ctxt2;
    int ret;

    ctxt2 = xmlSchemaNewValidCtxt(schema);
    xmlSchemaSetValidErrors(ctxt2, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    ret = xmlSchemaValidateDoc(ctxt2, xmlDoc);

    xmlSchemaFreeValidCtxt(ctxt2);
    xmlFreeDoc(xmlDoc);
    
    if(schema != NULL)
        xmlSchemaFree(schema);


    if(ret == 0)
        return true;
    else    
        return false;
}


bool writeGPXdoc(GPXdoc* doc, char* fileName)
{
    printf("inWrite, filename=%s\n", fileName);
    if(doc == NULL || fileName == NULL)
        return false;
    
    int extensionIndex = getSubstringIndex(fileName, ".gpx");

    if(extensionIndex == -1)
        return false;
    if((strlen(fileName) - 4) != extensionIndex)
        return false;

    xmlDocPtr xmlDoc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL;/* node pointers */

    root_node = gpxDocToNode(doc);

    xmlDoc = xmlNewDoc(BAD_CAST "1.0");
    xmlDocSetRootElement(xmlDoc, root_node);

    xmlSaveFormatFileEnc(fileName, xmlDoc, "ISO-8859-1", 1);

    xmlFreeDoc(xmlDoc);

    return true;    
}

/* Module 2 */
float getRouteLen(const Route *rt)
{
    if (rt == NULL)
        return 0.0;

    double dist = 0.0;

    if(getLength(rt->waypoints) > 0)
        for(Node* node = rt->waypoints->head; node->next != NULL; node = node->next)
        {
            Waypoint* w1 = node->data;
            Waypoint* w2 = node->next->data;

            dist += haversine(w1->latitude, w1->longitude, w2->latitude, w2->longitude);
        }

    return dist;
}

float getTrackLen(const Track *tr)
{
    if(tr == NULL)
        return 0.0;

    float dist = 0.0;

    List* waypointList = initializeList(&(waypointToString), &(dummyDelete), &(compareWaypoints));

    for(Node* node = tr->segments->head; node != NULL; node = node->next)
    {
        TrackSegment* seg = node->data;

        for(Node* node2 = seg->waypoints->head; node2 != NULL; node2 = node2->next)
        {
            insertBack(waypointList, node2->data);
        }
    }

    if(getLength(waypointList) > 0)
        for(Node* node = waypointList->head; node->next != NULL; node = node->next)
        {
            Waypoint* wpt = node->data;
            Waypoint* wpt2 = node->next->data;

            dist += haversine(wpt->latitude, wpt->longitude, wpt2->latitude, wpt2->longitude);
        }

    freeList(waypointList);

    return dist;
}

float round10(float len)
{
    float integer = (float) floor(mag(len));

    int up = round(len);
    int down = round(len);

    float returny = 0.5;

    while(up % 10 != 0)//find nearest 10 higher than len
    {
        up += 1;
    }

    while(down % 10 != 0)//find nearest 10 lower than len
    {
        down -= 1;
    }

    int upDiff = mag(up) - mag(integer);//difference between |len| and |higher 10|
    int downDiff = mag(down) - mag(integer);//difference between |len| and |lower 10|


    if(mag(upDiff) <= mag(downDiff))//if the higher 10 is closer than the lower 10
        returny = (float) up;
    else
        returny = (float) down;

    if(len < 0)//if the input was -'ve then the rounded number will also be -'ve (or 0)
        returny *= -1;

    if(mag(returny) == 0.0)//gets rid of occasional "-0"
        returny = 0.0;

    return returny;
}


int numRoutesWithLength(const GPXdoc* doc, float len, float delta)
{
    if(doc == NULL || len < 0 || delta < 0)
        return 0;

    int counter = 0;
    
    for(Node* node = doc->routes->head; node != NULL; node = node->next)
    {
        float dist = getRouteLen(node->data);

        if(mag(len - dist) <= delta)
            counter++;
    }

    return counter;
}

int numTracksWithLength(const GPXdoc* doc, float len, float delta)
{
    if(doc == NULL || len < 0 || delta < 0)
        return 0;

    int counter = 0;
    
    for(Node* node = doc->tracks->head; node != NULL; node = node->next)
    {
        float dist = getTrackLen(node->data);

        if(mag(len - dist) <= delta)
            counter++;
    }

    return counter;
}


bool isLoopRoute(const Route* route, float delta)
{
    if(route == NULL || delta < 0)
        return false;

    if(getLength(route->waypoints) < 4)
        return false;

    Waypoint* first = route->waypoints->head->data;
    Waypoint* last = route->waypoints->tail->data;

    if(haversine(first->latitude, first->longitude, last->latitude, last->longitude) < delta)
        return true;
    else
        return false;
}

bool isLoopTrack(const Track *tr, float delta)
{
    if(tr == NULL || delta < 0)
        return false;

    if(getLength(tr->segments) == 0)
        return false;
    
    List* waypointList = initializeList(&(waypointToString), &(dummyDelete), &(compareWaypoints));

    for(Node* node = tr->segments->head; node != NULL; node = node->next)
    {
        TrackSegment* seg = node->data;


        for(Node* node2 = seg->waypoints->head; node2 != NULL; node2 = node2->next)
        {
            insertBack(waypointList, node2->data);
        }
    }

    if(getLength(waypointList) < 4)
        return false;

    Waypoint* first = waypointList->head->data;
    Waypoint* last = waypointList->tail->data;

    freeList(waypointList);

    if(haversine(first->latitude, first->longitude, last->latitude, last->longitude) < delta)
        return true;
    else
        return false;
}

//1
List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta)
{
    if(doc == NULL || delta < 0)
        return NULL;

    List* routesBetween = initializeList(&(routeToString), &(dummyDelete), &(compareRoutes));

    float startDist = 0.0;
    float endDist = 0.0;

    for(Node* node = doc->routes->head; node != NULL; node = node->next)
    {
        Route* route = node->data;

        Waypoint* wStart = route->waypoints->head->data;

        Waypoint* wEnd = route->waypoints->tail->data;

        startDist = mag(haversine(sourceLat, sourceLong, wStart->latitude, wStart->longitude));

        endDist = mag(haversine(destLat, destLong, wEnd->latitude, wEnd->longitude));

        if((startDist <= delta) && (endDist <= delta))
            insertBack(routesBetween, route);
    }

    if(getLength(routesBetween) == 0)
    {   
        freeList(routesBetween);
        return NULL;
    }

    return routesBetween;
}
//2
List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta)
{
    if(doc == NULL || delta < 0)
        return NULL;

    List* tracksBetween = initializeList(&(trackToString), &(dummyDelete), &(compareTracks));

    float startDist = 0.0;
    float endDist = 0.0;

    for(Node* node = doc->tracks->head; node != NULL; node = node->next)
    {
        Track* track = node->data;

        TrackSegment* segStart = track->segments->head->data;

        TrackSegment* segEnd = track->segments->tail->data;

        Waypoint* wStart = segStart->waypoints->head->data;

        Waypoint* wEnd = segEnd->waypoints->tail->data;

        startDist = mag(haversine(sourceLat, sourceLong, wStart->latitude, wStart->longitude));

        endDist = mag(haversine(destLat, destLong, wEnd->longitude, wEnd->longitude));

        if((startDist <= delta) && (endDist <= delta))
            insertBack(tracksBetween, track);
        
    }

    if(getLength(tracksBetween) == 0)
    {   
        freeList(tracksBetween);
        return NULL;
    }

    return tracksBetween;
}

char* trackToJSON(const Track *tr)
{
    if(tr == NULL)
        return NULL;
    
    Track* track = (Track*) tr;
    return trackToJSON2(track);
}

char* routeToJSON(const Route *rt)
{
    if(rt == NULL)
        return NULL;

    Route* route = (Route*) rt;
    return routeToJSON2(route);
}

char* routeListToJSON(const List *list)
{
    if(list == NULL)
        return NULL;

    List* l = (List*) list;

    char* JSON = listToJSON(l, routeToJSON2);

    return JSON;
}

char* trackListToJSON(const List *list)
{
    if(list == NULL)
        return NULL;

    List* l = (List*) list;

    char* JSON = listToJSON(l, trackToJSON2);

    return JSON;
}
//3
char* GPXtoJSON(const GPXdoc* gpx)
{
    if(gpx == NULL)
        return "{}";

    GPXdoc* doc = (GPXdoc*) gpx;

    int memSize = 1024;

    char* JSON = calloc(memSize, sizeof(char));

    sprintf(JSON, "{\"version\":%.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}", doc->version, doc->creator, getNumWaypoints(doc), getNumRoutes(doc), getNumTracks(doc));

    return JSON;
}




//BONUS*********************************************

void addWaypoint(Route *rt, Waypoint *pt)
{
    if(rt == NULL || pt == NULL)
        return;
    
    insertBack(rt->waypoints, pt);
}

void addRoute(GPXdoc* doc, Route* rt)
{
    if(doc == NULL || rt == NULL)
        return;

    insertBack(doc->routes, rt);
}

GPXdoc* JSONtoGPX(const char* gpxString)
{
    if(gpxString == NULL)
        return NULL;

    char* JSON = (char*) gpxString;

    int commaPos = getIndex(JSON, ',');

    char* verHalf = substringCopy(JSON, 0, commaPos);

    char* creatorHalf = substringCopy(JSON, commaPos + 1, strlen(JSON));

    int verColon = getIndex(verHalf, ':');

    char* verString = substringCopy(verHalf, verColon + 1, strlen(verHalf));

    free(verHalf);
    
    double version = atof(verString);

    free(verString);

    int creatorColon = getIndex(creatorHalf, ':');

    char* creatorString = substringCopy(creatorHalf, creatorColon + 2, strlen(creatorHalf) - 2);

    free(creatorHalf);

    GPXdoc* doc = calloc(1, sizeof(GPXdoc));

    strcpy(doc->namespace, "http://www.topografix.com/GPX/1/1");

    doc->version = version;

    doc->creator = calloc(256, sizeof(char));
    strcpy(doc->creator, creatorString);

    free(creatorString);

    doc->waypoints = initializeList(&(waypointToString), &(deleteWaypoint), &(compareWaypoints));

    doc->routes = initializeList(&(routeToString), &(deleteRoute), &(compareRoutes));

    doc->tracks = initializeList(&(trackToString), &(deleteTrack), &(compareTracks));
    
    return doc;
}

Waypoint* JSONtoWaypoint(const char* gpxString)
{
    if(gpxString == NULL)
        return NULL;
    
    char* JSON = (char*) gpxString;

    int commaPos = getIndex(JSON, ',');

    char* latHalf = substringCopy(JSON, 0, commaPos);

    char* lonHalf = substringCopy(JSON, commaPos + 1, strlen(JSON));

    int latColon = getIndex(latHalf, ':');

    char* latString = substringCopy(latHalf, latColon + 1, strlen(latHalf));

    double lat = atof(latString);

    free(latString);
    free(latHalf);


    int lonColon = getIndex(lonHalf, ':');

    char* lonString = substringCopy(lonHalf, lonColon + 1, strlen(lonHalf) - 1);

    double lon = atof(lonString);

    free(lonString);
    free(lonHalf);

    Waypoint* wpt = calloc(1, sizeof(Waypoint));

    wpt->name = calloc(256, sizeof(char));

    wpt->otherData = initializeList(&(gpxDataToString), &(deleteGpxData), &(compareGpxData));

    wpt->latitude = lat;

    wpt->longitude = lon;

    return wpt;
}

Route* JSONtoRoute(const char* gpxString)
{
    if(gpxString == NULL)
        return NULL;

    char* JSON = (char*) gpxString;

    int colon = getIndex(JSON, ':');

    char* nameString = substringCopy(JSON, colon + 2, strlen(JSON) - 2);

    Route* route = calloc(1, sizeof(Route));

    route->name = calloc(256, sizeof(char));

    route->otherData = initializeList(&(gpxDataToString), &(deleteGpxData), &(compareGpxData));

    route->waypoints = initializeList(&(waypointToString), &(deleteWaypoint), &(compareWaypoints));

    strcpy(route->name, nameString);

    free(nameString);

    return route;
}
